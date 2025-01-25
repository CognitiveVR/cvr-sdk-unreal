/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/

#include "C3DUtil/Util.h"
#include "HeadMountedDisplayFunctionLibrary.h"

double FUtil::GetTimestamp()
{
	//#pragma warning(push)
	//#pragma warning(disable:4244) //Disable warning regarding loss of accuracy, no concern.

	long ts = time(0);
	double miliseconds = FDateTime::UtcNow().GetMillisecond();
	double finalTime = ts + miliseconds*0.001;

	return finalTime;
	//http://stackoverflow.com/questions/997946/how-to-get-current-time-and-date-in-c

	//#pragma warning(pop)
}

FString FUtil::GetDeviceName(FString DeviceName)
{
	if (DeviceName == "OculusRift")
	{
		return "rift";
	}
	if (DeviceName == "OculusXRHMD")
	{
		return "rift";
	}
	if (DeviceName == "OculusHMD")
	{
		return "rift";
	}
	if (DeviceName == "OSVR")
	{
		return "rift";
	}
	if (DeviceName == "SimpleHMD")
	{
		return "rift";
	}
	if (DeviceName == "SteamVR")
	{
		return "vive";
	}
	return FString("unknown");
}

void FUtil::SetSessionProperties()
{
	auto cog = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();

	FString HMDDeviceName = UHeadMountedDisplayFunctionLibrary::GetHMDDeviceName().ToString();
	cog->SetSessionProperty("c3d.device.model", HMDDeviceName);

#if defined INCLUDE_TOBII_PLUGIN
	cog->SetSessionProperty("c3d.device.eyetracking.enabled", true);
	cog->SetSessionProperty("c3d.device.eyetracking.type", FString("Tobii"));
	cog->SetSessionProperty("c3d.app.sdktype", FString("Tobii"));
#elif defined SRANIPAL_1_2_API
	cog->SetSessionProperty("c3d.device.eyetracking.enabled", true);
	cog->SetSessionProperty("c3d.device.eyetracking.type", FString("Tobii"));
	cog->SetSessionProperty("c3d.app.sdktype", FString("SRAnipal"));
#elif defined SRANIPAL_1_3_API
	cog->SetSessionProperty("c3d.device.eyetracking.enabled", true);
	cog->SetSessionProperty("c3d.device.eyetracking.type", FString("Tobii"));
	cog->SetSessionProperty("c3d.app.sdktype", FString("SRAnipal"));
#elif defined INCLUDE_VARJO_PLUGIN
	cog->SetSessionProperty("c3d.device.eyetracking.enabled", true);
	cog->SetSessionProperty("c3d.device.eyetracking.type", FString("Varjo"));
	cog->SetSessionProperty("c3d.app.sdktype", FString("Varjo"));
#elif defined INCLUDE_PICOMOBILE_PLUGIN
	//TODO check that pico eye tracking is enabled
	cog->SetSessionProperty("c3d.device.eyetracking.enabled", true);
	cog->SetSessionProperty("c3d.device.eyetracking.type", FString("Tobii"));
	cog->SetSessionProperty("c3d.app.sdktype", FString("Pico"));
	cog->SetSessionProperty("c3d.device.model", FPlatformMisc::GetCPUBrand()); //returns pretty device name
#elif defined INCLUDE_HPGLIA_PLUGIN
	//TODO check that omnicept eye tracking is enabled
	cog->SetSessionProperty("c3d.device.eyetracking.enabled", true);
	cog->SetSessionProperty("c3d.device.eyetracking.type", FString("Tobii"));
	cog->SetSessionProperty("c3d.app.sdktype", FString("HP Omnicept"));
#elif defined INCLUDE_OCULUS_PLUGIN
	cog->SetSessionProperty("c3d.app.sdktype", FString("Oculus HMD"));
	cog->SetSessionProperty("c3d.device.model", FPlatformMisc::GetCPUBrand());
#elif defined INCLUDE_PICO_PLUGIN
	cog->SetSessionProperty("c3d.device.eyetracking.enabled", true);
	cog->SetSessionProperty("c3d.app.sdktype", FString("PICO"));
	cog->SetSessionProperty("c3d.device.model", FPlatformMisc::GetCPUBrand());
#else
	cog->SetSessionProperty("c3d.device.eyetracking.enabled", false);
	cog->SetSessionProperty("c3d.device.eyetracking.type", FString("None"));
	cog->SetSessionProperty("c3d.app.sdktype", FString("Default"));
#endif

	FString sdkVersion = Cognitive3D_SDK_VERSION;
	cog->SetSessionProperty("c3d.version", sdkVersion);
	cog->SetSessionProperty("c3d.app.engine", FString("Unreal"));

	if (!cog->GetUserID().IsEmpty())
		cog->SetParticipantId(cog->GetUserID());
	cog->SetSessionProperty("c3d.deviceid", cog->GetDeviceID());

	const UGeneralProjectSettings& projectSettings = *GetDefault< UGeneralProjectSettings>();
	cog->SetSessionProperty("c3d.app.version", projectSettings.ProjectVersion);
	FString projName = FApp::GetProjectName();
	cog->SetSessionProperty("c3d.app.name", projName);

	FString engineVersion = FEngineVersion::Current().ToString().Replace(TEXT("+"), TEXT(" "));;
	cog->SetSessionProperty("c3d.app.engine.version", engineVersion);

	auto platformName = UGameplayStatics::GetPlatformName();
	if (platformName.Compare("Windows", ESearchCase::IgnoreCase) == 0 || platformName.Compare("Mac", ESearchCase::IgnoreCase) == 0 || platformName.Compare("Linux", ESearchCase::IgnoreCase) == 0)
	{
		cog->SetSessionProperty("c3d.device.type", FString("Desktop"));
	}
	else if (platformName.Compare("IOS", ESearchCase::IgnoreCase) == 0 || platformName.Compare("Android", ESearchCase::IgnoreCase) == 0)
	{
		cog->SetSessionProperty("c3d.device.type", FString("Mobile"));
	}
	else if (platformName.Compare("PS", ESearchCase::IgnoreCase) == 0 || platformName.Contains("xbox", ESearchCase::IgnoreCase) || platformName.Contains("Switch", ESearchCase::IgnoreCase))
	{
		cog->SetSessionProperty("c3d.device.type", FString("Console"));
	}
	else
	{
		cog->SetSessionProperty("c3d.device.type", FString("Unknown"));
	}
	FString GPUBrand = "Unknown";
	FString CPUChipset = "Unknown";

#if PLATFORM_ANDROID
	GPUBrand = FAndroidMisc::GetPrimaryGPUBrand();
	CPUChipset = FAndroidMisc::GetCPUChipset();
	cog->SetSessionProperty("c3d.device.cpu", CPUChipset);
	cog->SetSessionProperty("c3d.device.gpu", GPUBrand);
	cog->SetSessionProperty("c3d.device.os", "Android OS " + FAndroidMisc::GetOSVersion());


#elif PLATFORM_WINDOWS
	GPUBrand = FPlatformMisc::GetPrimaryGPUBrand();
	CPUChipset = FPlatformMisc::GetCPUBrand();
	cog->SetSessionProperty("c3d.device.cpu", CPUChipset);
	cog->SetSessionProperty("c3d.device.gpu", GPUBrand);
	cog->SetSessionProperty("c3d.device.os", FPlatformMisc::GetOSVersion());
#endif


	const FPlatformMemoryConstants& MemoryConstants = FPlatformMemory::GetConstants();
	cog->SetSessionProperty("c3d.device.memory", (int)MemoryConstants.TotalPhysicalGB);

	FString HMDName = GetSpecificHMDFromHardware(GPUBrand, HMDDeviceName, CPUChipset, (int)MemoryConstants.TotalPhysicalGB);
	cog->SetSessionProperty("c3d.device.hmd.type", HMDName);

}

FString FUtil::GetSpecificHMDFromHardware(FString GPUBrand, FString HMDDeviceName, FString CPUChipset, int32 DeviceMemory)
{
	if (GPUBrand.Contains(TEXT("Adreno"), ESearchCase::IgnoreCase))
	{
		if (GPUBrand.Contains(TEXT("740"), ESearchCase::IgnoreCase))
		{
			if (CPUChipset.Contains(TEXT("eureka"), ESearchCase::IgnoreCase))
			{
				return TEXT("Meta Quest 3");
			}
			else if (CPUChipset.Contains(TEXT("panther"), ESearchCase::IgnoreCase))
			{
				return TEXT("Meta Quest 3S");
			}
			return TEXT("Meta Quest 3 -");
		}

		else if (GPUBrand.Contains(TEXT("650"), ESearchCase::IgnoreCase))
		{
			if ((HMDDeviceName.Contains(TEXT("Oculus"), ESearchCase::IgnoreCase)) || (HMDDeviceName.Contains(TEXT("Quest"), ESearchCase::IgnoreCase))
				|| (HMDDeviceName.Contains(TEXT("Meta"), ESearchCase::IgnoreCase)))
			{
				if (DeviceMemory == 12)
				{
					return TEXT("Meta Quest Pro");
				}
				else if (DeviceMemory == 6)
				{
					return TEXT("Meta Quest 2");
				}

				return TEXT("Meta Quest 2 -");
			}
			else if (HMDDeviceName.Contains(TEXT("Pico"), ESearchCase::IgnoreCase))
			{
				if (FPlatformMisc::GetCPUBrand().Contains(TEXT("3"), ESearchCase::IgnoreCase) && DeviceMemory == 8)
				{
					return TEXT("PICO Neo 3 Pro Eye");
				}
				if (DeviceMemory == 8)
				{
					return TEXT("PICO 4");
				}
				else if (DeviceMemory == 6)
				{
					return TEXT("PICO Neo 3");
				}
				return TEXT("PICO 4 -");
			}
			else if ((HMDDeviceName.Contains(TEXT("Vive"), ESearchCase::IgnoreCase)) || (HMDDeviceName.Contains(TEXT("Wave"), ESearchCase::IgnoreCase)))
			{
				if (DeviceMemory == 8)
				{
					return TEXT("HTC Vive Focus 3");
				}
				else if (DeviceMemory == 12)
				{
					return TEXT("HTC Vive XR Elite");
				}
				
				return TEXT("HTC Vive Focus 3 -");
			}

		}

		else if (GPUBrand.Contains(TEXT("630"), ESearchCase::IgnoreCase))
		{
			return TEXT("PICO Neo 2");
		}

		else if (GPUBrand.Contains(TEXT("540"), ESearchCase::IgnoreCase))
		{
			return TEXT("Meta Quest");
		}
	}


	// Apple Devices
	else if (GPUBrand.Contains(TEXT("Apple M2"), ESearchCase::IgnoreCase))
	{
		return TEXT("Apple Vision Pro");
	}

	// PC-tethered VR Headsets (Generalized)
	else if (GPUBrand.Contains(TEXT("NVIDIA"), ESearchCase::IgnoreCase) ||
		GPUBrand.Contains(TEXT("AMD"), ESearchCase::IgnoreCase))
	{
		return TEXT("PC-tethered VR Headset");
	}

	// Unknown or Unlisted Devices
	return TEXT("Unknown or Unsupported HMD");
}