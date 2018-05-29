using UnrealBuildTool;

public class CognitiveVREditor : ModuleRules
{
	public CognitiveVREditor(TargetInfo Target)
	{
        PublicDependencyModuleNames.AddRange(
            new string[]
            {
				"CognitiveVR",
				"HTTP",
				"Json",
				"JsonUtilities"
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
                "MeshUtilities",
				"MaterialUtilities",
                "ImageWrapper",
                "MainFrame",
                "UnrealEd",
                "PluginBrowser",
                "Projects",
                "CognitiveVR"
            }
		);
	}
}