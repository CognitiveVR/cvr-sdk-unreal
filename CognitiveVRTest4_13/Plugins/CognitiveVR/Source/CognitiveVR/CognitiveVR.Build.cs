// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
namespace UnrealBuildTool.Rules
{
	public class CognitiveVR : ModuleRules
	{
		public CognitiveVR(TargetInfo Target)
		{
            PublicIncludePathModuleNames.AddRange(
                new string[] {
                    "Core",
                    "CoreUObject",
                    "Engine",
					"Analytics"
					// ... add public include paths required here ...
				}
				);
			
			PrivateIncludePaths.AddRange(
				new string[] {
					"CognitiveVR/Private",
                    "CognitiveVR/Public",
					"CognitiveVR/Private/util",
                    "GearVR/Private"
					// ... add other private include paths required here ...
				}
				);

            PrivateDependencyModuleNames.AddRange(
                new string[]
                {
                    "HeadMountedDisplay",

                    "Core",
                    "CoreUObject",
                    "DesktopPlatform",
                    "AssetTools",
                    "Engine",
                    "Slate",
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

			PublicDependencyModuleNames.AddRange(
				new string[]
				{
					"Core",
					"CoreUObject",
					"Engine",
					"AnalyticsBlueprintLibrary",
					"Analytics",
					"AnalyticsVisualEditing",
					"Projects",
					"HTTP",
					"Json",
                    "JsonUtilities"
                }
				);
		}
	}
}