#include "CognitiveSceneExportUtils.h"

#include "Dom/JsonObject.h"
#include "HAL/FileManager.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/FileHelper.h"
#include "Misc/Guid.h"
#include "Misc/Paths.h"
#include "PlatformHttp.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

DEFINE_LOG_CATEGORY_STATIC(LogCognitiveSceneExport, Log, All);

namespace
{
using FValues = TArray<TSharedPtr<FJsonValue>>;
using FObject = TSharedPtr<FJsonObject>;

struct FBufferCopy
{
	FString Path;
	TArray<uint8> Bytes;
	int32 DeclaredLength = 0;
	bool Dirty = false;
};

struct FAccessorSpan
{
	int32 Buffer = INDEX_NONE;
	int32 Offset = 0;
	int32 Stride = 0;
	int32 Count = 0;
	int32 Component = 0;
	int32 ElementSize = 0;
	FString Type;
	bool Sparse = false;
};

class FNormalizer
{
public:
	FString Error;
	FString Directory;
	FObject Root;
	FValues Buffers, Views, Accessors, Meshes, Materials, Images, Skins, Animations, Nodes;
	TArray<FBufferCopy> Binary;
	TArray<FAccessorSpan> Spans;
	int32 RemovedPrimitives = 0, PromotedIndices = 0, FixedTangents = 0, ClampedMaterials = 0, RenamedImages = 0;
	TMap<FString, FString> RepairedImageUris;

	bool Fail(const FString& Message)
	{
		Error = Message;
		return false;
	}
	FObject Object(const FValues& Values, int32 Index)
	{
		if (!Values.IsValidIndex(Index) || !Values[Index].IsValid() || Values[Index]->Type != EJson::Object)
		{
			Fail(FString::Printf(TEXT("Invalid glTF object at index %d."), Index));
			return nullptr;
		}
		return Values[Index]->AsObject();
	}
	bool Array(const FObject& Parent, const FString& Key, FValues& Out, bool Required = false)
	{
		const FValues* Found = nullptr;
		if (!Parent.IsValid())
		{
			return Fail(TEXT("Invalid glTF parent object."));
		}
		if (!Parent->HasField(Key))
		{
			return !Required || Fail(TEXT("Missing glTF array: ") + Key);
		}
		if (!Parent->TryGetArrayField(Key, Found))
		{
			return Fail(TEXT("Invalid glTF array: ") + Key);
		}
		Out = *Found;
		return true;
	}
	bool Integer(const FObject& Parent, const FString& Key, int32& Out, int32 Default = INDEX_NONE)
	{
		if (!Parent.IsValid())
		{
			return Fail(TEXT("Invalid glTF parent object."));
		}
		if (!Parent->HasField(Key))
		{
			Out = Default;
			return true;
		}
		const TSharedPtr<FJsonValue> Field = Parent->TryGetField(Key);
		double Number = 0;
		if (!Field.IsValid() || Field->Type != EJson::Number || !Field->TryGetNumber(Number) || !FMath::IsFinite(Number) || Number < 0 || Number > MAX_int32 || Number != double(int32(Number)))
		{
			return Fail(TEXT("Invalid nonnegative glTF integer: ") + Key);
		}
		Out = int32(Number);
		return true;
	}
	bool Index(const FObject& Parent, const FString& Key, int32 Limit, int32& Out, bool Required = true)
	{
		if (!Integer(Parent, Key, Out))
		{
			return false;
		}
		if (Out == INDEX_NONE && !Required)
		{
			return true;
		}
		return (Out >= 0 && Out < Limit) || Fail(TEXT("Invalid glTF reference: ") + Key);
	}
	bool LocalPath(const FString& Uri, FString& Out)
	{
		const FString Decoded = FPlatformHttp::UrlDecode(Uri);
		if (Decoded.IsEmpty() || !FPaths::IsRelative(Decoded) || Decoded.Contains(TEXT(":")) || Uri.Contains(TEXT("?")) || Uri.Contains(TEXT("#")))
		{
			return Fail(TEXT("Export requires a local external glTF resource URI: ") + Uri);
		}
		Out = FPaths::ConvertRelativePathToFull(FPaths::Combine(Directory, Decoded));
		FPaths::NormalizeFilename(Out);
		if (!FPaths::CollapseRelativeDirectories(Out) || !Out.StartsWith(Directory + TEXT("/"), ESearchCase::IgnoreCase))
		{
			return Fail(TEXT("glTF resource escapes the scene export directory: ") + Uri);
		}
		return IFileManager::Get().FileSize(*Out) > 0 || Fail(TEXT("Missing or empty glTF resource: ") + Uri);
	}
	bool RepairImageUri(const FObject& Image)
	{
		FString Uri;
		if (!Image->TryGetStringField(TEXT("uri"), Uri) || Uri.StartsWith(TEXT("data:")))
		{
			return true;
		}
		if (const FString* Repaired = RepairedImageUris.Find(Uri))
		{
			Image->SetStringField(TEXT("uri"), *Repaired);
			return true;
		}
		const FString Decoded = FPlatformHttp::UrlDecode(Uri);
		bool bAscii = true;
		for (const TCHAR Char : Decoded)
		{
			bAscii &= Char >= 32 && Char < 127;
		}
		if (bAscii && IFileManager::Get().FileSize(*FPaths::Combine(Directory, Decoded)) > 0)
		{
			return true;
		}
		TArray<FString> Files;
		IFileManager::Get().FindFiles(Files, *FPaths::Combine(Directory, TEXT("*")), true, false);
		for (const FString& File : Files)
		{
			FString Mangled, Safe;
			for (const TCHAR Char : File)
			{
				if (Char < 0x80)
				{
					Mangled.AppendChar(Char);
					Safe.AppendChar(Char);
					continue;
				}
				Mangled.AppendChar(TCHAR(Char & 0xFF));
				Mangled.AppendChar(TCHAR((Char >> 8) & 0xFF));
				Safe += FString::Printf(TEXT("_u%04X_"), uint32(Char));
			}
			if (File != Decoded && Mangled != Decoded)
			{
				continue;
			}
			if (Safe != File && !IFileManager::Get().Move(*FPaths::Combine(Directory, Safe), *FPaths::Combine(Directory, File), true, true))
			{
				return Fail(TEXT("Cannot give an exported image an ASCII file name: ") + Safe);
			}
			RepairedImageUris.Add(Uri, Safe);
			Image->SetStringField(TEXT("uri"), Safe);
			++RenamedImages;
			return true;
		}
		return true;
	}
	int32 ComponentBytes(int32 Component) const
	{
		switch (Component)
		{
			case 5120:
			case 5121:
				return 1;
			case 5122:
			case 5123:
				return 2;
			case 5125:
			case 5126:
				return 4;
			default:
				return 0;
		}
	}
	int32 ElementBytes(const FString& Type, int32 Bytes) const
	{
		if (Type == TEXT("SCALAR"))
		{
			return Bytes;
		}
		if (Type == TEXT("VEC2"))
		{
			return Bytes * 2;
		}
		if (Type == TEXT("VEC3"))
		{
			return Bytes * 3;
		}
		if (Type == TEXT("VEC4"))
		{
			return Bytes * 4;
		}
		if (Type == TEXT("MAT2"))
		{
			return Align(Bytes * 2, 4) * 2;
		}
		if (Type == TEXT("MAT3"))
		{
			return Align(Bytes * 3, 4) * 3;
		}
		if (Type == TEXT("MAT4"))
		{
			return Bytes * 16;
		}
		return 0;
	}
	bool ViewRange(int32 ViewIndex, int32 Offset, int32 Count, int32 Size, int32& Buffer, int32& Absolute, int32& Stride, bool AllowStride)
	{
		FObject View = Object(Views, ViewIndex);
		int32 Start = 0, Length = 0;
		if (!View || !Index(View, TEXT("buffer"), Binary.Num(), Buffer) || !Integer(View, TEXT("byteOffset"), Start, 0) || !Integer(View, TEXT("byteLength"), Length) || !Integer(View, TEXT("byteStride"), Stride, Size))
		{
			return false;
		}
		if (Length < 0 || Stride < Size || (!AllowStride && View->HasField(TEXT("byteStride"))))
		{
			return Fail(TEXT("Invalid glTF buffer view stride or length."));
		}
		const int64 Required = Count > 0 ? int64(Count - 1) * Stride + Size : 0;
		if (int64(Start) + Length > Binary[Buffer].DeclaredLength || int64(Offset) + Required > Length)
		{
			return Fail(TEXT("glTF accessor or view exceeds its buffer byteLength."));
		}
		Absolute = Start + Offset;
		return true;
	}
	bool ReadSpan(int32 IndexValue, FAccessorSpan& Span)
	{
		FObject Accessor = Object(Accessors, IndexValue);
		int32 View = INDEX_NONE, Offset = 0;
		if (!Accessor || !Integer(Accessor, TEXT("count"), Span.Count) || !Integer(Accessor, TEXT("componentType"), Span.Component) || !Integer(Accessor, TEXT("byteOffset"), Offset, 0) || !Integer(Accessor, TEXT("bufferView"), View) || !Accessor->TryGetStringField(TEXT("type"), Span.Type))
		{
			return Fail(TEXT("Malformed glTF accessor."));
		}
		const int32 Bytes = ComponentBytes(Span.Component);
		Span.ElementSize = ElementBytes(Span.Type, Bytes);
		if (Span.Count < 0 || Bytes == 0 || Span.ElementSize == 0 || Offset % Bytes != 0)
		{
			return Fail(TEXT("Unsupported or malformed glTF accessor format."));
		}
		if (View != INDEX_NONE && !ViewRange(View, Offset, Span.Count, Span.ElementSize, Span.Buffer, Span.Offset, Span.Stride, true))
		{
			return false;
		}
		const FObject* Sparse = nullptr;
		if (Accessor->HasField(TEXT("sparse")))
		{
			if (!Accessor->TryGetObjectField(TEXT("sparse"), Sparse))
			{
				return Fail(TEXT("Invalid sparse glTF accessor."));
			}
			Span.Sparse = true;
			int32 Count = 0;
			const FObject* Indices = nullptr;
			const FObject* Values = nullptr;
			if (!Integer(*Sparse, TEXT("count"), Count) || Count <= 0 || Count > Span.Count || !(*Sparse)->TryGetObjectField(TEXT("indices"), Indices) || !(*Sparse)->TryGetObjectField(TEXT("values"), Values))
			{
				return Fail(TEXT("Invalid sparse glTF accessor data."));
			}
			int32 SparseView = 0, SparseOffset = 0, Component = 0, Buffer = 0, Absolute = 0, Stride = 0;
			if (!Integer(*Indices, TEXT("componentType"), Component) || (Component != 5121 && Component != 5123 && Component != 5125) || !Index(*Indices, TEXT("bufferView"), Views.Num(), SparseView) || !Integer(*Indices, TEXT("byteOffset"), SparseOffset, 0) || !ViewRange(SparseView, SparseOffset, Count, ComponentBytes(Component), Buffer, Absolute, Stride, false))
			{
				return Fail(TEXT("Invalid sparse glTF index range."));
			}
			uint32 Previous = 0;
			for (int32 i = 0; i < Count; ++i)
			{
				uint32 Value = 0;
				FMemory::Memcpy(&Value, Binary[Buffer].Bytes.GetData() + Absolute + i * Stride, ComponentBytes(Component));
				if (Value >= uint32(Span.Count) || (i > 0 && Value <= Previous))
				{
					return Fail(TEXT("Sparse glTF indices must be ordered and within accessor count."));
				}
				Previous = Value;
			}
			if (!Index(*Values, TEXT("bufferView"), Views.Num(), SparseView) || !Integer(*Values, TEXT("byteOffset"), SparseOffset, 0) || !ViewRange(SparseView, SparseOffset, Count, Span.ElementSize, Buffer, Absolute, Stride, false))
			{
				return Fail(TEXT("Invalid sparse glTF value range."));
			}
		}
		if (Span.Count > 0 && Span.Buffer == INDEX_NONE && !Span.Sparse)
		{
			return Fail(TEXT("glTF accessor has no buffer data."));
		}
		FValues Minimum, Maximum;
		if (!Array(Accessor, TEXT("min"), Minimum) || !Array(Accessor, TEXT("max"), Maximum))
		{
			return false;
		}
		for (const FString& Key : { FString(TEXT("min")), FString(TEXT("max")) })
		{
			FValues Bounds;
			if (!Array(Accessor, Key, Bounds))
			{
				return false;
			}
			if (!Accessor->HasField(Key))
			{
				continue;
			}
			const int32 Components = Span.Type == TEXT("MAT2") ? 4 : Span.Type == TEXT("MAT3") ? 9 : Span.ElementSize / Bytes;
			if (Bounds.Num() != Components)
			{
				return Fail(TEXT("Invalid glTF accessor bounds size."));
			}
			for (const auto& Value : Bounds)
			{
				double Number = 0;
				if (!Value.IsValid() || Value->Type != EJson::Number || !Value->TryGetNumber(Number) || !FMath::IsFinite(Number))
				{
					return Fail(TEXT("Non-finite or malformed glTF accessor bounds."));
				}
			}
		}
		if (Minimum.Num() > 0 && Maximum.Num() > 0)
		{
			for (int32 c = 0; c < Minimum.Num(); ++c)
			{
				if (Minimum[c]->AsNumber() > Maximum[c]->AsNumber())
				{
					return Fail(TEXT("glTF accessor minimum exceeds maximum."));
				}
			}
		}
		if (Span.Component == 5126 && Span.Buffer != INDEX_NONE)
		{
			for (int32 i = 0; i < Span.Count; ++i)
			{
				for (int32 c = 0; c < Span.ElementSize / 4; ++c)
				{
					float Value;
					FMemory::Memcpy(&Value, Binary[Span.Buffer].Bytes.GetData() + Span.Offset + i * Span.Stride + c * 4, 4);
					if (!FMath::IsFinite(Value))
					{
						return Fail(TEXT("Non-finite glTF accessor data."));
					}
				}
			}
		}
		return true;
	}
	bool Direct(int32 Accessor, const FString& Type, int32 Component, FAccessorSpan*& Out)
	{
		if (!Spans.IsValidIndex(Accessor))
		{
			return Fail(TEXT("Invalid glTF accessor reference."));
		}
		Out = &Spans[Accessor];
		return (Out->Buffer != INDEX_NONE && !Out->Sparse && Out->Type == Type && Out->Component == Component) || Fail(TEXT("Export normalization requires non-sparse geometry in the expected glTF format."));
	}
	float Float(const FAccessorSpan& Span, int32 Vertex, int32 Component) const
	{
		float Value;
		FMemory::Memcpy(&Value, Binary[Span.Buffer].Bytes.GetData() + Span.Offset + Vertex * Span.Stride + Component * 4, 4);
		return Value;
	}
	bool PromoteIndex(int32 AccessorIndex, int32 VertexCount)
	{
		FAccessorSpan& Span = Spans[AccessorIndex];
		if (Span.Sparse || Span.Buffer == INDEX_NONE || Span.Type != TEXT("SCALAR") || (Span.Component != 5121 && Span.Component != 5123 && Span.Component != 5125))
		{
			return Fail(TEXT("Unsupported glTF primitive indices."));
		}
		const int32 Width = ComponentBytes(Span.Component);
		bool Normalized = false;
		if (Object(Accessors, AccessorIndex)->TryGetBoolField(TEXT("normalized"), Normalized) && Normalized)
		{
			return Fail(TEXT("glTF primitive indices cannot be normalized."));
		}
		const uint32 Maximum = Width == 1 ? 255u : Width == 2 ? 65535u : MAX_uint32;
		bool NeedsPromotion = false;
		for (int32 i = 0; i < Span.Count; ++i)
		{
			uint32 Value = 0;
			FMemory::Memcpy(&Value, Binary[Span.Buffer].Bytes.GetData() + Span.Offset + i * Span.Stride, Width);
			if (Value >= uint32(VertexCount))
			{
				return Fail(TEXT("glTF primitive index exceeds its POSITION count."));
			}
			NeedsPromotion |= Value == Maximum;
		}
		if (!NeedsPromotion)
		{
			return true;
		}
		if (Width == 4)
		{
			return Fail(TEXT("Cannot promote a 32-bit glTF primitive restart index."));
		}
		FBufferCopy& Buffer = Binary[Span.Buffer];
		const int32 NewWidth = Width * 2;
		const int64 End = Align(int64(Buffer.Bytes.Num()), int64(4)) + int64(Span.Count) * NewWidth;
		if (End > MAX_int32)
		{
			return Fail(TEXT("Normalized glTF buffer exceeds supported size."));
		}
		const int32 Start = int32(Align(int64(Buffer.Bytes.Num()), int64(4)));
		Buffer.Bytes.SetNumZeroed(int32(End));
		for (int32 i = 0; i < Span.Count; ++i)
		{
			uint32 Value = 0;
			FMemory::Memcpy(&Value, Buffer.Bytes.GetData() + Span.Offset + i * Span.Stride, Width);
			FMemory::Memcpy(Buffer.Bytes.GetData() + Start + i * NewWidth, &Value, NewWidth);
		}
		FObject View = MakeShared<FJsonObject>();
		View->SetNumberField(TEXT("buffer"), Span.Buffer);
		View->SetNumberField(TEXT("byteOffset"), Start);
		View->SetNumberField(TEXT("byteLength"), Span.Count * NewWidth);
		View->SetNumberField(TEXT("target"), 34963);
		FObject Accessor = Object(Accessors, AccessorIndex);
		Accessor->SetNumberField(TEXT("bufferView"), Views.Num());
		Accessor->SetNumberField(TEXT("componentType"), Width == 1 ? 5123 : 5125);
		Accessor->RemoveField(TEXT("byteOffset"));
		Views.Add(MakeShared<FJsonValueObject>(View));
		Buffer.DeclaredLength = Buffer.Bytes.Num();
		Buffer.Dirty = true;
		Object(Buffers, Span.Buffer)->SetNumberField(TEXT("byteLength"), Buffer.DeclaredLength);
		if (!ReadSpan(AccessorIndex, Span))
		{
			return false;
		}
		++PromotedIndices;
		return true;
	}
	bool RepairTangents(int32 TangentIndex, int32 NormalIndex, int32 VertexCount)
	{
		FAccessorSpan* Tangent = nullptr;
		FAccessorSpan* Normal = nullptr;
		if (!Direct(TangentIndex, TEXT("VEC4"), 5126, Tangent) || Tangent->Count != VertexCount)
		{
			return Fail(TEXT("Invalid glTF TANGENT accessor."));
		}
		bool Changed = false;
		for (int32 i = 0; i < VertexCount; ++i)
		{
			FVector T(Float(*Tangent, i, 0), Float(*Tangent, i, 1), Float(*Tangent, i, 2));
			const float W = Float(*Tangent, i, 3);
			if (T.ContainsNaN() || !FMath::IsFinite(W))
			{
				return Fail(TEXT("Non-finite glTF tangent."));
			}
			if (!T.IsNearlyZero())
			{
				if (!FMath::IsNearlyEqual(FMath::Abs(W), 1.0f, 0.001f))
				{
					return Fail(TEXT("Invalid glTF tangent handedness."));
				}
				continue;
			}
			if (!Normal && (!Direct(NormalIndex, TEXT("VEC3"), 5126, Normal) || Normal->Count != VertexCount))
			{
				return Fail(TEXT("Cannot repair a zero tangent without matching NORMAL data."));
			}
			FVector N(Float(*Normal, i, 0), Float(*Normal, i, 1), Float(*Normal, i, 2));
			if (N.ContainsNaN() || !N.Normalize())
			{
				return Fail(TEXT("Cannot repair a tangent with an invalid normal."));
			}
			const FVector Axis = FMath::Abs(N.X) < FMath::Abs(N.Y) ? (FMath::Abs(N.X) < FMath::Abs(N.Z) ? FVector(1, 0, 0) : FVector(0, 0, 1)) : (FMath::Abs(N.Y) < FMath::Abs(N.Z) ? FVector(0, 1, 0) : FVector(0, 0, 1));
			T = FVector::CrossProduct(N, Axis).GetSafeNormal();
			const float Values[] = { float(T.X), float(T.Y), float(T.Z), W < 0 ? -1.0f : 1.0f };
			FMemory::Memcpy(Binary[Tangent->Buffer].Bytes.GetData() + Tangent->Offset + i * Tangent->Stride, Values, sizeof(Values));
			Binary[Tangent->Buffer].Dirty = true;
			Changed = true;
			++FixedTangents;
		}
		if (Changed)
		{
			Object(Accessors, TangentIndex)->RemoveField(TEXT("min"));
			Object(Accessors, TangentIndex)->RemoveField(TEXT("max"));
		}
		return true;
	}
	bool Remap(const FObject& Parent, const FString& Key, const TArray<int32>& Mapping, bool Required = false)
	{
		int32 Old = INDEX_NONE;
		if (!Index(Parent, Key, Mapping.Num(), Old, Required))
		{
			return false;
		}
		if (Old == INDEX_NONE)
		{
			return true;
		}
		if (Mapping[Old] == INDEX_NONE)
		{
			return Fail(TEXT("Removed empty glTF data is still referenced by ") + Key);
		}
		Parent->SetNumberField(Key, Mapping[Old]);
		return true;
	}
	bool RemapAttributes(const FObject& Parent, const TArray<int32>& Mapping)
	{
		if (!Parent)
		{
			return Fail(TEXT("Invalid glTF attributes object."));
		}
		TArray<FString> Keys;
		Parent->Values.GetKeys(Keys);
		for (const FString& Key : Keys)
		{
			if (!Remap(Parent, Key, Mapping, true))
			{
				return false;
			}
		}
		return true;
	}
	void Compact(FValues& Values, const TArray<bool>& Remove, TArray<int32>& Mapping)
	{
		FValues Kept;
		Mapping.Init(INDEX_NONE, Values.Num());
		for (int32 i = 0; i < Values.Num(); ++i)
		{
			if (!Remove[i])
			{
				Mapping[i] = Kept.Num();
				Kept.Add(Values[i]);
			}
		}
		Values = MoveTemp(Kept);
	}
	bool TextureReferences(const FObject& Parent, int32 TextureCount)
	{
		for (const auto& Pair : Parent->Values)
		{
			if (!Pair.Value.IsValid())
			{
				return Fail(TEXT("Invalid glTF material property."));
			}
			if (Pair.Value->Type != EJson::Object)
			{
				continue;
			}
			FObject Child = Pair.Value->AsObject();
			if (Pair.Key.EndsWith(TEXT("Texture")))
			{
				int32 IndexValue = 0;
				if (!Index(Child, TEXT("index"), TextureCount, IndexValue))
				{
					return false;
				}
			}
			if (!TextureReferences(Child, TextureCount))
			{
				return false;
			}
		}
		return true;
	}
	bool CheckExtensionReferences(const TSharedPtr<FJsonValue>& Value, bool InExtension = false)
	{
		if (!Value.IsValid())
		{
			return Fail(TEXT("Invalid glTF JSON value."));
		}
		if (Value->Type == EJson::Object)
		{
			for (const auto& Pair : Value->AsObject()->Values)
			{
				if (InExtension && (Pair.Key == TEXT("bufferView") || Pair.Key == TEXT("accessor") || Pair.Key == TEXT("accessors") || Pair.Key == TEXT("attributes") || Pair.Key == TEXT("mesh")))
				{
					return Fail(TEXT("An extension owns geometry references unsupported by scene normalization: ") + Pair.Key);
				}
				if (!CheckExtensionReferences(Pair.Value, InExtension || Pair.Key == TEXT("extensions")))
				{
					return false;
				}
			}
		}
		else if (Value->Type == EJson::Array)
		{
			for (const auto& Element : Value->AsArray())
			{
				if (!CheckExtensionReferences(Element, InExtension))
				{
					return false;
				}
			}
		}
		return true;
	}
	bool Normalize()
	{
		const FObject* Asset = nullptr;
		FString Version;
		if (!Root->TryGetObjectField(TEXT("asset"), Asset) || !(*Asset)->TryGetStringField(TEXT("version"), Version) || Version != TEXT("2.0"))
		{
			return Fail(TEXT("Expected an exported glTF 2.0 asset."));
		}
		if (!CheckExtensionReferences(MakeShared<FJsonValueObject>(Root)))
		{
			return false;
		}
		if (!Array(Root, TEXT("buffers"), Buffers, true) || !Array(Root, TEXT("bufferViews"), Views, true) || !Array(Root, TEXT("accessors"), Accessors, true) || !Array(Root, TEXT("meshes"), Meshes, true) || !Array(Root, TEXT("materials"), Materials) || !Array(Root, TEXT("images"), Images) || !Array(Root, TEXT("skins"), Skins) || !Array(Root, TEXT("animations"), Animations) || !Array(Root, TEXT("nodes"), Nodes))
		{
			return false;
		}
		FValues Extensions;
		if (!Array(Root, TEXT("extensionsUsed"), Extensions))
		{
			return false;
		}
		for (const auto& Extension : Extensions)
		{
			FString Name;
			if (!Extension.IsValid() || !Extension->TryGetString(Name))
			{
				return Fail(TEXT("Invalid glTF extension name."));
			}
			if (Name == TEXT("KHR_draco_mesh_compression") || Name == TEXT("EXT_meshopt_compression"))
			{
				return Fail(TEXT("Compressed glTF geometry is unsupported by scene normalization."));
			}
		}
		Binary.SetNum(Buffers.Num());
		TSet<FString> BufferPaths;
		for (int32 i = 0; i < Buffers.Num(); ++i)
		{
			FObject Buffer = Object(Buffers, i);
			FString Uri;
			if (!Buffer || !Buffer->TryGetStringField(TEXT("uri"), Uri) || !Integer(Buffer, TEXT("byteLength"), Binary[i].DeclaredLength) || Binary[i].DeclaredLength <= 0 || !LocalPath(Uri, Binary[i].Path))
			{
				return Fail(Error.IsEmpty() ? TEXT("Invalid external glTF buffer.") : Error);
			}
			if (BufferPaths.Contains(Binary[i].Path.ToLower()))
			{
				return Fail(TEXT("Multiple glTF buffers share a URI; normalization is unsupported."));
			}
			BufferPaths.Add(Binary[i].Path.ToLower());
			if (!FFileHelper::LoadFileToArray(Binary[i].Bytes, *Binary[i].Path) || Binary[i].Bytes.Num() < Binary[i].DeclaredLength)
			{
				return Fail(TEXT("glTF buffer is shorter than its declared byteLength: ") + Uri);
			}
		}
		for (int32 i = 0; i < Views.Num(); ++i)
		{
			int32 Buffer = 0, Offset = 0, Stride = 0;
			if (!ViewRange(i, 0, 0, 0, Buffer, Offset, Stride, true))
			{
				return false;
			}
		}
		Spans.SetNum(Accessors.Num());
		for (int32 i = 0; i < Accessors.Num(); ++i)
		{
			if (!ReadSpan(i, Spans[i]))
			{
				return false;
			}
		}
		for (int32 i = 0; i < Images.Num(); ++i)
		{
			FObject Image = Object(Images, i);
			FString Uri, Path;
			int32 View = 0;
			if (!Image)
			{
				return false;
			}
			if (Image->HasField(TEXT("uri")))
			{
				if (!RepairImageUri(Image) || !Image->TryGetStringField(TEXT("uri"), Uri) || !LocalPath(Uri, Path))
				{
					return false;
				}
			}
			else if (!Index(Image, TEXT("bufferView"), Views.Num(), View))
			{
				return false;
			}
		}
		FValues Textures, Samplers;
		if (!Array(Root, TEXT("textures"), Textures) || !Array(Root, TEXT("samplers"), Samplers))
		{
			return false;
		}
		for (int32 i = 0; i < Textures.Num(); ++i)
		{
			FObject Texture = Object(Textures, i);
			int32 Image = 0, Sampler = INDEX_NONE;
			if (!Texture || !Index(Texture, TEXT("source"), Images.Num(), Image) || !Index(Texture, TEXT("sampler"), Samplers.Num(), Sampler, false))
			{
				return Fail(TEXT("Invalid glTF texture image or sampler reference."));
			}
		}
		for (int32 i = 0; i < Materials.Num(); ++i)
		{
			FObject Material = Object(Materials, i);
			FValues Factors;
			if (!Material || !TextureReferences(Material, Textures.Num()) || !Array(Material, TEXT("emissiveFactor"), Factors))
			{
				return false;
			}
			if (!Material->HasField(TEXT("emissiveFactor")))
			{
				continue;
			}
			if (Factors.Num() != 3)
			{
				return Fail(TEXT("glTF emissiveFactor must contain three values."));
			}
			bool Changed = false;
			for (auto& Value : Factors)
			{
				double Number = 0;
				if (!Value.IsValid() || Value->Type != EJson::Number || !Value->TryGetNumber(Number) || !FMath::IsFinite(Number))
				{
					return Fail(TEXT("Invalid glTF emissiveFactor."));
				}
				const double Clamped = FMath::Clamp(Number, 0.0, 1.0);
				Changed |= Clamped != Number;
				Value = MakeShared<FJsonValueNumber>(Clamped);
			}
			if (Changed)
			{
				Material->SetArrayField(TEXT("emissiveFactor"), Factors);
				++ClampedMaterials;
			}
		}
		TArray<bool> EmptyMeshes;
		EmptyMeshes.Init(false, Meshes.Num());
		for (int32 i = 0; i < Meshes.Num(); ++i)
		{
			FObject Mesh = Object(Meshes, i);
			FValues Primitives, Kept;
			if (!Mesh || !Array(Mesh, TEXT("primitives"), Primitives, true))
			{
				return false;
			}
			for (int32 p = 0; p < Primitives.Num(); ++p)
			{
				FObject Primitive = Object(Primitives, p);
				const FObject* Attributes = nullptr;
				int32 Position = 0, Indices = INDEX_NONE, Normal = INDEX_NONE, Tangent = INDEX_NONE, Material = INDEX_NONE;
				if (!Primitive || !Primitive->TryGetObjectField(TEXT("attributes"), Attributes) || !Index(*Attributes, TEXT("POSITION"), Accessors.Num(), Position) || !Index(Primitive, TEXT("indices"), Accessors.Num(), Indices, false) || !Index(Primitive, TEXT("material"), Materials.Num(), Material, false))
				{
					return Fail(TEXT("Malformed glTF primitive references."));
				}
				const int32 Count = Spans[Position].Count;
				if (Count == 0 || (Indices != INDEX_NONE && Spans[Indices].Count == 0))
				{
					++RemovedPrimitives;
					continue;
				}
				int32 Mode = 4;
				const int32 DrawCount = Indices == INDEX_NONE ? Count : Spans[Indices].Count;
				if (!Integer(Primitive, TEXT("mode"), Mode, 4) || Mode > 6 || (Mode == 1 && DrawCount % 2 != 0) || (Mode == 4 && DrawCount % 3 != 0) || (Mode == 2 && DrawCount < 2) || (Mode == 3 && DrawCount < 2) || (Mode >= 5 && DrawCount < 3))
				{
					return Fail(TEXT("Invalid glTF primitive mode or element count."));
				}
				FAccessorSpan* PositionSpan = nullptr;
				if (!Direct(Position, TEXT("VEC3"), 5126, PositionSpan))
				{
					return false;
				}
				float Min[] = { MAX_flt, MAX_flt, MAX_flt }, Max[] = { -MAX_flt, -MAX_flt, -MAX_flt };
				for (int32 v = 0; v < Count; ++v)
				{
					for (int32 c = 0; c < 3; ++c)
					{
						const float Value = Float(*PositionSpan, v, c);
						if (!FMath::IsFinite(Value))
						{
							return Fail(TEXT("Non-finite glTF POSITION data."));
						}
						Min[c] = FMath::Min(Min[c], Value);
						Max[c] = FMath::Max(Max[c], Value);
					}
				}
				FValues MinValues, MaxValues;
				for (int32 c = 0; c < 3; ++c)
				{
					MinValues.Add(MakeShared<FJsonValueNumber>(Min[c]));
					MaxValues.Add(MakeShared<FJsonValueNumber>(Max[c]));
				}
				Object(Accessors, Position)->SetArrayField(TEXT("min"), MinValues);
				Object(Accessors, Position)->SetArrayField(TEXT("max"), MaxValues);
				for (const auto& Attribute : (*Attributes)->Values)
				{
					int32 Accessor = 0;
					if (!Index(*Attributes, Attribute.Key, Accessors.Num(), Accessor) || Spans[Accessor].Count != Count)
					{
						return Fail(TEXT("glTF primitive attributes have inconsistent vertex counts."));
					}
				}
				if (Indices != INDEX_NONE && !PromoteIndex(Indices, Count))
				{
					return false;
				}
				if (!Index(*Attributes, TEXT("NORMAL"), Accessors.Num(), Normal, false) || !Index(*Attributes, TEXT("TANGENT"), Accessors.Num(), Tangent, false))
				{
					return false;
				}
				if (Tangent != INDEX_NONE && !RepairTangents(Tangent, Normal, Count))
				{
					return false;
				}
				Kept.Add(Primitives[p]);
			}
			Mesh->SetArrayField(TEXT("primitives"), Kept);
			EmptyMeshes[i] = Kept.Num() == 0;
		}
		TArray<int32> MeshMap;
		Compact(Meshes, EmptyMeshes, MeshMap);
		if (Meshes.Num() == 0)
		{
			return Fail(TEXT("Scene export contains no usable mesh geometry. Load and show its content sublevels before exporting."));
		}
		TSet<int32> RemovedMeshNodes;
		for (int32 i = 0; i < Nodes.Num(); ++i)
		{
			FObject Node = Object(Nodes, i);
			int32 Mesh = INDEX_NONE;
			if (!Node || !Index(Node, TEXT("mesh"), MeshMap.Num(), Mesh, false))
			{
				return false;
			}
			if (Mesh != INDEX_NONE)
			{
				if (MeshMap[Mesh] == INDEX_NONE)
				{
					Node->RemoveField(TEXT("mesh"));
					Node->RemoveField(TEXT("skin"));
					Node->RemoveField(TEXT("weights"));
					RemovedMeshNodes.Add(i);
				}
				else
				{
					Node->SetNumberField(TEXT("mesh"), MeshMap[Mesh]);
				}
			}
		}
		TArray<bool> EmptyAccessors;
		EmptyAccessors.Init(false, Accessors.Num());
		for (int32 i = 0; i < Accessors.Num(); ++i)
		{
			EmptyAccessors[i] = Spans[i].Count == 0;
		}
		TArray<int32> AccessorMap;
		Compact(Accessors, EmptyAccessors, AccessorMap);
		for (int32 i = 0; i < Meshes.Num(); ++i)
		{
			FValues Primitives;
			if (!Array(Object(Meshes, i), TEXT("primitives"), Primitives, true))
			{
				return false;
			}
			for (int32 p = 0; p < Primitives.Num(); ++p)
			{
				FObject Primitive = Object(Primitives, p);
				const FObject* Attributes = nullptr;
				FValues Targets;
				if (!Primitive->TryGetObjectField(TEXT("attributes"), Attributes) || !RemapAttributes(*Attributes, AccessorMap) || !Remap(Primitive, TEXT("indices"), AccessorMap) || !Array(Primitive, TEXT("targets"), Targets))
				{
					return false;
				}
				for (int32 t = 0; t < Targets.Num(); ++t)
				{
					if (!RemapAttributes(Object(Targets, t), AccessorMap))
					{
						return false;
					}
				}
			}
		}
		for (int32 i = 0; i < Skins.Num(); ++i)
		{
			if (!Remap(Object(Skins, i), TEXT("inverseBindMatrices"), AccessorMap))
			{
				return false;
			}
		}
		for (int32 i = 0; i < Animations.Num(); ++i)
		{
			FValues AnimationSamplers;
			if (!Array(Object(Animations, i), TEXT("samplers"), AnimationSamplers, true))
			{
				return false;
			}
			for (int32 s = 0; s < AnimationSamplers.Num(); ++s)
			{
				if (!Remap(Object(AnimationSamplers, s), TEXT("input"), AccessorMap, true) || !Remap(Object(AnimationSamplers, s), TEXT("output"), AccessorMap, true))
				{
					return false;
				}
			}
			FValues Channels;
			if (!Array(Object(Animations, i), TEXT("channels"), Channels, true))
			{
				return false;
			}
			for (int32 c = 0; c < Channels.Num(); ++c)
			{
				FObject Channel = Object(Channels, c);
				const FObject* Target = nullptr;
				FString Path;
				int32 Node = INDEX_NONE;
				if (!Channel || !Channel->TryGetObjectField(TEXT("target"), Target) || !(*Target)->TryGetStringField(TEXT("path"), Path) || !Index(*Target, TEXT("node"), Nodes.Num(), Node, false))
				{
					return Fail(TEXT("Invalid glTF animation target."));
				}
				if (Path == TEXT("weights") && RemovedMeshNodes.Contains(Node))
				{
					return Fail(TEXT("Cannot preserve a weights animation targeting a removed empty mesh."));
				}
			}
		}
		TArray<bool> EmptyViews;
		EmptyViews.Init(false, Views.Num());
		for (int32 i = 0; i < Views.Num(); ++i)
		{
			int32 Length = 0;
			if (!Integer(Object(Views, i), TEXT("byteLength"), Length))
			{
				return false;
			}
			EmptyViews[i] = Length == 0;
		}
		TArray<int32> ViewMap;
		Compact(Views, EmptyViews, ViewMap);
		for (int32 i = 0; i < Accessors.Num(); ++i)
		{
			FObject Accessor = Object(Accessors, i);
			const FObject* Sparse = nullptr;
			if (!Remap(Accessor, TEXT("bufferView"), ViewMap))
			{
				return false;
			}
			if (Accessor->HasField(TEXT("sparse")))
			{
				const FObject* Indices = nullptr;
				const FObject* Values = nullptr;
				if (!Accessor->TryGetObjectField(TEXT("sparse"), Sparse) || !(*Sparse)->TryGetObjectField(TEXT("indices"), Indices) || !(*Sparse)->TryGetObjectField(TEXT("values"), Values) || !Remap(*Indices, TEXT("bufferView"), ViewMap, true) || !Remap(*Values, TEXT("bufferView"), ViewMap, true))
				{
					return false;
				}
			}
		}
		for (int32 i = 0; i < Images.Num(); ++i)
		{
			if (!Remap(Object(Images, i), TEXT("bufferView"), ViewMap))
			{
				return false;
			}
		}
		Root->SetArrayField(TEXT("buffers"), Buffers);
		Root->SetArrayField(TEXT("bufferViews"), Views);
		Root->SetArrayField(TEXT("accessors"), Accessors);
		Root->SetArrayField(TEXT("meshes"), Meshes);
		for (int32 i = 0; i < Accessors.Num(); ++i)
		{
			FAccessorSpan Span;
			if (!ReadSpan(i, Span))
			{
				return false;
			}
		}
		return true;
	}
};
}

bool CognitiveSceneExportUtils::NormalizeGLTFForUpload(const FString& FilePath, FString& OutError)
{
	OutError.Reset();
	FNormalizer Normalizer;
	FString Source;
	Normalizer.Directory = FPaths::ConvertRelativePathToFull(FPaths::GetPath(FilePath));
	FPaths::NormalizeDirectoryName(Normalizer.Directory);
	if (!FFileHelper::LoadFileToString(Source, *FilePath) || !FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Source), Normalizer.Root) || !Normalizer.Root.IsValid())
	{
		OutError = TEXT("Cannot read or parse exported glTF: ") + FilePath;
		return false;
	}
	if (!Normalizer.Normalize())
	{
		OutError = Normalizer.Error.IsEmpty() ? TEXT("Malformed or unsupported exported glTF.") : Normalizer.Error;
		return false;
	}
	FString Output;
	if (!FJsonSerializer::Serialize(Normalizer.Root.ToSharedRef(), TJsonWriterFactory<>::Create(&Output)))
	{
		OutError = TEXT("Cannot serialize normalized glTF.");
		return false;
	}
	struct FCommitFile
	{
		FString Original, Temporary, Backup;
	};
	TArray<FCommitFile> Files;
	const FString Suffix = TEXT(".c3d-") + FGuid::NewGuid().ToString(EGuidFormats::Digits);
	auto AddFile = [&Files, &Suffix](const FString& Path)
	{
		FCommitFile File;
		File.Original = Path;
		File.Temporary = Path + Suffix + TEXT(".tmp");
		File.Backup = Path + Suffix + TEXT(".bak");
		Files.Add(File);
	};
	auto Cleanup = [&Files]()
	{
		for (const FCommitFile& File : Files)
		{
			IFileManager::Get().Delete(*File.Temporary, false, false, true);
			IFileManager::Get().Delete(*File.Backup, false, false, true);
		}
	};
	for (const FBufferCopy& Buffer : Normalizer.Binary)
	{
		if (!Buffer.Dirty)
		{
			continue;
		}
		AddFile(Buffer.Path);
		if (!FFileHelper::SaveArrayToFile(Buffer.Bytes, *Files.Last().Temporary))
		{
			OutError = TEXT("Cannot stage normalized glTF buffer: ") + Buffer.Path;
			Cleanup();
			return false;
		}
	}
	AddFile(FilePath);
	if (!FFileHelper::SaveStringToFile(Output, *Files.Last().Temporary, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
	{
		OutError = TEXT("Cannot stage normalized glTF: ") + FilePath;
		Cleanup();
		return false;
	}
	for (const FCommitFile& File : Files)
	{
		if (IFileManager::Get().Copy(*File.Backup, *File.Original, false) != COPY_OK)
		{
			OutError = TEXT("Cannot preserve original glTF export: ") + File.Original;
			Cleanup();
			return false;
		}
	}
	for (int32 i = 0; i < Files.Num(); ++i)
	{
		const FCommitFile& File = Files[i];
		if (IFileManager::Get().Move(*File.Original, *File.Temporary, true, false, false, true))
		{
			continue;
		}
		bool Restored = true;
		for (int32 j = 0; j <= i; ++j)
		{
			Restored &= IFileManager::Get().Copy(*Files[j].Original, *Files[j].Backup, true) == COPY_OK;
		}
		OutError = TEXT("Cannot commit normalized glTF export: ") + File.Original;
		OutError += Restored ? TEXT(". Original export restored. Recovery copies retained at:") : TEXT(". Recovery copies and staged files retained; originals are available at:");
		for (const FCommitFile& Recovery : Files)
		{
			OutError += TEXT("\n") + Recovery.Backup;
		}
		return false;
	}
	Cleanup();
	UE_LOG(LogCognitiveSceneExport, Log, TEXT("Validated scene export %s; removed %d empty primitives, promoted %d index accessors, repaired %d zero tangents, clamped %d emissive materials, renamed %d non-ASCII images."), *FilePath, Normalizer.RemovedPrimitives, Normalizer.PromotedIndices, Normalizer.FixedTangents, Normalizer.ClampedMaterials, Normalizer.RenamedImages);
	return true;
}

bool CognitiveSceneExportUtils::UseMetricSceneUnits()
{
	const FString SettingsPath = FPaths::Combine(FPaths::ProjectConfigDir(), TEXT("c3dlocal"), TEXT("Cognitive3DSettings.ini"));
	FConfigFile Settings;
	Settings.Read(SettingsPath);
	FString Value;
	return Settings.GetString(TEXT("/Script/Cognitive3D.Cognitive3DSettings"), TEXT("MetricSceneUnits"), Value) && Value.ToBool();
}

float CognitiveSceneExportUtils::GetExportUniformScale()
{
	return UseMetricSceneUnits() ? 0.01f : 1.0f;
}

int32 CognitiveSceneExportUtils::GetSceneSettingsScale()
{
	return UseMetricSceneUnits() ? 1 : 100;
}
