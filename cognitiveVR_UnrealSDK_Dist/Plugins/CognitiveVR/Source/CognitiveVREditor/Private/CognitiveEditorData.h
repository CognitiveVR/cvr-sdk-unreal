#pragma once

class FOrganizationData
{
public:
	FString id = "";
	FString name = "";
	FString prefix = "";
};

class FProductData
{
public:
	FString id = "";
	FString name = "";
	FString orgId = "";
	FString customerId = "";
};

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

class FDynamicData
{
public:
	FString Name = "";
	FString MeshName = "";
	int32 Id = -1;

	FDynamicData(FString name, FString meshname, int32 id)
	{
		Name = name;
		MeshName = meshname;
		Id = id;
	}
};