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
	double miliseconds = FDateTime::Now().GetMillisecond();
	double finalTime = ts + miliseconds*0.001;

	return finalTime;
	//http://stackoverflow.com/questions/997946/how-to-get-current-time-and-date-in-c

	#pragma warning(pop)
}

long Util::GetTimestampLong()
{
#pragma warning(push)
#pragma warning(disable:4244) //Disable warning regarding loss of accuracy, no concern.

	return time(0);
	//http://stackoverflow.com/questions/997946/how-to-get-current-time-and-date-in-c

#pragma warning(pop)
}

std::string Util::GetTimestampStr(long t)
{
    if (t == 0) {
        t = Util::GetTimestamp();
    }

    return Util::ToString(t);
}

template < typename T > std::string Util::ToString( const T& n )
{
    std::ostringstream stm ;
    stm << n ;
    return stm.str() ;
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


	//include nightmare to access gearvr class
	/*IHeadMountedDisplay* HMD = GEngine->HMDDevice.Get();
	if (HMD && HMD->GetHMDDeviceType() == EHMDDeviceType::DT_GearVR)
	{
	FGearVR* OculusHMD = static_cast<FGearVR*>(HMD);

	OculusHMD->StartOVRQuitMenu();
	}*/


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


void Util::AppendToJsonArray(TSharedPtr<FJsonValueArray> &json, FString &fstring)
{
	TArray<TSharedPtr<FJsonValue>> ValueArray = json.Get()->AsArray();
	TSharedPtr<FJsonValue> tempVal = MakeShareable(new FJsonValueString(fstring));
	ValueArray.Emplace(tempVal);

	json = MakeShareable(new FJsonValueArray(ValueArray));
}

void Util::AppendToJsonArray(TSharedPtr<FJsonValueArray> &json, std::string &stdstring)
{
	FString fstring = stdstring.c_str();
	TArray<TSharedPtr<FJsonValue>> ValueArray = json.Get()->AsArray();
	TSharedPtr<FJsonValue> tempVal = MakeShareable(new FJsonValueString(fstring));
	ValueArray.Emplace(tempVal);

	json = MakeShareable(new FJsonValueArray(ValueArray));
}

void Util::AppendToJsonArray(TSharedPtr<FJsonValueArray> &json, bool &mybool)
{
	TArray<TSharedPtr<FJsonValue>> ValueArray = json.Get()->AsArray();
	TSharedPtr<FJsonValue> tempVal = MakeShareable(new FJsonValueNumber(mybool));
	ValueArray.Emplace(tempVal);

	json = MakeShareable(new FJsonValueArray(ValueArray));
}

void Util::AppendToJsonArray(TSharedPtr<FJsonValueArray> &json, long &longValue)
{
	TArray<TSharedPtr<FJsonValue>> ValueArray = json.Get()->AsArray();
	TSharedPtr<FJsonValue> tempVal = MakeShareable(new FJsonValueNumber(longValue));
	ValueArray.Emplace(tempVal);

	json = MakeShareable(new FJsonValueArray(ValueArray));
}
void Util::AppendToJsonArray(TSharedPtr<FJsonValueArray> &json, int &intValue)
{
	TArray<TSharedPtr<FJsonValue>> ValueArray = json.Get()->AsArray();
	TSharedPtr<FJsonValue> tempVal = MakeShareable(new FJsonValueNumber(intValue));
	ValueArray.Emplace(tempVal);

	json = MakeShareable(new FJsonValueArray(ValueArray));
}
void Util::AppendToJsonArray(TSharedPtr<FJsonValueArray>& json, double &doubleValue)
{
	TArray<TSharedPtr<FJsonValue>> ValueArray = json.Get()->AsArray();
	TSharedPtr<FJsonValue> tempVal = MakeShareable(new FJsonValueNumber(doubleValue));
	ValueArray.Emplace(tempVal);

	json = MakeShareable(new FJsonValueArray(ValueArray));
}
void Util::AppendToJsonArray(TSharedPtr<FJsonValueArray>& json)
{
	TArray<TSharedPtr<FJsonValue>> ValueArray = json.Get()->AsArray();
	TSharedPtr<FJsonValue> tempVal = MakeShareable(new FJsonValueNumber(NULL));
	ValueArray.Emplace(tempVal);

	json = MakeShareable(new FJsonValueArray(ValueArray));
}
void Util::AppendToJsonArray(TSharedPtr<FJsonValueArray>& json, TSharedPtr<FJsonObject> & object)
{
	TArray<TSharedPtr<FJsonValue>> ValueArray = json.Get()->AsArray();
	TSharedPtr<FJsonValue> tempVal = MakeShareable(new FJsonValueObject(object));
	ValueArray.Emplace(tempVal);

	json = MakeShareable(new FJsonValueArray(ValueArray));
}

void Util::AppendToJsonObject(TSharedPtr<FJsonObject> &json, std::string &name, std::string &stdstringValue)
{
	FString fstring = stdstringValue.c_str();
	FString fkey = name.c_str();
	json.Get()->SetStringField(fkey, fstring);
}

void Util::AppendToJsonObject(TSharedPtr<FJsonObject> &json, std::string &name, FString &stringValue)
{
	FString fkey = name.c_str();
	json.Get()->SetStringField(fkey, stringValue);
}

void Util::AppendToJsonObject(TSharedPtr<FJsonObject> &json, std::string &name, bool &booleanValue)
{
	FString fkey = name.c_str();
	json.Get()->SetBoolField(fkey, booleanValue);
}

void Util::AppendToJsonObject(TSharedPtr<FJsonObject> &json, std::string &name, int &integerValue)
{
	FString fkey = name.c_str();
	json.Get()->SetNumberField(fkey, integerValue);
}

void Util::AppendToJsonObject(TSharedPtr<FJsonObject> &json, std::string &name, double &doubleValue)
{
	FString fkey = name.c_str();
	json.Get()->SetNumberField(fkey, doubleValue);
}