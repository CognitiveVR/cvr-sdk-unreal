/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/

using UnrealBuildTool;

public class Cognitive3DEditor : ModuleRules
{
	public Cognitive3DEditor(ReadOnlyTargetRules Target):base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		bLegacyPublicIncludePaths = true;

        PrivateIncludePaths.AddRange(
            new string[] {
                "Cognitive3D/Private",
                "Cognitive3D/Private/C3DApi",
                "Cognitive3D/Private/C3DNetwork",
                "Cognitive3D/Private/C3DUtil",
                System.IO.Path.GetFullPath(Target.RelativeEnginePath) + "/Source/Editor/Blutility/Private",
                System.IO.Path.GetFullPath(Target.RelativeEnginePath) + "/Source/Developer/MeshUtilities/Private"
            });


        PublicDependencyModuleNames.AddRange(
            new string[]
            {
				"Cognitive3D",
				"HTTP",
				"Json",
				"JsonUtilities",
				"MaterialBaking",
                "Blutility",
                "UnrealEd",
                "AssetTools",
                "SceneOutliner",
                "EditorScriptingUtilities",
                "MeshUtilities",
                "GLTFExporter",
                "AssetRegistry",
                "Landscape"
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
                "Cognitive3D",
				"RenderCore",
                "GLTFExporter"
            }
		);
		PrivateIncludePathModuleNames.AddRange(
			new string[] {
				"AssetTools",
				"UnrealEd"
			});

        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.Add("Kismet");
        }

        var pluginsDirectory = System.IO.Path.Combine(Target.ProjectFile.Directory.ToString(), "Plugins");
        //HP Omnicept
        if (System.IO.Directory.Exists(System.IO.Path.Combine(pluginsDirectory, "HPGlia")))
        {
            System.Console.WriteLine("Cognitive3D.Build.cs found HP Glia Omnicept folder");
            PublicDependencyModuleNames.Add("HPGlia");
        }
    }
}
