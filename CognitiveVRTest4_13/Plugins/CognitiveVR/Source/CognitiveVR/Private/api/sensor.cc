/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#include "Private/api/sensor.h"

using namespace cognitivevrapi;

class UPlayerTracker
{
public:
	static void SendJson(FString url, FString json);
};

Sensors::Sensors(FAnalyticsProviderCognitiveVR* sp)
{
	s = sp;
	CognitiveLog::Warning("Sensor::Sensor - INITIALIZED");

	FString ValueReceived;

	ValueReceived = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "SensorDataLimit", false);

	if (ValueReceived.Len() > 0)
	{
		int32 sensorLimit = FCString::Atoi(*ValueReceived);
		if (sensorLimit > 0)
		{
			SensorThreshold = sensorLimit;
		}
	}
}

void Sensors::RecordSensor(FString Name, float value)
{
	if (somedatapoints.Contains(Name))
	{
		somedatapoints[Name].Append(",[" + FString::SanitizeFloat(Util::GetTimestamp()) + "," + FString::SanitizeFloat(value) + "]");
	}
	else
	{
		somedatapoints.Emplace(Name, "[" + FString::SanitizeFloat(Util::GetTimestamp()) + "," + FString::SanitizeFloat(value) + "]");
	}
	

	/*TArray<FJsonValueNumber> sensorData;
	sensorData.Init(0, 2);
	sensorData[0] = Util::GetTimestamp();
	sensorData[1] = value;

	//[x.xxxx,y.yyyy]

	if (sensorDataJson.Contains(Name))
	{
		TArray<TArray<FJsonValueNumber>>* index = sensorDataJson.Find(Name);
		index->Add(sensorData);
	}
	else
	{
		sensorDataJson.Emplace(Name).Add(sensorData);
	}*/

	sensorDataCount ++;
	if (sensorDataCount >= SensorThreshold)
	{
		Sensors::SendData();
	}
}

void Sensors::SendData()
{
	FString out = SensorDataToString();
	somedatapoints.Empty();
	sensorDataCount = 0;
	if (out != "")
	{
		UPlayerTracker::SendJson("sensors", out);
	}
}

FString Sensors::SensorDataToString()
{
	TSharedPtr<FJsonObject> wholeObj = MakeShareable(new FJsonObject);

	TArray< TSharedPtr<FJsonValue> > DataArray;

	wholeObj->SetStringField("name", s->GetDeviceID());
	wholeObj->SetNumberField("timestamp", s->GetSessionTimestamp());
	wholeObj->SetStringField("sessionid", s->GetSessionID());
	wholeObj->SetNumberField("part", jsonPart);
	jsonPart++;
	
	wholeObj->SetStringField("data", "SENSORDATAHERE");

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(wholeObj.ToSharedRef(), Writer);

	//TODO use ustruct with array to format this to json instead of doing it manually
	FString allData;
	if (somedatapoints.Num() == 0)
	{
		CognitiveLog::Info("Sensors::SensorDataToString 0 datapoints to write!");
		return "";
	}
	for (const auto& Entry : somedatapoints)
	{
		allData = allData.Append("{\"name\":\""+Entry.Key + "\",\"data\":[" + Entry.Value + "]},");
	}
	allData.RemoveAt(allData.Len());

	FString complete = "[" + allData + "]";
	const TCHAR* charcomplete = *complete;
	OutputString = OutputString.Replace(TEXT("\"SENSORDATAHERE\""), charcomplete);

	return OutputString;
}