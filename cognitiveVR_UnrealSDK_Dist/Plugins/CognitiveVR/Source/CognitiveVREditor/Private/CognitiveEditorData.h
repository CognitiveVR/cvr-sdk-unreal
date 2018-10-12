#pragma once

namespace cognitivevrapi
{
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
		FString Id = "";

		FDynamicData(FString name, FString meshname, FString id)
		{
			Name = name;
			MeshName = meshname;
			Id = id;
		}
	};
}