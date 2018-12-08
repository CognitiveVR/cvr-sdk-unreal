/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#include "util/util.h"
//#include "HeadMountedDisplay.h"
#include "GenericPlatformDriver.h"

using namespace cognitivevrapi;

double Util::GetTimestamp()
{
	#pragma warning(push)
	#pragma warning(disable:4244) //Disable warning regarding loss of accuracy, no concern.

	long ts = time(0);
	double miliseconds = FDateTime::UtcNow().GetMillisecond();
	double finalTime = ts + miliseconds*0.001;

	return finalTime;
	//http://stackoverflow.com/questions/997946/how-to-get-current-time-and-date-in-c

	#pragma warning(pop)
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

void Util::SetHardwareSessionProperties()
{
	auto cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider();

	FString appName;
	GConfig->GetString(TEXT("/Script/EngineSettings.GeneralProjectSettings"), TEXT("ProjectName"), appName, GGameIni);

	cog->SetSessionProperty("c3d.app.name", appName);

	FString appVersion = "1.0";
	GConfig->GetString(TEXT("/Script/EngineSettings.GeneralProjectSettings"), TEXT("ProjectVersion"), appVersion, GGameIni);
	cog->SetSessionProperty("c3d.app.version", appVersion);

	FString engineVersion = FEngineVersion::Current().ToString().Replace(TEXT("+"), TEXT(" "));;
	cog->SetSessionProperty("c3d.app.engine.version", engineVersion);

	cog->SetSessionProperty("c3d.device.type", "Desktop"); //TODO handheld/desktop/console

	cog->SetSessionProperty("c3d.device.cpu", FWindowsPlatformMisc::GetCPUBrand());

	//TODO device model

	cog->SetSessionProperty("c3d.device.gpu", FWindowsPlatformMisc::GetPrimaryGPUBrand());

	FString osVersionOut;
	FString osSubVersionOut;
	FWindowsPlatformMisc::GetOSVersions(osVersionOut, osSubVersionOut);
	cog->SetSessionProperty("c3d.device.os", osVersionOut + " " + osSubVersionOut);

	const FPlatformMemoryConstants& MemoryConstants = FPlatformMemory::GetConstants();
	cog->SetSessionProperty("c3d.device.memory", (int)MemoryConstants.TotalPhysicalGB);
}