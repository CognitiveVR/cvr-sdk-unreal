// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
namespace UnrealBuildTool.Rules
{
	public class CognitiveVR : ModuleRules
	{
		public CognitiveVR(TargetInfo Target)
		{
            //PCHUsage = PCHUsageMode.NoSharedPCHs;

            PublicIncludePathModuleNames.AddRange(
                new string[] {
                    "Core",
                    "CoreUObject",
                    "Engine",
					"Analytics"
					// ... add public include paths required here ...
				}
				);
			PublicIncludePaths.AddRange(
				new string[] {
                    "Analytics",
                    "CognitiveVR/Private"
					//"CognitiveVRAnalytics/Public",
					// ... add other private include paths required here ...
				}
				);
			
			PrivateIncludePaths.AddRange(
				new string[] {
					"Runtime/CognitiveVRAnalytics/Private",
					"Engine",
					"HTTP",
					"Private",
					"Analytics",
					"Settings",
					"Private/unreal",
					"CognitiveVR/Private",
					"CognitiveVR/Private/api",
					"CognitiveVR/Private/network",
					"CognitiveVR/Private/unreal",
					"CognitiveVR/Private/util"
					// ... add other private include paths required here ...
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


			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					"HTTP",                    
					// ... add private dependencies that you statically link with here ...	
				}
			);
		}
	}
}