/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/


namespace UnrealBuildTool.Rules
{
	public class Cognitive3D : ModuleRules
	{
		private string pluginsDirectory;

		public Cognitive3D(ReadOnlyTargetRules Target): base(Target)
		{
			PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
			bLegacyPublicIncludePaths = true;

			PublicDependencyModuleNames.Add("DeveloperSettings");
			pluginsDirectory = System.IO.Path.Combine(Target.ProjectFile.Directory.ToString(), "Plugins");

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
					"Cognitive3D/Private",
                    "Cognitive3D/Public",
					"Cognitive3D/Private/C3DUtil"
					// ... add other private include paths required here ...
				}
				);

            PrivateDependencyModuleNames.AddRange(
                new string[]
                {
					"Slate",
					"SlateCore"
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
                    "JsonUtilities",
					"UMG",
					"EngineSettings",
                    "HeadMountedDisplay",
                    "EyeTracker",
					"EnhancedInput",
					"InputCore",
					"MediaAssets",
                    "Landscape"
                }
				);

            if (Target.bBuildEditor)
            {
                PrivateDependencyModuleNames.Add("UnrealEd");
            }

            if (Target.Platform == UnrealTargetPlatform.Android)
            {
                PublicDependencyModuleNames.AddRange(new string[]{"AndroidPermission", // Common for Android builds
					"ApplicationCore", // Required for certain Android API calls
					"Launch" // Required for Android builds
				}
                );
                PublicRuntimeLibraryPaths.Add(System.IO.Path.Combine(ModuleDirectory, "Android/lib"));
                AdditionalPropertiesForReceipt.Add("AndroidPlugin", System.IO.Path.Combine(ModuleDirectory, "Android/Cognitive3D_UPL.xml"));
            }

            //uncomment the following line to enable Oculus/Meta functionality. Uses OculusVR for UE4 and OculusXR (MetaXR) for UE5.
            //Specifically adds eye tracking support for Quest Pro and uses Meta APIs for room size and boundary events
            //MetaXRPlugin();

            //Uncomment the following line to enable Oculus Passthrough features. UE 5.2 onward
            //MUST ALSO ENABLE OCULUS PLUGIN ABOVE!
            //MetaXRPassthrough();

            //Uncomment the following line to enable Oculus Platform features
            //Uses Meta Platform Plugin to get Oculus Username. Also gets the user's subscription status
            //MetaXRPlatform();

            //Uncomment the following line to enable PICOXR SDK features
            //PICOXR();

            //Uncomment the following line to enable eye tracking support using IEyeTracker interface (varjo openxr support, etc)
            //OpenXREyeTracking();

            //Uncomment the following line to enable Vive WaveVR eye tracking support
            //WaveVREyeTracking();

            //Varjo (up to and including version 3.0.0)
            //Varjo();

            //Uncomment to enable Tobii Eye Tracking
            //TobiiEyeTracking();

            //Uncomment to enable Vive SRanipal version 1.3+ for eye tracking
            //Legacy support, prefer to use Vive OpenXR
            //SRanipalVivePro();

            //Pico Neo 2 Eye tracking
            //Legacy support, prefer to use PicoXR SDK
            //PicoMobile();

            //HP Omnicept eye tracking and sensors
            //Legacy support
            //HPGlia();

            //this is all for runtime audio capture support using ExitPoll Surveys
            if (Target.Platform == UnrealTargetPlatform.Win64
				|| Target.Platform == UnrealTargetPlatform.Win32
            )
			{
				// Add __WINDOWS_WASAPI__ so that RtAudio compiles with WASAPI
				PublicDefinitions.Add("__WINDOWS_DS__");

				// Allow us to use direct sound
				AddEngineThirdPartyPrivateStaticDependencies(Target, "DirectSound");
			
				string DirectXSDKDir = Target.UEThirdPartySourceDirectory + "Windows/DirectX";
				PublicSystemIncludePaths.Add( DirectXSDKDir + "/include");

				PublicAdditionalLibraries.AddRange(
						new string[] {
						"dxguid.lib",
						"dsound.lib"
						}
					);
			}
		}

		void MetaXRPlugin()
        {
			PublicDefinitions.Add("INCLUDE_OCULUS_PLUGIN");
			PublicDependencyModuleNames.AddRange(new string[] { "OculusHMD", "OculusInput" });
		}

		void MetaXRPassthrough()
		{
			PublicDefinitions.Add("INCLUDE_OCULUS_PASSTHROUGH");
			PublicDependencyModuleNames.AddRange(new string[] { "OculusXRPassthrough" });
		}

		void MetaXRPlatform()
        {
			PublicDefinitions.Add("INCLUDE_OCULUS_PLATFORM");
			PublicDependencyModuleNames.AddRange(new string[] { "OVRPlatform", "OVRPlatformSDK" });
			PublicIncludePathModuleNames.AddRange(new string[] { "OVRPlatformSDK" });
			PublicIncludePaths.Add(System.IO.Path.GetFullPath(Target.RelativeEnginePath) + "/Plugins/Marketplace/MetaXRPlatform/Source/OVRPlatformSDK/LibOVRPlatform/include/Windows");
			PrivateIncludePaths.Add(System.IO.Path.GetFullPath(Target.RelativeEnginePath) + "/Plugins/Marketplace/MetaXRPlatform/Source/OVRPlatform/Private");
		}

		void PICOXR()
        {
			PublicDefinitions.Add("INCLUDE_PICO_PLUGIN");
            PublicDependencyModuleNames.AddRange(new string[] { "PICOXRHMD", "PICOXRInput", "InputDevice" });
            PrivateIncludePaths.Add(System.IO.Path.Combine(pluginsDirectory, "PICOXR/Source/PICOXRInput/Private"));
        }

		void OpenXREyeTracking()
        {
			PublicDefinitions.Add("OPENXR_EYETRACKING");
		}

		void WaveVREyeTracking()
        {
			PublicDefinitions.Add("WAVEVR_EYETRACKING");
			PublicDependencyModuleNames.Add("WaveVR");
		}

		void Varjo()
        {
            PublicDefinitions.Add("INCLUDE_VARJO_PLUGIN");
            PublicDependencyModuleNames.Add("VarjoHMD");
			PublicDependencyModuleNames.Add("VarjoEyeTracker");
		}

		void TobiiEyeTracking()
        {
            PublicDefinitions.Add("INCLUDE_TOBII_PLUGIN");
            PrivateIncludePaths.AddRange(
				new string[] {
				"../../TobiiEyeTracking/Source/TobiiCore/Private",
				"../../TobiiEyeTracking/Source/TobiiCore/Public"
				});

			PublicDependencyModuleNames.Add("TobiiCore");
		}

		void SRanipalVivePro()
        {
			PublicDefinitions.Add("SRANIPAL_1_3_API");
			PrivateIncludePaths.AddRange(
				new string[] {
				"../../SRanipal/Source/SRanipal/Private",
				"../../SRanipal/Source/SRanipal/Public",
				"../../SRanipal/Source/SRanipalEye/Private",
				"../../SRanipal/Source/SRanipalEye/Public",
				"../../SRanipal/Source/SRanipalEyeTracker/Private",
				"../../SRanipal/Source/SRanipalEyeTracker/Public"
				});

			PublicDependencyModuleNames.Add("SRanipal");
			PublicDependencyModuleNames.Add("SRanipalEye");
			PublicDependencyModuleNames.Add("SRanipalEyeTracker");

			string BaseDirectory = System.IO.Path.GetFullPath(System.IO.Path.Combine(ModuleDirectory, "..", "..", ".."));
			string SRanipalDir = System.IO.Path.Combine(BaseDirectory, "SRanipal", "Binaries", Target.Platform.ToString());
			PublicAdditionalLibraries.Add(System.IO.Path.Combine(SRanipalDir, "SRanipal.lib"));
			PublicDelayLoadDLLs.Add(System.IO.Path.Combine(SRanipalDir, "SRanipal.dll"));
		}

		void PicoMobile()
        {
            PublicDefinitions.Add("INCLUDE_PICOMOBILE_PLUGIN");
            PublicDependencyModuleNames.Add("PicoMobile");
		}

		void HPGlia()
        {
            PublicDefinitions.Add("INCLUDE_HPGLIA_PLUGIN");
            PublicDependencyModuleNames.Add("HPGlia");
		}
	}
}
