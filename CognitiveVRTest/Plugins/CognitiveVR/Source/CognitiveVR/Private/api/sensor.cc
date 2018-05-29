/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#include "Private/api/sensor.h"

using namespace cognitivevrapi;

Sensors::Sensors(FAnalyticsProviderCognitiveVR* sp)
{
	s = sp;

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

	sensorDataCount ++;
	if (sensorDataCount >= SensorThreshold)
	{
		Sensors::SendData();
	}
}

void Sensors::SendData()
{
	if (s == NULL || !s->HasStartedSession())
	{
		return;
	}

	TSharedPtr<FJsonObject> wholeObj = MakeShareable(new FJsonObject);

	TArray< TSharedPtr<FJsonValue> > DataArray;

	wholeObj->SetStringField("name", s->GetUserID());
	wholeObj->SetNumberField("timestamp", (int32)s->GetSessionTimestamp());
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
		return;
	}
	for (const auto& Entry : somedatapoints)
	{
		allData = allData.Append("{\"name\":\"" + Entry.Key + "\",\"data\":[" + Entry.Value + "]},");
	}
	allData.RemoveAt(allData.Len());

	FString complete = "[" + allData + "]";
	const TCHAR* charcomplete = *complete;
	OutputString = OutputString.Replace(TEXT("\"SENSORDATAHERE\""), charcomplete);

	s->network->NetworkCall("sensors", OutputString);

	somedatapoints.Empty();
	sensorDataCount = 0;
}