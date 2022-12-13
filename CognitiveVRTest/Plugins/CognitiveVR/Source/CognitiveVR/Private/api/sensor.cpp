/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/

#include "CognitiveVR/Private/api/sensor.h"

USensors::USensors()
{

}

void USensors::Initialize()
{
	LastSensorValues.Empty();
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

	ValueReceived = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "SensorExtremeLimit", false);
	if (ValueReceived.Len() > 0)
	{
		int32 parsedValue = FCString::Atoi(*ValueReceived);
		if (parsedValue > 0)
		{
			ExtremeBatchSize = parsedValue;
		}
	}

	ValueReceived = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "SensorMinTimer", false);
	if (ValueReceived.Len() > 0)
	{
		int32 parsedValue = FCString::Atoi(*ValueReceived);
		if (parsedValue > 0)
		{
			MinTimer = parsedValue;
		}
	}

	ValueReceived = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "SensorAutoTimer", false);
	if (ValueReceived.Len() > 0)
	{
		int32 parsedValue = FCString::Atoi(*ValueReceived);
		if (parsedValue > 0)
		{
			AutoTimer = parsedValue;
		}
	}

	cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
}

void USensors::StartSession()
{
	auto world = ACognitiveVRActor::GetCognitiveSessionWorld();
	if (world == nullptr)
	{
		GLog->Log("USensors::StartSession world from ACognitiveVRActor is null!");
		return;
	}
	world->GetTimerManager().SetTimer(AutoSendHandle, FTimerDelegate::CreateRaw(this, &USensors::SendData, false), AutoTimer, true);
}

void USensors::InitializeSensor(FString sensorName, float hzRate, float initialValue)
{
	if (sensorData.Contains(sensorName))
	{
		return;
	}
	sensorData.Add(sensorName, new SensorData(sensorName, hzRate));
	SensorDataPoints.Add(sensorName, TArray<FString>());
	LastSensorValues.Add(sensorName, initialValue);
}

void USensors::RecordSensor(FString Name, float value)
{
	UWorld* world = ACognitiveVRActor::GetCognitiveSessionWorld();

	float time = UGameplayStatics::GetRealTimeSeconds(world);
	if (SensorDataPoints.Contains(Name))
	{
		//check time since world startup
		if (time < sensorData[Name]->NextRecordTime)
		{
			return; //recording above rate!
		}
		SensorDataPoints[Name].Add("[" + FString::SanitizeFloat(Util::GetTimestamp()) + "," + FString::SanitizeFloat(value) + "]");
	}
	else
	{
		InitializeSensor(Name, 10, value);
	}

	sensorData[Name]->NextRecordTime = (time + sensorData[Name]->UpdateInterval);
	LastSensorValues.Add(Name, (float)value);
	sensorDataCount ++;
	if (sensorDataCount >= SensorThreshold)
	{
		USensors::TrySendData();
	}
}

void USensors::RecordSensor(FString Name, double value)
{
	UWorld* world = ACognitiveVRActor::GetCognitiveSessionWorld();
	float time = UGameplayStatics::GetRealTimeSeconds(world);
	if (SensorDataPoints.Contains(Name))
	{
		//check time since world startup
		if (time < sensorData[Name]->NextRecordTime)
		{
			return; //recording above rate!
		}
		SensorDataPoints[Name].Add("[" + FString::SanitizeFloat(Util::GetTimestamp()) + "," + FString::SanitizeFloat(value) + "]");
	}
	else
	{
		InitializeSensor(Name, 10, value);
	}

	sensorData[Name]->NextRecordTime = (time + sensorData[Name]->UpdateInterval);
	LastSensorValues.Add(Name, (float)value);
	sensorDataCount++;
	if (sensorDataCount >= SensorThreshold)
	{
		USensors::TrySendData();
	}
}

void USensors::TrySendData()
{
	bool withinMinTimer = LastSendTime + MinTimer > UCognitiveVRBlueprints::GetSessionDuration();
	bool withinExtremeBatchSize = sensorDataCount < ExtremeBatchSize;

	if (withinMinTimer && withinExtremeBatchSize)
	{
		return;
	}
	SendData(false);
}

void USensors::SendData(bool copyDataToCache)
{
	if (!cog.IsValid() || !cog->HasStartedSession())
	{
		return;
	}
	//SensorDataPoints indicates the different sensors as key/values. can have empty lists of data, so check sensorDataCount too
	if (SensorDataPoints.Num() == 0 || sensorDataCount == 0)
	{
		return;
	}

	LastSendTime = UCognitiveVRBlueprints::GetSessionDuration();

	TSharedPtr<FJsonObject> wholeObj = MakeShareable(new FJsonObject);

	wholeObj->SetStringField("name", cog->GetUserID());
	if (!cog->LobbyId.IsEmpty())
	{
		wholeObj->SetStringField("lobbyId", cog->LobbyId);
	}
	wholeObj->SetNumberField("timestamp", cog->GetSessionTimestamp());
	wholeObj->SetStringField("sessionid", cog->GetSessionID());
	wholeObj->SetNumberField("part", jsonPart);
	wholeObj->SetStringField("formatversion", "2.0");
	jsonPart++;

	wholeObj->SetStringField("data", "SENSORDATAHERE");

	FString OutputString;
	auto Writer = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&OutputString);
	FJsonSerializer::Serialize(wholeObj.ToSharedRef(), Writer);

	FString allData;
	int32 dataEntries = 0;

	for (const auto& Entry : SensorDataPoints)
	{
		if (Entry.Value.Num() == 0) { continue; } //skip sensors that don't have any new data points

		dataEntries++;
		allData = allData.Append("{\"name\":\"" + Entry.Key + "\",");

		if (sensorData.Contains(Entry.Key))
		{
			allData = allData.Append("\"sensorHzLimitType\":\"" + FString(sensorData[Entry.Key]->Rate) + "\",");
			if (sensorData[Entry.Key]->UpdateInterval >= 0.1f)
			{
				allData = allData.Append("\"sensorHzLimited\":\"true\",");
			}
		}

		allData.Append("\"data\":[");

		for (int32 i = 0; i < Entry.Value.Num(); i++)
		{
			if (i != 0)
				allData.Append(",");
			allData.Append(Entry.Value[i]);
		}

		allData.Append("]},");
	}

	//if no data was serialized, skip sending an empty request
	if (allData.Len() == 0)
	{
		for (auto& entry : SensorDataPoints)
		{
			entry.Value.Empty();
		}
		sensorDataCount = 0;
		return;
	}

	//remove final comma
	allData.RemoveAt(allData.Len() - 1);

	FString complete = "[" + allData + "]";
	const TCHAR* charcomplete = *complete;
	OutputString = OutputString.Replace(TEXT("\"SENSORDATAHERE\""), charcomplete);

	cog->network->NetworkCall("sensors", OutputString, copyDataToCache);

	for (auto& entry : SensorDataPoints)
	{
		entry.Value.Empty();
	}
	sensorDataCount = 0;
}

TMap<FString, float> USensors::GetLastSensorValues()
{
	return LastSensorValues;
}

void USensors::PreSessionEnd()
{
	//clean up auto send timer
	auto world = ACognitiveVRActor::GetCognitiveSessionWorld();
	if (world == nullptr) { return; }
	world->GetTimerManager().ClearTimer(AutoSendHandle);
}

void USensors::PostSessionEnd()
{
	cog.Reset();
}