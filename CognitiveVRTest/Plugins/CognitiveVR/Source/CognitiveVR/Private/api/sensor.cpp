/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/

#include "CognitiveVR/Private/api/sensor.h"

Sensors::Sensors()
{
	cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();

	FString ValueReceived;
	LastSensorValues.Empty();

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
}

void Sensors::StartSession()
{
	if (!cog.IsValid()) {
		return;
	}
	if (cog->EnsureGetWorld() == NULL)
	{
		CognitiveLog::Warning("Sensors::StartSession - GetWorld is Null! Likely missing PlayerTrackerComponent on Player actor");
		return;
	}
	if (cog->EnsureGetWorld()->GetGameInstance() == NULL) {
		return;
	}
	cog->EnsureGetWorld()->GetGameInstance()->GetTimerManager().SetTimer(AutoSendHandle, FTimerDelegate::CreateRaw(this, &Sensors::SendData, false), AutoTimer, true);
}

void Sensors::InitializeSensor(FString sensorName, float hzRate, float initialValue)
{
	if (sensorData.Contains(sensorName))
	{
		return;
	}
	sensorData.Add(sensorName, new SensorData(sensorName, hzRate));
	SensorDataPoints.Add(sensorName, TArray<FString>());
	LastSensorValues.Add(sensorName, initialValue);
}

void Sensors::RecordSensor(FString Name, float value)
{
	float time = UGameplayStatics::GetRealTimeSeconds(cog->GetWorld());
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
		Sensors::TrySendData();
	}
}

void Sensors::RecordSensor(FString Name, double value)
{
	float time = UGameplayStatics::GetRealTimeSeconds(cog->GetWorld());
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
		Sensors::TrySendData();
	}
}

void Sensors::TrySendData()
{
	if (cog->EnsureGetWorld() != NULL)
	{
		bool withinMinTimer = LastSendTime + MinTimer > UCognitiveVRBlueprints::GetSessionDuration();
		bool withinExtremeBatchSize = sensorDataCount < ExtremeBatchSize;
		
		if (withinMinTimer && withinExtremeBatchSize)
		{
			return;
		}
	}
	SendData();
}

void Sensors::SendData(bool copyDataToCache)
{
	if (!cog.IsValid() || !cog->HasStartedSession())
	{
		return;
	}

	if (SensorDataPoints.Num() == 0)
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
	//remove final comma
	allData.RemoveAt(allData.Len()-1);

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

TMap<FString, float> Sensors::GetLastSensorValues()
{
	return LastSensorValues;
}