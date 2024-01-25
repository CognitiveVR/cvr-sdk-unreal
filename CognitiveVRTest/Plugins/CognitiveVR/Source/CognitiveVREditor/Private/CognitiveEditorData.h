#pragma once


	class FEditorSceneData
	{
	public:
		FString Name = "";
		FString Id = "";
		int32 VersionNumber = 1;
		int32 VersionId = 0;

		FEditorSceneData(FString name, FString id, int32 versionnumber, int32 versionid)
		{
			Name = name;
			Id = id;
			VersionNumber = versionnumber;
			VersionId = versionid;
		}
	};

	enum class EDynamicTypes : uint8
	{
		CustomId UMETA(DisplayName = "CustomId"),
		GeneratedId UMETA(DisplayName = "GeneratedId"),
		DynamicIdPool UMETA(DisplayName = "DynamicIdPool"),
		DynamicIdPoolAsset UMETA(DisplayName = "DynamicIdPoolAsset")
	};

	class FDynamicData
	{
	public:
		FString Name = "";
		FString MeshName = "";
		FString Id = "";

		//array for ids in id pool
		TArray<FString> DynamicPoolIds;

		//enum of dynamic type
		EDynamicTypes DynamicType = EDynamicTypes::CustomId;

		FDynamicData(FString name, FString meshname, FString id, EDynamicTypes type = EDynamicTypes::CustomId)
		{
			Name = name;
			MeshName = meshname;
			Id = id;
			DynamicType = type;
		}

		FDynamicData(FString name, FString meshname, FString id, TArray<FString> idArray, EDynamicTypes type)
		{
			Name = name;
			MeshName = meshname;
			Id = id;
			DynamicPoolIds = idArray;
			DynamicType = type;
		}
		
	};
