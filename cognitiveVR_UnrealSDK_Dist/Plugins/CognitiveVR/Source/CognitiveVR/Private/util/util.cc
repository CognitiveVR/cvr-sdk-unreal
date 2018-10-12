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

TSharedPtr<FJsonObject> Util::DeviceScraper(TSharedPtr<FJsonObject> properties)
{
	FString appName;
	GConfig->GetString(TEXT("/Script/EngineSettings.GeneralProjectSettings"), TEXT("ProjectName"), appName, GGameIni);
	properties->SetStringField("cvr.app.name", appName);

	FString appVersion = "1.0";
	GConfig->GetString(TEXT("/Script/EngineSettings.GeneralProjectSettings"), TEXT("ProjectVersion"), appVersion, GGameIni);
	properties->SetStringField("cvr.app.version", appVersion);

	FString engineVersion = FEngineVersion::Current().ToString().Replace(TEXT("+"), TEXT(" "));;
	properties->SetStringField("cvr.engine.version", engineVersion);

	properties->SetStringField("cvr.device.cpu.brand", FWindowsPlatformMisc::GetCPUBrand());
	properties->SetStringField("cvr.device.cpu.vendor", FWindowsPlatformMisc::GetCPUVendor());
	properties->SetNumberField("cvr.device.cores", FWindowsPlatformMisc::NumberOfCores());
	
	const FPlatformMemoryConstants& MemoryConstants = FPlatformMemory::GetConstants();
	properties->SetNumberField("cvr.device.memory (GB)", MemoryConstants.TotalPhysicalGB);

	FGPUDriverInfo info = FWindowsPlatformMisc::GetGPUDriverInfo(FWindowsPlatformMisc::GetPrimaryGPUBrand());
	properties->SetStringField("cvr.device.gpu.provider", info.ProviderName);
	properties->SetStringField("cvr.device.gpu", FWindowsPlatformMisc::GetPrimaryGPUBrand());
	properties->SetStringField("cvr.device.gpu.driver.version", info.UserDriverVersion);
	properties->SetStringField("cvr.device.gpu.driver.date", info.DriverDate);

	FString osVersionOut;
	FString osSubVersionOut;
	FWindowsPlatformMisc::GetOSVersions(osVersionOut, osSubVersionOut);

	properties->SetStringField("cvr.device.os", osVersionOut+osSubVersionOut);

	return properties;
}