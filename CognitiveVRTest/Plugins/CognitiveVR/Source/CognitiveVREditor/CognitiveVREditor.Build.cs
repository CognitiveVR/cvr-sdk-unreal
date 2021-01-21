using UnrealBuildTool;

public class CognitiveVREditor : ModuleRules
{
	public CognitiveVREditor(ReadOnlyTargetRules Target):base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PrivateIncludePaths.AddRange(
            new string[] {
                "CognitiveVR/Private",
                "CognitiveVR/Private/api",
                "CognitiveVR/Private/network",
                "CognitiveVR/Private/util"
            });


        PublicDependencyModuleNames.AddRange(
            new string[]
            {
				"CognitiveVR",
				"HTTP",
				"Json",
				"JsonUtilities",
				"MaterialBaking"
            });

        PrivateDependencyModuleNames.AddRange(
			new string[] {
                "Core",
                "CoreUObject",
                "DesktopPlatform",
                "AssetTools",
                "Engine",
                "Slate",
				"InputCore",
				"SlateCore",
				"PropertyEditor",
                "EditorStyle",
				"LevelEditor",
				"WorkspaceMenuStructure",
                "MeshUtilities",
				"MaterialUtilities",
                "ImageWrapper",
                "MainFrame",
                "UnrealEd",
                "PluginBrowser",
                "Projects",
                "CognitiveVR",
				"RenderCore"
            }
		);
		PrivateIncludePathModuleNames.AddRange(
			new string[] {
				"AssetTools",
				"UnrealEd"
			});
	}
}
