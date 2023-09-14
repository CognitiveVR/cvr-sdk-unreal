using UnrealBuildTool;

public class CognitiveVREditor : ModuleRules
{
	public CognitiveVREditor(ReadOnlyTargetRules Target):base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		bLegacyPublicIncludePaths = true;

        PrivateIncludePaths.AddRange(
            new string[] {
                "CognitiveVR/Private",
                "CognitiveVR/Private/api",
                "CognitiveVR/Private/network",
                "CognitiveVR/Private/util",
                System.IO.Path.GetFullPath(Target.RelativeEnginePath) + "/Source/Editor/Blutility/Private",
                System.IO.Path.GetFullPath(Target.RelativeEnginePath) + "/Source/Developer/MeshUtilities/Private"
            });


        PublicDependencyModuleNames.AddRange(
            new string[]
            {
				"CognitiveVR",
				"HTTP",
				"Json",
				"JsonUtilities",
				"MaterialBaking",
                "Blutility",
                "UnrealEd",
                "AssetTools",
                "SceneOutliner",
                "EditorScriptingUtilities",
                "MeshUtilities"
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

        var pluginsDirectory = System.IO.Path.Combine(Target.ProjectFile.Directory.ToString(), "Plugins");
        //HP Omnicept
        if (System.IO.Directory.Exists(System.IO.Path.Combine(pluginsDirectory, "HPGlia")))
        {
            System.Console.WriteLine("CognitiveVR.Build.cs found HP Glia Omnicept folder");
            PublicDependencyModuleNames.Add("HPGlia");
        }
    }
}
