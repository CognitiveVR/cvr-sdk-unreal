using UnrealBuildTool;

public class CognitiveVREditor : ModuleRules
{
	public CognitiveVREditor(TargetInfo Target)
	{
		PublicDependencyModuleNames.AddRange(
			new string[] {
				"Core",
				"CoreUObject",
				"Engine",
				"Slate",
				"UnrealEd",
				//"DemoEditorExtensionsRuntime",
                "MeshUtilities",
                "Landscape",
                "AssetTools",
                "DesktopPlatform",
                "Projects",
                "AssetRegistry",
                "Analytics"
            }
		);
		
		PrivateDependencyModuleNames.AddRange(
			new string[] {
				"InputCore",
				"SlateCore",
				"PropertyEditor",
				"LevelEditor",
                "MeshUtilities",
                "MainFrame",
                "UnrealEd",
                "PluginBrowser",
                "Projects"
            }
		);
	}
}