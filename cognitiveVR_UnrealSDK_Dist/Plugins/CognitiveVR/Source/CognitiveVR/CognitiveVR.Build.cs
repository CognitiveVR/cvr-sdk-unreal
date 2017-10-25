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
					"CognitiveVR/Private/util"
					// ... add other private include paths required here ...
				}
				);

            PrivateDependencyModuleNames.AddRange(
                new string[]
                {
                    "HeadMountedDisplay"
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
				
		if (Target.Platform == UnrealTargetPlatform.Win32 ||
            Target.Platform == UnrealTargetPlatform.Win64)
        {
            // Add __WINDOWS_WASAPI__ so that RtAudio compiles with WASAPI
            Definitions.Add("__WINDOWS_DS__");

            // Allow us to use direct sound
            AddEngineThirdPartyPrivateStaticDependencies(Target, "DirectSound");
        }
		
		string DirectXSDKDir = UEBuildConfiguration.UEThirdPartySourceDirectory + "Windows/DirectX";
		PublicSystemIncludePaths.Add( DirectXSDKDir + "/include");

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			PublicLibraryPaths.Add( DirectXSDKDir + "/Lib/x64");
		}
		else if (Target.Platform == UnrealTargetPlatform.Win32)
		{
			PublicLibraryPaths.Add( DirectXSDKDir + "/Lib/x86");
		}

		PublicAdditionalLibraries.AddRange(
				new string[] {
 				"dxguid.lib",
 				"dsound.lib"
 				}
			);
		}
	}
}