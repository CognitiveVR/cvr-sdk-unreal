/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#include "Private/api/sensor.h"

//using namespace cognitivevrapi;

cognitivevrapi::Sensors::Sensors(FAnalyticsProviderCognitiveVR* sp)
{
	cog = sp;

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
}

void cognitivevrapi::Sensors::StartSession()
{
	if (cog->GetWorld() == NULL)
	{
		CognitiveLog::Warning("Sensors::StartSession - GetWorld is Null! Likely missing PlayerTrackerComponent on Player actor");
		return;
	}

	cog->GetWorld()->GetGameInstance()->GetTimerManager().SetTimer(AutoSendHandle, FTimerDelegate::CreateRaw(this, &Sensors::SendData), AutoTimer, false);
}

void cognitivevrapi::Sensors::RecordSensor(FString Name, float value)
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
		Sensors::TrySendData();
	}
}

void cognitivevrapi::Sensors::TrySendData()
{
	if (cog->GetWorld() != NULL)
	{
		bool withinMinTimer = LastSendTime + MinTimer > cog->GetWorld()->GetRealTimeSeconds();
		bool withinExtremeBatchSize = sensorDataCount < ExtremeBatchSize;
		
		if (withinMinTimer && withinExtremeBatchSize)
		{
			return;
		}
	}
	SendData();
}

void cognitivevrapi::Sensors::SendData()
{
	if (cog == NULL || !cog->HasStartedSession())
	{
		return;
	}

	if (somedatapoints.Num() == 0)
	{
		cog->GetWorld()->GetGameInstance()->GetTimerManager().SetTimer(AutoSendHandle, FTimerDelegate::CreateRaw(this, &Sensors::SendData), AutoTimer, false);
		return;
	}

	if (cog->GetWorld() != NULL)
	{
		LastSendTime = cog->GetWorld()->GetRealTimeSeconds();
	}

	cog->GetWorld()->GetGameInstance()->GetTimerManager().SetTimer(AutoSendHandle, FTimerDelegate::CreateRaw(this, &Sensors::SendData), AutoTimer, false);

	TSharedPtr<FJsonObject> wholeObj = MakeShareable(new FJsonObject);

	TArray< TSharedPtr<FJsonValue> > DataArray;

	wholeObj->SetStringField("name", cog->GetUserID());
	if (!cog->LobbyId.IsEmpty())
	{
		wholeObj->SetStringField("lobbyId", cog->LobbyId);
	}
	wholeObj->SetNumberField("timestamp", (int32)cog->GetSessionTimestamp());
	wholeObj->SetStringField("sessionid", cog->GetSessionID());
	wholeObj->SetNumberField("part", jsonPart);
	wholeObj->SetStringField("formatversion", "1.0");
	jsonPart++;

	wholeObj->SetStringField("data", "SENSORDATAHERE");

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(wholeObj.ToSharedRef(), Writer);

	//TODO use ustruct with array to format this to json instead of doing it manually
	FString allData;

	for (const auto& Entry : somedatapoints)
	{
		allData = allData.Append("{\"name\":\"" + Entry.Key + "\",\"data\":[" + Entry.Value + "]},");
	}
	allData.RemoveAt(allData.Len());

	FString complete = "[" + allData + "]";
	const TCHAR* charcomplete = *complete;
	OutputString = OutputString.Replace(TEXT("\"SENSORDATAHERE\""), charcomplete);

	cog->network->NetworkCall("sensors", OutputString);

	somedatapoints.Empty();
	sensorDataCount = 0;
}