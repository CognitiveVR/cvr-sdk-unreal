// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
namespace UnrealBuildTool.Rules
{
	public class CognitiveVR : ModuleRules
	{
		public CognitiveVR(ReadOnlyTargetRules Target): base(Target)
		{
			PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
			bLegacyPublicIncludePaths = true;

			//set modules and dependencies
			SetUnrealModules();
			SetUnrealIncludes();
			SetDirectSoundIncludes();

			/*
			 * 	The optional parameter 'includeModule' can be set:
			 *	true - it will include the required plugin module dependencies. useful if you have engine-level plugins adding features instead of project-level plugins
			 *	false - it will exclude the plugin module, even if it is included in the project plugins folder
			 *	null(default) - it will search the plugins folder for supported plugins, and include them if they are present
			
			 *	For example:
			 *	CheckVarjoPlugin();
			 *	CheckVarjoPlugin(includeModule: true);
			 *	CheckVarjoPlugin(includeModule: false);
			*/

			CheckVarjoPlugin();
			CheckTobiiEyeTrackingPlugin();
			CheckViveProEyeSRanipal_1_2Plugin();
			CheckViveProEyeSRanipal_1_3Plugin();
			CheckPicoNeoEyePlugin();
			CheckHPOmniceptPlugin();
			CheckViveVavePlugin();

			//uncomment this to enable eye tracking support using IEyeTracker interface (varjo openxr support, etc)
			//PublicDefinitions.Add("OPENXR_EYETRACKING");
			//PublicDefinitions.Add("OPENXR_LOCALSPACE"); //uncomment this if OPENXR implemented in local space instead of worldspace
		}

		void CheckVarjoPlugin(bool? includeModule = null)
		{
			if (includeModule.HasValue && includeModule.Value == false)
			{
				return;
			}

			if ((includeModule.HasValue && includeModule.Value == true)
				|| PluginDirectoryExists("Varjo"))
			{
				System.Console.WriteLine("CognitiveVR.Build.cs adding Varjo dependencies");
				PublicDependencyModuleNames.Add("VarjoHMD");
				PublicDependencyModuleNames.Add("VarjoEyeTracker");
			}
		}

		void CheckTobiiEyeTrackingPlugin(bool? includeModule = null)
		{
			if (includeModule.HasValue && includeModule.Value == false)
			{
				return;
			}

			if ((includeModule.HasValue && includeModule.Value == true)
				|| PluginDirectoryExists("TobiiEyeTracking"))
			{
				System.Console.WriteLine("CognitiveVR.Build.cs adding TobiiEyeTracking dependencies");
				PrivateIncludePaths.AddRange(
					new string[] {
					"../../TobiiEyeTracking/Source/TobiiCore/Private",
					"../../TobiiEyeTracking/Source/TobiiCore/Public"
					});

				PublicDependencyModuleNames.Add("TobiiCore");				
			}
		}

		void CheckViveProEyeSRanipal_1_2Plugin(bool? includeModule = null)
		{
			if (includeModule.HasValue && includeModule.Value == false)
			{
				return;
			}

			if ((includeModule.HasValue && includeModule.Value == true)
				|| PluginDirectoryExists("SRanipal"))
			{
				string pluginsDirectory = GetPluginsDirectory();
				string sranipalPlugin = System.IO.Path.Combine(pluginsDirectory, "SRanipal");
				string sranipalSource = System.IO.Path.Combine(sranipalPlugin, "Source");
				string[] sourceModules = System.IO.Directory.GetDirectories(sranipalSource);
				if (sourceModules.Length == 2) //1.1.0.1 and 1.2.0.1 only have eye and lip modules
				{
					PublicDefinitions.Add("SRANIPAL_1_2_API");
					System.Console.WriteLine("CognitiveVR.Build.cs adding SRanipal Plugin 1.2 dependencies");
					PrivateIncludePaths.AddRange(
						new string[] {
						"../../SRanipal/Source/SRanipal/Private",
						"../../SRanipal/Source/SRanipal/Public"
						});

					PublicDependencyModuleNames.Add("SRanipal");
					string BaseDirectory = System.IO.Path.GetFullPath(System.IO.Path.Combine(ModuleDirectory, "..", "..", ".."));
					string SRanipalDir = System.IO.Path.Combine(BaseDirectory, "SRanipal", "Binaries", Target.Platform.ToString());
					PublicAdditionalLibraries.Add(System.IO.Path.Combine(SRanipalDir, "SRanipal.lib"));
					PublicDelayLoadDLLs.Add(System.IO.Path.Combine(SRanipalDir, "SRanipal.dll"));
				}
			}
		}

		void CheckViveProEyeSRanipal_1_3Plugin(bool? includeModule = null)
		{
			if (includeModule.HasValue && includeModule.Value == false)
			{
				return;
			}

			if ((includeModule.HasValue && includeModule.Value == true)
				|| PluginDirectoryExists("SRanipal"))
			{
				string pluginsDirectory = GetPluginsDirectory();
				string sranipalPlugin = System.IO.Path.Combine(pluginsDirectory, "SRanipal");
				string sranipalSource = System.IO.Path.Combine(sranipalPlugin, "Source");
				string[] sourceModules = System.IO.Directory.GetDirectories(sranipalSource);
				if (sourceModules.Length == 5)
				{
					PublicDefinitions.Add("SRANIPAL_1_3_API");
					System.Console.WriteLine("CognitiveVR.Build.cs adding SRanipal Plugin 1.3.0.9 (or newer) dependencies");
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
			}
		}

		void CheckPicoNeoEyePlugin(bool? includeModule = null)
		{
			if (includeModule.HasValue && includeModule.Value == false)
			{
				return;
			}

			if ((includeModule.HasValue && includeModule.Value == true)
				|| PluginDirectoryExists("PicoMobile"))
			{
				System.Console.WriteLine("CognitiveVR.Build.cs adding PicoMobile dependencies");
				PublicDependencyModuleNames.Add("PicoMobile");
			}
		}
		
		void CheckHPOmniceptPlugin(bool? includeModule = null)
		{
			if (includeModule.HasValue && includeModule.Value == false)
			{
				return;
			}

			if ((includeModule.HasValue && includeModule.Value == true)
				|| PluginDirectoryExists("HPGlia"))
			{
				System.Console.WriteLine("CognitiveVR.Build.cs adding HPGlia dependencies");
				PublicDependencyModuleNames.Add("HPGlia");
			}
		}

		void CheckViveVavePlugin(bool? includeModule = null)
		{
			if (includeModule.HasValue && includeModule.Value == false)
			{
				return;
			}
			if ((includeModule.HasValue && includeModule.Value == true)
				|| PluginDirectoryExists("WaveVR"))
			{
				System.Console.WriteLine("CognitiveVR.Build.cs adding WaveVR dependencies");
				PublicDefinitions.Add("WAVEVR_EYETRACKING");
				PublicDependencyModuleNames.Add("WaveVR");
			}
		}

		string GetPluginsDirectory()
		{
			return System.IO.Path.Combine(Target.ProjectFile.Directory.ToString(), "Plugins");	
		}

		bool PluginDirectoryExists(string pluginName)
        {
			string pluginsDirectory = GetPluginsDirectory();
			bool result = System.IO.Directory.Exists(System.IO.Path.Combine(pluginsDirectory, pluginName));
			if (result == true)
            {
				System.Console.WriteLine("CognitiveVR.Build.cs found " + pluginName + " plugin folder");
			}
			return result;
		}

		void SetUnrealIncludes()
		{
			PublicIncludePathModuleNames.AddRange(
			new string[] {
				"Core",
				"CoreUObject",
				"Engine",
				"Analytics"
			});

			PrivateIncludePaths.AddRange(
			new string[] {
				"CognitiveVR/Private",
				"CognitiveVR/Public",
				"CognitiveVR/Private/util"
			});
		}

		void SetUnrealModules()
		{
			PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"HeadMountedDisplay",
				"Slate",
				"SlateCore"
			});

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
				"EyeTracker",
				"DeveloperSettings"
			});
		}

		void SetDirectSoundIncludes()
		{
			//this is all for runtime audio capture support
			if (Target.Platform == UnrealTargetPlatform.Win64
				|| Target.Platform == UnrealTargetPlatform.Win32
				)
			{
				// Add __WINDOWS_WASAPI__ so that RtAudio compiles with WASAPI
				PublicDefinitions.Add("__WINDOWS_DS__");

				// Allow us to use direct sound
				AddEngineThirdPartyPrivateStaticDependencies(Target, "DirectSound");

				string DirectXSDKDir = Target.UEThirdPartySourceDirectory + "Windows/DirectX";
				PublicSystemIncludePaths.Add(DirectXSDKDir + "/include");

				PublicAdditionalLibraries.AddRange(
				new string[]
				{
					"dxguid.lib",
					"dsound.lib"
				});
			}
		}
	}
}
