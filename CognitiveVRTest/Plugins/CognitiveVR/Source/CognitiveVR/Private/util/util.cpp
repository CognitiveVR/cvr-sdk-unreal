/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/

#include "util/util.h"

double Util::GetTimestamp()
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

FString Util::GetDeviceName(FString DeviceName)
{
	if (DeviceName == "OculusRift")
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

void Util::SetSessionProperties()
{
	auto cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();

	FString HMDDeviceName = UHeadMountedDisplayFunctionLibrary::GetHMDDeviceName().ToString();
	cog->SetSessionProperty("c3d.device.hmd.type", HMDDeviceName);

#if defined TOBII_EYETRACKING_ACTIVE
	cog->SetSessionProperty("c3d.device.eyetracking.enabled", "true");
	cog->SetSessionProperty("c3d.device.eyetracking.type", "Tobii");
	cog->SetSessionProperty("c3d.app.sdktype", "Tobii");
#elif defined SRANIPAL_1_2_API
	cog->SetSessionProperty("c3d.device.eyetracking.enabled", "true");
	cog->SetSessionProperty("c3d.device.eyetracking.type", "Tobii");
	cog->SetSessionProperty("c3d.app.sdktype", "SRAnipal");
#elif defined SRANIPAL_1_3_API
	cog->SetSessionProperty("c3d.device.eyetracking.enabled", "true");
	cog->SetSessionProperty("c3d.device.eyetracking.type", "Tobii");
	cog->SetSessionProperty("c3d.app.sdktype", "SRAnipal");
#elif defined VARJOEYETRACKER_API
	cog->SetSessionProperty("c3d.device.eyetracking.enabled", "true");
	cog->SetSessionProperty("c3d.device.eyetracking.type", "Varjo");
	cog->SetSessionProperty("c3d.app.sdktype", "Varjo");
#elif defined PICOMOBILE_API
	//TODO check that pico eye tracking is enabled
	cog->SetSessionProperty("c3d.device.eyetracking.enabled", "true");
	cog->SetSessionProperty("c3d.device.eyetracking.type", "Tobii");
	cog->SetSessionProperty("c3d.app.sdktype", "Pico");
	cog->SetSessionProperty("c3d.device.hmd.type", FPlatformMisc::GetCPUBrand()); //returns pretty device name
#elif defined HPGLIA_API
	//TODO check that omnicept eye tracking is enabled
	cog->SetSessionProperty("c3d.device.eyetracking.enabled", "true");
	cog->SetSessionProperty("c3d.device.eyetracking.type", "Tobii");
	cog->SetSessionProperty("c3d.app.sdktype", "HP Omnicept");
#else
	cog->SetSessionProperty("c3d.device.eyetracking.enabled", "false");
	cog->SetSessionProperty("c3d.device.eyetracking.type", "None");
	cog->SetSessionProperty("c3d.app.sdktype", "Default");
#endif


	cog->SetSessionProperty("c3d.version", COGNITIVEVR_SDK_VERSION);
	cog->SetSessionProperty("c3d.app.engine", "Unreal");

	if (!cog->GetUserID().IsEmpty())
		cog->SetParticipantId(cog->GetUserID());
	cog->SetSessionProperty("c3d.deviceid", cog->GetDeviceID());

	const UGeneralProjectSettings& projectSettings = *GetDefault< UGeneralProjectSettings>();
	cog->SetSessionProperty("c3d.app.version", projectSettings.ProjectVersion);
	cog->SetSessionProperty("c3d.app.name", FApp::GetProjectName());

	FString engineVersion = FEngineVersion::Current().ToString().Replace(TEXT("+"), TEXT(" "));;
	cog->SetSessionProperty("c3d.app.engine.version", engineVersion);

	auto platformName = UGameplayStatics::GetPlatformName();
	if (platformName.Compare("Windows", ESearchCase::IgnoreCase) == 0 || platformName.Compare("Mac", ESearchCase::IgnoreCase) == 0 || platformName.Compare("Linux", ESearchCase::IgnoreCase) == 0)
	{
		cog->SetSessionProperty("c3d.device.type", "Desktop");
	}
	else if (platformName.Compare("IOS", ESearchCase::IgnoreCase) == 0 || platformName.Compare("Android", ESearchCase::IgnoreCase) == 0)
	{
		cog->SetSessionProperty("c3d.device.type", "Mobile");
	}
	else if (platformName.Compare("PS", ESearchCase::IgnoreCase) == 0 || platformName.Contains("xbox", ESearchCase::IgnoreCase) || platformName.Contains("Switch", ESearchCase::IgnoreCase))
	{
		cog->SetSessionProperty("c3d.device.type", "Console");
	}
	else
	{
		cog->SetSessionProperty("c3d.device.type", "Unknown");
	}

#if PLATFORM_ANDROID
	cog->SetSessionProperty("c3d.device.cpu", FPlatformMisc::GetCPUChipset());
	cog->SetSessionProperty("c3d.device.gpu", FPlatformMisc::GetPrimaryGPUBrand());
	cog->SetSessionProperty("c3d.device.os", FPlatformMisc::GetOSVersion());


#elif PLATFORM_WINDOWS
	cog->SetSessionProperty("c3d.device.cpu", FPlatformMisc::GetCPUBrand());
	cog->SetSessionProperty("c3d.device.gpu", FPlatformMisc::GetPrimaryGPUBrand());
	cog->SetSessionProperty("c3d.device.os", FPlatformMisc::GetOSVersion());
#endif

	const FPlatformMemoryConstants& MemoryConstants = FPlatformMemory::GetConstants();
	cog->SetSessionProperty("c3d.device.memory", (int)MemoryConstants.TotalPhysicalGB);
}