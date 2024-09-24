/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/

#include "Cognitive3D/Private/C3DApi/SensorRecorder.h"
#include "Cognitive3D/Public/Cognitive3DBlueprints.h"
#include "Cognitive3D/Private/C3DNetwork/Network.h"

FSensors::FSensors()
{
	cog = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
}

void FSensors::StartSession()
{
	LastSensorValues.Empty();
	SensorDataPoints.Empty();
	sensorData.Empty();
	jsonPart = 1;

	FString ValueReceived;

	ValueReceived = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/Cognitive3D.Cognitive3DSettings", "SensorDataLimit", false);
	if (ValueReceived.Len() > 0)
	{
		int32 sensorLimit = FCString::Atoi(*ValueReceived);
		if (sensorLimit > 0)
		{
			SensorThreshold = sensorLimit;
		}
	}

	ValueReceived = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/Cognitive3D.Cognitive3DSettings", "SensorAutoTimer", false);
	if (ValueReceived.Len() > 0)
	{
		int32 parsedValue = FCString::Atoi(*ValueReceived);
		if (parsedValue > 0)
		{
			AutoTimer = parsedValue;
		}
	}

	auto world = ACognitive3DActor::GetCognitiveSessionWorld();
	if (world == nullptr)
	{
		GLog->Log("USensors::StartSession world from ACognitive3DActor is null!");
		return;
	}
	world->GetTimerManager().SetTimer(AutoSendHandle, FTimerDelegate::CreateRaw(this, &FSensors::SendData, false), AutoTimer, true);
}

void FSensors::InitializeSensor(FString sensorName, float hzRate, float initialValue)
{
	if (sensorData.Contains(sensorName))
	{
		return;
	}
	sensorData.Add(sensorName, new SensorData(sensorName, hzRate));
	SensorDataPoints.Add(sensorName, TArray<FString>());
	LastSensorValues.Add(sensorName, initialValue);
}

void FSensors::RecordSensor(FString Name, float value)
{
	if (FMath::IsNaN(value)) { return; }

	UWorld* world = ACognitive3DActor::GetCognitiveSessionWorld();
	if (world == nullptr) { return; }
	if (cog->CurrentTrackingSceneId.IsEmpty()) { return; }

	float realtime = UGameplayStatics::GetRealTimeSeconds(world);
	if (SensorDataPoints.Contains(Name))
	{
		//check time since world startup
		if (realtime < sensorData[Name]->NextRecordTime)
		{
			return; //recording above rate!
		}
		SensorDataPoints[Name].Add("[" + FString::SanitizeFloat(FUtil::GetTimestamp()) + "," + FString::SanitizeFloat(value) + "]");
	}
	else
	{
		InitializeSensor(Name, 10, value);
	}

	sensorData[Name]->NextRecordTime = (realtime + sensorData[Name]->UpdateInterval);
	LastSensorValues.Add(Name, (float)value);
	sensorDataCount ++;
	if (sensorDataCount >= SensorThreshold)
	{
		SendData(false);
	}
}

void FSensors::RecordSensor(FString Name, double value)
{
	if (FMath::IsNaN(value)) { return; }

	UWorld* world = ACognitive3DActor::GetCognitiveSessionWorld();
	if (world == nullptr) { return; }
	if (cog->CurrentTrackingSceneId.IsEmpty()) { return; }

	float realtime = UGameplayStatics::GetRealTimeSeconds(world);
	if (SensorDataPoints.Contains(Name))
	{
		//check time since world startup
		if (realtime < sensorData[Name]->NextRecordTime)
		{
			return; //recording above rate!
		}
		SensorDataPoints[Name].Add("[" + FString::SanitizeFloat(FUtil::GetTimestamp()) + "," + FString::SanitizeFloat(value) + "]");
	}
	else
	{
		InitializeSensor(Name, 10, value);
	}

	sensorData[Name]->NextRecordTime = (realtime + sensorData[Name]->UpdateInterval);
	LastSensorValues.Add(Name, (float)value);
	sensorDataCount++;
	if (sensorDataCount >= SensorThreshold)
	{
		SendData(false);
	}
}

void FSensors::SendData(bool copyDataToCache)
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

	LastSendTime = UCognitive3DBlueprints::GetSessionDuration();

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

	sensorDataCount = 0;
	for (auto& entry : SensorDataPoints)
	{
		entry.Value.Empty();
	}

	//if no data was serialized, skip sending an empty request
	if (allData.Len() == 0)
	{	
		return;
	}

	//remove final comma
	allData.RemoveAt(allData.Len() - 1);

	FString complete = "[" + allData + "]";
	const TCHAR* charcomplete = *complete;
	OutputString = OutputString.Replace(TEXT("\"SENSORDATAHERE\""), charcomplete);

	UE_LOG(LogTemp, Warning, TEXT("calling sensors network call"));
	cog->network->NetworkCall("sensors", OutputString, copyDataToCache);

	for (auto& entry : SensorDataPoints)
	{
		entry.Value.Empty();
	}
}

TMap<FString, float> FSensors::GetLastSensorValues()
{
	return LastSensorValues;
}

void FSensors::PreSessionEnd()
{
	//clean up auto send timer
	auto world = ACognitive3DActor::GetCognitiveSessionWorld();
	if (world == nullptr) { return; }
	world->GetTimerManager().ClearTimer(AutoSendHandle);
}

void FSensors::PostSessionEnd()
{
	
}