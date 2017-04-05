
#include "CognitiveVREditorPrivatePCH.h"
#include "BaseEditorToolCustomization.h"
#include "CognitiveVRSettings.h"

#define LOCTEXT_NAMESPACE "BaseToolEditor"

void FBaseEditorToolCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TSet<UClass*> Classes;

	TArray<TWeakObjectPtr<UObject>> ObjectsBeingCustomized;
	DetailBuilder.GetObjectsBeingCustomized(ObjectsBeingCustomized);

	DetailLayoutPtr = &DetailBuilder;

	UClass* Class = NULL;

	for (auto WeakObject : ObjectsBeingCustomized)
	{
		if (UObject* Instance = WeakObject.Get())
		{
			Class = Instance->GetClass();
			break;
		}
	}

	IDetailCategoryBuilder& SettingsCategory = DetailBuilder.EditCategory(TEXT("Export Settings"));

	MinPolygonProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UCognitiveVRSettings, MinPolygons));
	MaxPolygonProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UCognitiveVRSettings, MaxPolygons));
	StaticOnlyProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UCognitiveVRSettings, staticOnly));
	MinSizeProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UCognitiveVRSettings, MinimumSize));
	MaxSizeProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UCognitiveVRSettings, MaximumSize));
	TextureResizeProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UCognitiveVRSettings, TextureResizeFactor));

	SettingsCategory.AddProperty(MinPolygonProperty);
	SettingsCategory.AddProperty(MaxPolygonProperty);
	SettingsCategory.AddProperty(StaticOnlyProperty);
	SettingsCategory.AddProperty(MinSizeProperty);
	SettingsCategory.AddProperty(MaxSizeProperty);
	SettingsCategory.AddProperty(TextureResizeProperty);

	// Create a commands category
	IDetailCategoryBuilder& Category = DetailBuilder.EditCategory(TEXT("Commands"));

	FText p = GetBlenderPath();
	if (p.EqualTo(FText::FromString("")) && !HasSearchedForBlender)
	{
		HasSearchedForBlender = true;
		UE_LOG(LogTemp, Warning, TEXT("blender path is empty. search for blender"));
		SearchForBlender();
	}

	Category.AddCustomRow(FText::FromString("Select Blender Horizontal"))
		.ValueContent()
		.HAlign(HAlign_Fill)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
		//.HAlign(HAlign_Right)
		.VAlign(VAlign_Center)
		.Padding(FMargin(0.0f, 0.0f, 30.0f, 0.0f))
		[
			SNew(SButton)
			.IsEnabled(true)
		.Text(FText::FromString("Select Blender"))
		.OnClicked(this, &FBaseEditorToolCustomization::Select_Blender)
		]

	+ SHorizontalBox::Slot()
		.Padding(FMargin(4.0f, 0.0f, 30.0f, 0.0f))
		[
			SNew(STextBlock)
			.Text(this, &FBaseEditorToolCustomization::GetBlenderPath)
		]
		];

	//select export meshes
	Category.AddCustomRow(FText::FromString("Commands"))
		.ValueContent()
		[
			SNew(SButton)
			//.IsEnabled(&FBaseEditorToolCustomization::HasFoundBlender.Get() || ButtonCaption.EqualTo(FText::FromString("Select Blender")))
		.IsEnabled(this, &FBaseEditorToolCustomization::HasFoundBlender)
		.Text(FText::FromString("Select Export Meshes"))
		.OnClicked(this, &FBaseEditorToolCustomization::Select_Export_Meshes)
		];

	//export selected scene
	Category.AddCustomRow(FText::FromString("Commands"))
		.ValueContent()
		[
			SNew(SButton)
			//.IsEnabled(&FBaseEditorToolCustomization::HasFoundBlender.Get() || ButtonCaption.EqualTo(FText::FromString("Select Blender")))
		.IsEnabled(this, &FBaseEditorToolCustomization::HasFoundBlender)
		.Text(FText::FromString("Export Selected"))
		.OnClicked(this, &FBaseEditorToolCustomization::Export_Selected)
		];

	//export whole scene
	Category.AddCustomRow(FText::FromString("Commands"))
		.ValueContent()
		[
			SNew(SButton)
			//.IsEnabled(&FBaseEditorToolCustomization::HasFoundBlender.Get() || ButtonCaption.EqualTo(FText::FromString("Select Blender")))
		.IsEnabled(this, &FBaseEditorToolCustomization::HasFoundBlender)
		.Text(FText::FromString("Export All"))
		.OnClicked(this, &FBaseEditorToolCustomization::Export_All)
		];

	Category.AddCustomRow(FText::FromString("Select Export Directory Horizontal"))
		.ValueContent()
		.HAlign(HAlign_Fill)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
		//.HAlign(HAlign_Right)
		.VAlign(VAlign_Center)
		.Padding(FMargin(0.0f, 0.0f, 30.0f, 0.0f))
		[
			SNew(SButton)
			.IsEnabled(true)
		.Text(FText::FromString("Select Export Directory"))
		.OnClicked(this, &FBaseEditorToolCustomization::Select_Export_Directory)
		]

	+ SHorizontalBox::Slot()
		.Padding(FMargin(4.0f, 0.0f, 30.0f, 0.0f))
		[
			SNew(STextBlock)
			.Text(this, &FBaseEditorToolCustomization::GetExportDirectory)
		]
		];

	//List Materials
	Category.AddCustomRow(FText::FromString("Commands"))
		.ValueContent()
		[
			SNew(SButton)
			//.IsEnabled(&FBaseEditorToolCustomization::HasFoundBlender.Get() || ButtonCaption.EqualTo(FText::FromString("Select Blender")))
		.IsEnabled(this, &FBaseEditorToolCustomization::HasFoundBlenderAndExportDir)
		.Text(FText::FromString("Export Transparent Textures"))
		.OnClicked(this, &FBaseEditorToolCustomization::List_Materials)
		];

	//Reduce Meshes
	Category.AddCustomRow(FText::FromString("Commands"))
		.ValueContent()
		[
			SNew(SButton)
			//.IsEnabled(&FBaseEditorToolCustomization::HasFoundBlender.Get() || ButtonCaption.EqualTo(FText::FromString("Select Blender")))
		.IsEnabled(this, &FBaseEditorToolCustomization::HasFoundBlenderAndExportDir)
		.Text(FText::FromString("Reduce Meshes"))
		.OnClicked(this, &FBaseEditorToolCustomization::Reduce_Meshes)
		];

	//Reduce Textures
	Category.AddCustomRow(FText::FromString("Commands"))
		.ValueContent()
		[
			SNew(SButton)
			//.IsEnabled(&FBaseEditorToolCustomization::HasFoundBlender.Get() || ButtonCaption.EqualTo(FText::FromString("Select Blender")))
		.IsEnabled(this, &FBaseEditorToolCustomization::HasFoundBlenderAndExportDir)
		.Text(FText::FromString("Reduce Textures"))
		.OnClicked(this, &FBaseEditorToolCustomization::Reduce_Textures)
		];



	//upload scene
	Category.AddCustomRow(FText::FromString("Commands"))
		.ValueContent()
		[
			SNew(SButton)
			.IsEnabled(this, &FBaseEditorToolCustomization::HasFoundBlenderAndExportDir)
			.Text(FText::FromString("Upload Scene"))
			.OnClicked(this, &FBaseEditorToolCustomization::UploadScene)
		];

	//upload scene
	/*Category.AddCustomRow(FText::FromString("Commands"))
		.ValueContent()
		[
			SNew(SButton)
			.IsEnabled(true)
			.Text(FText::FromString("Test Ini Write"))
			.OnClicked(this, &FBaseEditorToolCustomization::DebugSendSceneData)
		];*/

	//IDetailCategoryBuilder& SceneKeyCategory = DetailBuilder.EditCategory(TEXT("Scene Keys"));

	//SceneKeysProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UCognitiveVRSettings, SceneKeyPair));

	//SceneKeyCategory.AddProperty(SceneKeysProperty);
}

float FBaseEditorToolCustomization::GetMinimumSize()
{
	float MinSize = 0;
	MinSizeProperty->GetValue(MinSize);
	return MinSize;
}

float FBaseEditorToolCustomization::GetMaximumSize()
{
	float MaxSize = 0;
	MaxSizeProperty->GetValue(MaxSize);
	return MaxSize;
}

bool FBaseEditorToolCustomization::GetStaticOnly()
{
	bool staticOnly = false;
	StaticOnlyProperty->GetValue(staticOnly);
	return staticOnly;
}

int FBaseEditorToolCustomization::GetMinPolygon()
{
	int MinCount = 0;
	MinPolygonProperty->GetValue(MinCount);
	return MinCount;
}

int FBaseEditorToolCustomization::GetMaxPolygon()
{
	int MaxCount = 0;
	MaxPolygonProperty->GetValue(MaxCount);
	return MaxCount;
}

int FBaseEditorToolCustomization::GetTextureRefacor()
{
	int TextureRefactor = 0;
	TextureResizeProperty->GetValue(TextureRefactor);
	return TextureRefactor;
}

FReply FBaseEditorToolCustomization::Export_Selected()
{
	FEditorFileUtils::Export(true);

	ExportDirectory = FEditorDirectories::Get().GetLastDirectory(ELastDirectory::UNR);

	ExportDirectory = FPaths::ConvertRelativePathToFull(ExportDirectory);

	return FReply::Handled();
}

FReply FBaseEditorToolCustomization::Export_All()
{
	FEditorFileUtils::Export(false);

	ExportDirectory = FEditorDirectories::Get().GetLastDirectory(ELastDirectory::UNR);

	ExportDirectory = FPaths::ConvertRelativePathToFull(ExportDirectory);

	return FReply::Handled();
}

FReply FBaseEditorToolCustomization::Select_Export_Meshes()
{
	UWorld* tempworld = GEditor->GetEditorWorldContext().World();

	if (!tempworld)
	{
		UE_LOG(LogTemp, Warning, TEXT("world is null"));
		return FReply::Handled();
	}

	//also grab landscapes

	GEditor->GetSelectedActors()->DeselectAll();
	GEditor->GetSelectedObjects()->DeselectAll();

	int32 ActorsExported = 0;

	for (TActorIterator<AStaticMeshActor> ObstacleItr(tempworld); ObstacleItr; ++ObstacleItr)
	{
		//get non-moveable static meshes only
		if (GetStaticOnly())
		{
			EComponentMobility::Type mobility = EComponentMobility::Static;
			const USceneComponent* sc = Cast<USceneComponent>(ObstacleItr->GetStaticMeshComponent());
			if (sc == NULL) { continue; }
			if (sc->Mobility == EComponentMobility::Movable) { continue; }
		}

		//get meshes in size range
		FVector origin;
		FVector boxBounds;

		ObstacleItr->GetActorBounds(false, origin, boxBounds);
		double magnitude = FMath::Sqrt(boxBounds.X*boxBounds.X + boxBounds.Y*boxBounds.Y + boxBounds.Z*boxBounds.Z);

		if (magnitude < GetMinimumSize())
		{
			continue;
		}

		if (magnitude > GetMaximumSize())
		{
			continue;
		}

		//get the selectable bit
		AStaticMeshActor *tempactor = *ObstacleItr;
		if (!tempactor)
		{
			continue;
		}



		GEditor->SelectActor(tempactor, true, false, true);
		ActorsExported++;
	}
	UE_LOG(LogTemp, Warning, TEXT("Found %d Static Meshes for Export"), ActorsExported);
	return FReply::Handled();
}


//open fiel type
FReply FBaseEditorToolCustomization::Select_Blender()
{
	FString title = "Select Blender.exe";
	FString fileTypes = ".exe";
	FString lastPath = FEditorDirectories::Get().GetLastDirectory(ELastDirectory::UNR);
	FString defaultfile = FString();
	FString outFilename = FString();
	if (PickFile(title, fileTypes, lastPath, defaultfile, outFilename))
	{
		BlenderPath = outFilename;
		//UE_LOG(LogTemp, Warning, TEXT("selected blender at path: %s"), *BlenderPath);
	}
	return FReply::Handled();
}

//open fiel type
FReply FBaseEditorToolCustomization::Select_Export_Directory()
{
	FString title = "Select Export Directory";
	FString fileTypes = ".exe";
	FString lastPath = FEditorDirectories::Get().GetLastDirectory(ELastDirectory::UNR);
	FString defaultfile = FString();
	FString outFilename = FString();
	if (PickDirectory(title, fileTypes, lastPath, defaultfile, outFilename))
	{
		UE_LOG(LogTemp, Warning, TEXT("FBaseEditorToolCustomization::Select_Export_Directory - picked a directory"));
		ExportDirectory = outFilename;
	}
	return FReply::Handled();
}

bool FBaseEditorToolCustomization::PickFile(const FString& Title, const FString& FileTypes, FString& InOutLastPath, const FString& DefaultFile, FString& OutFilename)
{
	OutFilename = FString();

	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	bool bFileChosen = false;
	TArray<FString> OutFilenames;
	if (DesktopPlatform)
	{
		void* ParentWindowWindowHandle = ChooseParentWindowHandle();

		bFileChosen = DesktopPlatform->OpenFileDialog(
			ParentWindowWindowHandle,
			Title,
			InOutLastPath,
			DefaultFile,
			FileTypes,
			EFileDialogFlags::None,
			OutFilenames
		);
	}

	bFileChosen = (OutFilenames.Num() > 0);

	if (bFileChosen)
	{
		// User successfully chose a file; remember the path for the next time the dialog opens.
		InOutLastPath = OutFilenames[0];
		OutFilename = OutFilenames[0];
	}

	return bFileChosen;
}

bool FBaseEditorToolCustomization::PickDirectory(const FString& Title, const FString& FileTypes, FString& InOutLastPath, const FString& DefaultFile, FString& OutFilename)
{
	OutFilename = FString();

	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	bool directoryChosen = false;
	TArray<FString> OutFilenames;
	if (DesktopPlatform)
	{
		void* ParentWindowWindowHandle = ChooseParentWindowHandle();

		directoryChosen = DesktopPlatform->OpenDirectoryDialog(
			ParentWindowWindowHandle,
			Title,
			InOutLastPath,
			OutFilename
		);
	}

	return directoryChosen;
}

void* FBaseEditorToolCustomization::ChooseParentWindowHandle()
{
	void* ParentWindowWindowHandle = NULL;
	IMainFrameModule& MainFrameModule = FModuleManager::LoadModuleChecked<IMainFrameModule>(TEXT("MainFrame"));
	const TSharedPtr<SWindow>& MainFrameParentWindow = MainFrameModule.GetParentWindow();
	if (MainFrameParentWindow.IsValid() && MainFrameParentWindow->GetNativeWindow().IsValid())
	{
		ParentWindowWindowHandle = MainFrameParentWindow->GetNativeWindow()->GetOSWindowHandle();
	}

	return ParentWindowWindowHandle;
}

FReply FBaseEditorToolCustomization::List_Materials()
{
	//look at export directory. find mtl file

	FString result;
	FString Ext;
	//get all files in the export directory
	IFileManager& FileManager = IFileManager::Get();
	TArray<FString> Files;

	if (Ext == "")
	{
		Ext = "*.*";
	}
	else
	{
		Ext = (Ext.Left(1) == ".") ? "*" + Ext : "*." + Ext;
	}

	FString FinalPath = ExportDirectory + "/" + Ext;
	FileManager.FindFiles(Files, *FinalPath, true, false);

	TArray<FColor> colors;
	FIntPoint point = FIntPoint(256, 256);

	for (int32 i = 0; i < Files.Num(); i++)
	{
		if (Files[i].EndsWith(".mtl"))
		{

			FString fullPath = ExportDirectory + "/" + Files[i];
			GLog->Log("MATERIAL " + fullPath);
			FString contents;

			if (FFileHelper::LoadFileToString(contents, *fullPath))
			{
				GLog->Log("loaded " + Files[i]);
				GLog->Log(contents);
			}
			else
			{
				GLog->Log("failed to load " + Files[i]);
			}
			TArray<FString> lines;
			int32 lineCount = contents.ParseIntoArray(lines, TEXT("\n"), true);

			//GLog->Log("material line count " + lineCount);
			for (int32 j = 0; j < lines.Num(); j++)
			{
				if (lines[j].Contains("newmtl"))
				{
					TArray<FString> matDirectories;
					lines[j].RemoveFromStart("newmtl ");
					lines[j].ParseIntoArray(matDirectories, TEXT("/"));

					TArray<FString> lineParts;
					matDirectories[matDirectories.Num() - 1].ParseIntoArray(lineParts, TEXT("_"));

					int32 floor_count = FMath::FloorToInt(lineParts.Num() / (float)2);

					FString trueMatName;
					for (int32 k = 0; k < lineParts.Num(); k++)
					{
						if (k + 1 == floor_count)
						{
							trueMatName = trueMatName + lineParts[k] + ".";
						}
						else
						{
							trueMatName = trueMatName + lineParts[k] + "_";
						}
					}
					trueMatName.RemoveFromEnd("_");

					//GLog->Log("true material name " + trueMatName);

					FString finalMatPath;
					FString finalDirectory;
					for (int32 l = 0; l < matDirectories.Num() - 1; l++)
					{
						finalMatPath = finalMatPath + matDirectories[l] + "/";
						finalDirectory = finalMatPath + matDirectories[l] + "/";
					}
					finalMatPath = "/" + finalMatPath + trueMatName;
					finalMatPath.RemoveAt(finalMatPath.Len() - 1);

					FStringAssetReference assetRef = FStringAssetReference(finalMatPath);

					UObject* assetRefLoaded = assetRef.TryLoad();

					if (assetRefLoaded != NULL)
					{
						//GLog->Log("asset ref loaded! " + assetRef.ToString());
						UMaterial* m = Cast<UMaterial>(assetRefLoaded);
						UMaterialInstance* mi = Cast<UMaterialInstance>(assetRefLoaded);

						if (m != NULL)
						{
							if (m->GetBlendMode() == EBlendMode::BLEND_Opaque)
							{
								//GLog->Log("opaque material should already have textures exported correctly");
								continue;
							}

							//TODO export material property transparency
							if (FMaterialUtilities::ExportMaterialProperty(m, EMaterialProperty::MP_BaseColor, point, colors))
							{

								FString BMPFilename = ExportDirectory + finalMatPath.Replace(TEXT("."), TEXT("_")) + TEXT("_D.bmp");

								GLog->Log("++++++++++writing base color for material " + BMPFilename);
								FFileHelper::CreateBitmap(*BMPFilename, point.X, point.Y, colors.GetData());
							}
						}
						else
						{
							if (mi->GetBlendMode() == EBlendMode::BLEND_Opaque)
							{
								//GLog->Log("opaque material should already have textures exported correctly");
								continue;
							}

							//TODO export material property transparency
							if (FMaterialUtilities::ExportMaterialProperty(mi, EMaterialProperty::MP_BaseColor, point, colors))
							{

								FString BMPFilename = ExportDirectory + finalMatPath.Replace(TEXT("."), TEXT("_")) + TEXT("_D.bmp");

								FFileHelper::CreateBitmap(*BMPFilename, point.X, point.Y, colors.GetData());
							}
						}
					}
				}
			}
		}
	}

	return FReply::Handled();
}

//run this as the next step after exporting the scene
FReply FBaseEditorToolCustomization::Reduce_Meshes()
{
	FString pythonscriptpath = IPluginManager::Get().FindPlugin(TEXT("CognitiveVR"))->GetBaseDir() / TEXT("Resources") / TEXT("DecimateExportedScene.py");
	const TCHAR* charPath = *pythonscriptpath;

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(FName("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	TArray<FAssetData> ScriptList;
	if (!AssetRegistry.GetAssetsByPath(FName(*pythonscriptpath), ScriptList))
	{
		UE_LOG(LogTemp, Warning, TEXT("Reduce_Meshes - Could not find decimateall.py script at path. Canceling"));
		return FReply::Handled();
	}

	FString stringurl = BlenderPath;

	if (BlenderPath.Len() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Reduce_Meshes - No path set for Blender.exe. Canceling"));
		return FReply::Handled();
	}

	UWorld* tempworld = GEditor->GetEditorWorldContext().World();
	if (!tempworld)
	{
		UE_LOG(LogTemp, Warning, TEXT("Reduce_Meshes - World is null. canceling"));
		return FReply::Handled();
	}

	//const TCHAR* url = *stringurl;
	FString SceneName = tempworld->GetMapName();
	FString ObjPath = ExportDirectory;

	if (ObjPath.Len() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("FBaseEditorToolCustomization::Reduce_Meshes No know export directory. Canceling"));
		return FReply::Handled();
	}

	FString MinPolyCount = FString::FromInt(GetMinPolygon());
	FString MaxPolyCount = FString::FromInt(GetMaxPolygon());

	FString escapedPythonPath = pythonscriptpath.Replace(TEXT(" "), TEXT("\" \""));
	FString escapedOutPath = ObjPath.Replace(TEXT(" "), TEXT("\" \""));

	FString escapedExcludeMeshes = "IGNOREEXCLUDEMESHES";

	FConfigSection* ExportSettings = GConfig->GetSectionPrivate(TEXT("/Script/CognitiveVR.CognitiveVRSettings"), false, true, GEngineIni);
	if (ExportSettings != NULL)
	{
		for (FConfigSection::TIterator It(*ExportSettings); It; ++It)
		{
			if (It.Key() == TEXT("ExcludeMeshes"))
			{
				FName nameEscapedExcludeMeshes;
				escapedExcludeMeshes = It.Value().GetValue();
				break;
			}
		}
	}

	escapedExcludeMeshes = escapedExcludeMeshes.Replace(TEXT(" "), TEXT("\" \""));

	FString productID = GetProductID();

	FString stringparams = " -P " + escapedPythonPath + " " + escapedOutPath + " " + MinPolyCount + " " + MaxPolyCount + " " + SceneName + " " + productID + " " + COGNITIVEVR_SDK_VERSION + " " + escapedExcludeMeshes;

	FString stringParamSlashed = stringparams.Replace(TEXT("\\"), TEXT("/"));

	UE_LOG(LogTemp, Warning, TEXT("FBaseEditorToolCustomization::Reduce_Meshes Params: %s"), *stringParamSlashed);


	const TCHAR* params = *stringParamSlashed;
	int32 priorityMod = 0;
	FProcHandle procHandle = FPlatformProcess::CreateProc(*BlenderPath, params, false, false, false, NULL, priorityMod, 0, nullptr);

	//TODO when procHandle is complete, upload exported files to sceneexplorer.com
	return FReply::Handled();
}

FString FBaseEditorToolCustomization::GetProductID()
{
	FString ValueReceived;
	GConfig->GetString(
		TEXT("Analytics"),
		TEXT("CognitiveVRApiKey"),
		ValueReceived,
		GEngineIni
	);
	return ValueReceived;
}

//run this as the next step after exporting the scene
FReply FBaseEditorToolCustomization::Reduce_Textures()
{
	FString pythonscriptpath = IPluginManager::Get().FindPlugin(TEXT("CognitiveVR"))->GetBaseDir() / TEXT("Resources") / TEXT("ConvertTextures.py");
	const TCHAR* charPath = *pythonscriptpath;

	//found something
	UE_LOG(LogTemp, Warning, TEXT("Python script path: %s"), charPath);


	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(FName("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();


	TArray<FAssetData> ScriptList;
	if (!AssetRegistry.GetAssetsByPath(FName(*pythonscriptpath), ScriptList))
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not find decimateall.py script at path. Canceling"));
		return FReply::Handled();
	}

	FString stringurl = BlenderPath;

	if (BlenderPath.Len() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No path set for Blender.exe. Canceling"));
		return FReply::Handled();
	}

	UWorld* tempworld = GEditor->GetEditorWorldContext().World();
	if (!tempworld)
	{
		UE_LOG(LogTemp, Warning, TEXT("World is null. canceling"));
		return FReply::Handled();
	}

	//const TCHAR* url = *stringurl;
	FString SceneName = tempworld->GetMapName();
	FString ObjPath = ExportDirectory;

	if (ObjPath.Len() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No know export directory. Canceling"));
		return FReply::Handled();
	}


	FString MaxPolyCount = FString::FromInt(0);
	FString resizeFactor = FString::FromInt(GetTextureRefacor());

	FString escapedPythonPath = pythonscriptpath.Replace(TEXT(" "), TEXT("\" \""));
	FString escapedOutPath = ObjPath.Replace(TEXT(" "), TEXT("\" \""));

	FString stringparams = " -P " + escapedPythonPath + " " + escapedOutPath + " " + resizeFactor + " " + MaxPolyCount + " " + SceneName;

	FString stringParamSlashed = stringparams.Replace(TEXT("\\"), TEXT("/"));

	UE_LOG(LogTemp, Warning, TEXT("Params: %s"), *stringParamSlashed);


	const TCHAR* params = *stringParamSlashed;
	int32 priorityMod = 0;
	FProcHandle procHandle = FPlatformProcess::CreateProc(*BlenderPath, params, false, false, false, NULL, priorityMod, 0, nullptr);

	//FString cmdPath = "C:\\Windows\\System32\\cmd.exe";
	//FString cmdPathS = "cmd.exe";
	//FProcHandle procHandle = FPlatformProcess::CreateProc(*cmdPath, NULL, false, false, false, NULL, priorityMod, 0, nullptr);

	//TODO can i just create a process and add parameters or do i need to run through cmd line??
	//system("cmd.exe");

	//TODO when procHandle is complete, upload exported files to sceneexplorer.com
	return FReply::Handled();
}

FReply FBaseEditorToolCustomization::Http_Request()
{
	//TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	//JsonObject->SetStringField(TEXT("some_string_field"), *FString::Printf(TEXT("%s"), *SomeFStringVariable));

	//FString OutputString;

	//TSharedRef<TJsonWriter<TCHAR>> JsonWriter = TJsonWriterFactory<>::Create(&OutputString);

	//FJsonSerializer::Serialize(JsonObject.ToSharedRef(), JsonWriter);

	TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();

	HttpRequest->SetVerb("GET");

	//HttpRequest->SetHeader("Content-Type", "application/json");

	HttpRequest->SetURL("https://s3.amazonaws.com/cvr-test/sdkversion.txt");

	//HttpRequest->SetContentAsString(OutputString);

	//HttpRequest->OnProcessRequestComplete().BindUObject(this, &FBaseEditorToolCustomization::OnYourFunctionCompleted);
	//HttpRequest->OnProcessRequestComplete().BindSP(this, &FBaseEditorToolCustomization::OnYourFunctionCompleted);

	HttpRequest->ProcessRequest();
	return FReply::Handled();
}

FReply FBaseEditorToolCustomization::UploadScene()
{
	//TODO get all the exported files
	//make a multipart form

	FString filesStartingWith = TEXT("");
	FString pngextension = TEXT("png");
	//FString fileExtensions = TEXT("obj");
	TArray<FString> filesInDirectory = GetAllFilesInDirectory(ExportDirectory, true, filesStartingWith, filesStartingWith, pngextension);

	TArray<FString> imagesInDirectory = GetAllFilesInDirectory(ExportDirectory, true, filesStartingWith, pngextension, filesStartingWith);

	//FString httpbody;
	//FString Content;
	//TArray<uint8> ContentBytes;
	TArray<FContentContainer> contentArray;

	UE_LOG(LogTemp, Log, TEXT("image count%d"), imagesInDirectory.Num());
	UE_LOG(LogTemp, Log, TEXT("file count%d"), filesInDirectory.Num());

	for (int32 i = 0; i < filesInDirectory.Num(); i++)
	{
		FString Content;
		FString temp;
		FString result;
		if (FFileHelper::LoadFileToString(result, *filesInDirectory[i]))
		{
			FContentContainer container = FContentContainer();
			UE_LOG(LogTemp, Warning, TEXT("Loaded file %s"), *filesInDirectory[i]);
			//loaded the file

			Content = Content.Append(TEXT("\r\n"));
			Content = Content.Append("--cJkER9eqUVwt2tgnugnSBFkGLAgt7djINNHkQP0i");
			Content = Content.Append(TEXT("\r\n"));
			Content = Content.Append("Content-Type: application/octet-stream");
			Content = Content.Append(TEXT("\r\n"));
			Content = Content.Append("Content-disposition: form-data; name=\"file\"; filename=\"" + FPaths::GetCleanFilename(filesInDirectory[i]) + "\"");
			Content = Content.Append(TEXT("\r\n"));
			Content = Content.Append(TEXT("\r\n"));

			container.Headers = Content;
			container.BodyText = *result;

			contentArray.Add(container);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("failed to load %s"), *filesInDirectory[i]);
		}
	}



	for (int32 i = 0; i < imagesInDirectory.Num(); i++)
	{
		FString Content;
		TArray<uint8> byteResult;
		if (FFileHelper::LoadFileToArray(byteResult, *imagesInDirectory[i]))
		{
			FContentContainer container = FContentContainer();
			UE_LOG(LogTemp, Warning, TEXT("Loaded file %s"), *imagesInDirectory[i]);
			//loaded the file
			Content = Content.Append(TEXT("\r\n"));
			Content = Content.Append("--cJkER9eqUVwt2tgnugnSBFkGLAgt7djINNHkQP0i");
			Content = Content.Append(TEXT("\r\n"));
			Content = Content.Append("Content-Type: image/png");
			Content = Content.Append(TEXT("\r\n"));
			Content = Content.Append("Content-disposition: form-data; name=\"file\"; filename=\"" + FPaths::GetCleanFilename(imagesInDirectory[i]) + "\"");
			Content = Content.Append(TEXT("\r\n"));
			Content = Content.Append(TEXT("\r\n"));

			container.Headers = Content;
			container.BodyBinary = byteResult;

			contentArray.Add(container);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("failed to load image %s"), *imagesInDirectory[i]);
		}
	}
	TArray<uint8> AllBytes;
	TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();

	for (int32 i = 0; i < contentArray.Num(); i++)
	{
		//reference
		//RequestPayload.SetNumUninitialized(Converter.Length());
		//FMemory::Memcpy(RequestPayload.GetData(), (const uint8*)Converter.Get(), RequestPayload.Num());


		//headers
		FTCHARToUTF8 Converter(*contentArray[i].Headers);
		auto data = (const uint8*)Converter.Get();
		AllBytes.Append(data, Converter.Length());

		//content
		if (contentArray[i].BodyText.Len() > 0)
		{
			FTCHARToUTF8 ConverterBody(*contentArray[i].BodyText);
			auto bodydata = (const uint8*)ConverterBody.Get();
			//TArray<uint8> outbytes;
			//StringToBytes((ANSI_TO_TCHAR(contentArray[i].BodyText*)), outbytes);

			AllBytes.Append(bodydata, ConverterBody.Length());
		}
		else
		{
			AllBytes.Append(contentArray[i].BodyBinary);
		}
	}



	TArray<uint8> EndBytes;
	FString EndString;

	EndString = TEXT("\r\n");
	FTCHARToUTF8 ConverterEnd1(*EndString);
	auto enddata1 = (const uint8*)ConverterEnd1.Get();
	AllBytes.Append(enddata1, ConverterEnd1.Length());

	EndString = TEXT("--cJkER9eqUVwt2tgnugnSBFkGLAgt7djINNHkQP0i--");
	FTCHARToUTF8 ConverterEnd2(*EndString);
	auto enddata2 = (const uint8*)ConverterEnd2.Get();
	AllBytes.Append(enddata2, ConverterEnd2.Length());

	EndString = TEXT("\r\n");
	FTCHARToUTF8 ConverterEnd3(*EndString);
	auto enddata3 = (const uint8*)ConverterEnd3.Get();
	AllBytes.Append(enddata3, ConverterEnd3.Length());

	//HttpRequest->SetURL("http://192.168.1.145:3000/api/scenes");
	HttpRequest->SetURL("https://sceneexplorer.com/api/scenes");
	HttpRequest->SetHeader("Content-Type", "multipart/form-data; boundary=\"cJkER9eqUVwt2tgnugnSBFkGLAgt7djINNHkQP0i\"");
	HttpRequest->SetHeader("Accept-Encoding", "identity");
	HttpRequest->SetVerb("POST");
	HttpRequest->SetContent(AllBytes);

	HttpRequest->OnProcessRequestComplete().BindSP(this, &FBaseEditorToolCustomization::OnUploadSceneCompleted);

	//DEBUGGING write http request contents to file
	/*

	FString SaveDirectory = FString("C:/Users/calder/Desktop");
	FString FileName = FString("UploadContent.txt");

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	if (PlatformFile.CreateDirectoryTree(*SaveDirectory))
	{
		// Get absolute file path
		FString AbsoluteFilePath = SaveDirectory + "/" + FileName;

		//FFileHelper::SaveStringToFile(Content, *AbsoluteFilePath);
		FFileHelper::SaveArrayToFile(AllBytes, *AbsoluteFilePath);
	}
	*/

	HttpRequest->ProcessRequest();
	return FReply::Handled();
}

void FBaseEditorToolCustomization::OnUploadSceneCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		UE_LOG(LogTemp, Warning, TEXT("Upload Scene Response is %s"), *Response->GetContentAsString());

		UWorld* myworld = GWorld->GetWorld();
		if (myworld == NULL)
		{
			UE_LOG(LogTemp, Warning, TEXT("Upload Scene - No world!"));
			return;
		}

		FString currentSceneName = myworld->GetMapName();
		currentSceneName.RemoveFromStart(myworld->StreamingLevelsPrefix);

		//FConfigSection* ScenePairs = GConfig->GetSectionPrivate(TEXT("/Script/CognitiveVR.CognitiveVRSettings"), false, true, GEngineIni);
		//GConfig->SetString(TEXT("/Script/CognitiveVR.CognitiveVRSettings"), *currentSceneName, *Response->GetContentAsString(), GEngineIni);


		GLog->Log(currentSceneName + " scene set with SceneKey " + *Response->GetContentAsString());

		FString responseNoQuotes = *Response->GetContentAsString().Replace(TEXT("\""), TEXT(""));

		SaveSceneData(currentSceneName, responseNoQuotes);
	}
}

FReply FBaseEditorToolCustomization::DebugSendSceneData()
{
	//+ FString::FromInt(FMath::Rand())
	SaveSceneData("FirstPersonExampleMap", "1234-asdf-5678-hjkl");
	return FReply::Handled();
}

void FBaseEditorToolCustomization::SaveSceneData(FString sceneName, FString sceneKey)
{
	FString keyValue = sceneName + "," + sceneKey;

	GConfig->Flush(true, GGameIni);
	TArray<FString> scenePairs;

	GConfig->GetArray(TEXT("/Script/CognitiveVR.CognitiveVRSceneSettings"), TEXT("SceneData"), scenePairs, GGameIni);

	bool didSetKey = false;
	for (int32 i = 0; i < scenePairs.Num(); i++)
	{
		FString name;
		FString key;
		scenePairs[i].Split(TEXT(","), &name, &key);
		if (*name == sceneName)
		{
			scenePairs[i] = keyValue;
			didSetKey = true;
			GLog->Log("FBaseEditorToolCustomization::SaveSceneData - found and replace key for scene " + name + " new value " + keyValue);
			break;
		}
	}
	if (!didSetKey)
	{
		scenePairs.Add(keyValue);
		GLog->Log("FBaseEditorToolCustomization::SaveSceneData - added new scene value and key for " + sceneName);
	}

	//remove scene names that don't have keys!
	for (int32 i = scenePairs.Num()-1; i >= 0; i--)
	{
		FString name;
		FString key;
		if (!scenePairs[i].Split(TEXT(","), &name, &key))
		{
			scenePairs.RemoveAt(i);
		}
	}


	GConfig->SetArray(TEXT("/Script/CognitiveVR.CognitiveVRSceneSettings"), TEXT("SceneData"), scenePairs, GGameIni);

	GConfig->Flush(false, GGameIni);
}

//https://answers.unrealengine.com/questions/212791/how-to-get-file-list-in-a-directory.html
/**
Gets all the files in a given directory.
@param directory The full path of the directory we want to iterate over.
@param fullpath Whether the returned list should be the full file paths or just the filenames.
@param onlyFilesStartingWith Will only return filenames starting with this string. Also applies onlyFilesEndingWith if specified.
@param onlyFilesEndingWith Will only return filenames ending with this string (it looks at the extension as well!). Also applies onlyFilesStartingWith if specified.
@return A list of files (including the extension).
*/
TArray<FString> FBaseEditorToolCustomization::GetAllFilesInDirectory(const FString directory, const bool fullPath, const FString onlyFilesStartingWith, const FString onlyFilesWithExtension, const FString ignoreExtension)
{
	// Get all files in directory
	TArray<FString> directoriesToSkip;
	IPlatformFile &PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	FLocalTimestampDirectoryVisitor Visitor(PlatformFile, directoriesToSkip, directoriesToSkip, false);
	//PlatformFile.IterateDirectoryStat(*directory, Visitor);
	Visitor.Visit(*directory, true);
	TArray<FString> files;

	for (TMap<FString, FDateTime>::TIterator TimestampIt(Visitor.FileTimes); TimestampIt; ++TimestampIt)
	{
		const FString filePath = TimestampIt.Key();
		const FString fileName = FPaths::GetCleanFilename(filePath);
		bool shouldAddFile = true;

		// Check if filename starts with required characters
		if (!onlyFilesStartingWith.IsEmpty())
		{
			const FString left = fileName.Left(onlyFilesStartingWith.Len());

			if (!(fileName.Left(onlyFilesStartingWith.Len()).Equals(onlyFilesStartingWith)))
				shouldAddFile = false;
		}

		// Check if file extension is required characters
		if (!onlyFilesWithExtension.IsEmpty())
		{
			if (!(FPaths::GetExtension(fileName, false).Equals(onlyFilesWithExtension, ESearchCase::IgnoreCase)))
				shouldAddFile = false;
		}

		if (!ignoreExtension.IsEmpty())
		{
			if ((FPaths::GetExtension(fileName, false).Equals(ignoreExtension, ESearchCase::IgnoreCase)))
				shouldAddFile = false;
		}

		// Add full path to results
		if (shouldAddFile)
		{
			files.Add(fullPath ? filePath : fileName);
		}
	}

	return files;
}

bool FBaseEditorToolCustomization::HasFoundBlender() const
{
	return FBaseEditorToolCustomization::GetBlenderPath().ToString().Contains("blender.exe");
}

bool FBaseEditorToolCustomization::HasFoundBlenderAndExportDir() const
{
	return FBaseEditorToolCustomization::GetBlenderPath().ToString().Contains("blender.exe") && !FBaseEditorToolCustomization::GetExportDirectory().EqualTo(FText::FromString(""));
}

bool FBaseEditorToolCustomization::HasSetExportDirectory() const
{
	return !FBaseEditorToolCustomization::GetExportDirectory().EqualTo(FText::FromString(""));
}

FText FBaseEditorToolCustomization::GetBlenderPath() const
{
	return FText::FromString(BlenderPath);
}

FText FBaseEditorToolCustomization::GetExportDirectory() const
{
	return FText::FromString(ExportDirectory);
}

void FBaseEditorToolCustomization::SearchForBlender()
{
	//try to find blender in program files
	FString testApp = "C:/Program Files/Blender Foundation/Blender/blender.exe";

	if (VerifyFileExists(testApp))
	{
		UE_LOG(LogTemp, Warning, TEXT("SearchForBlender - Found Blender in Program Files"));
		BlenderPath = testApp;
	}
}

TSharedRef<IDetailCustomization> FBaseEditorToolCustomization::MakeInstance()
{
	return MakeShareable(new FBaseEditorToolCustomization);
}

FReply FBaseEditorToolCustomization::ExecuteToolCommand(IDetailLayoutBuilder* DetailBuilder, UFunction* MethodToExecute)
{
	TArray<TWeakObjectPtr<UObject>> ObjectsBeingCustomized;
	DetailBuilder->GetObjectsBeingCustomized(ObjectsBeingCustomized);

	for (auto WeakObject : ObjectsBeingCustomized)
	{
		if (UObject* Instance = WeakObject.Get())
		{
			Instance->CallFunctionByNameWithArguments(*MethodToExecute->GetName(), *GLog, nullptr, true);
		}
	}


	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE