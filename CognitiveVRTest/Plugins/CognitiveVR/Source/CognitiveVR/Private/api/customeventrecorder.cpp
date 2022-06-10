/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#include "CognitiveVR/Private/api/customeventrecorder.h"

uint64 CustomEventRecorder::lastFrameCount = 0;
int32 CustomEventRecorder::consecutiveFrame = 0;

CustomEventRecorder::CustomEventRecorder()
{
	cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	FString ValueReceived;

	ValueReceived = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "CustomEventBatchSize", false);
	if (ValueReceived.Len() > 0)
	{
		int32 customEventLimit = FCString::Atoi(*ValueReceived);
		if (customEventLimit > 0)
		{
			CustomEventBatchSize = customEventLimit;
		}
	}

	ValueReceived = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "CustomEventExtremeLimit", false);
	if (ValueReceived.Len() > 0)
	{
		int32 parsedValue = FCString::Atoi(*ValueReceived);
		if (parsedValue > 0)
		{
			ExtremeBatchSize = parsedValue;
		}
	}

	ValueReceived = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "CustomEventMinTimer", false);
	if (ValueReceived.Len() > 0)
	{
		int32 parsedValue = FCString::Atoi(*ValueReceived);
		if (parsedValue > 0)
		{
			MinTimer = parsedValue;
		}
	}

	ValueReceived = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "CustomEventAutoTimer", false);
	if (ValueReceived.Len() > 0)
	{
		int32 parsedValue = FCString::Atoi(*ValueReceived);
		if (parsedValue > 0)
		{
			AutoTimer = parsedValue;
		}
	}
}

void CustomEventRecorder::StartSession()
{
	if (!cog.IsValid()) {
		return;
	}
	if (cog->EnsureGetWorld() == NULL)
	{
		CognitiveLog::Warning("CustomEvent::StartSession - GetWorld is Null! Likely missing PlayerTrackerComponent on Player actor");
		return;
	}
	if (cog->EnsureGetWorld()->GetGameInstance() == NULL) {
		return;
	}
	cog->EnsureGetWorld()->GetGameInstance()->GetTimerManager().SetTimer(AutoSendHandle, FTimerDelegate::CreateRaw(this, &CustomEventRecorder::SendData, false), AutoTimer, true);
}

void CustomEventRecorder::Send(FString category)
{
	CustomEventRecorder::Send(category, cog->GetPlayerHMDPosition(), NULL, "");
}

void CustomEventRecorder::Send(FString category, TSharedPtr<FJsonObject> properties)
{
	CustomEventRecorder::Send(category, cog->GetPlayerHMDPosition(), properties, "");
}

void CustomEventRecorder::Send(FString category, FVector Position)
{
	CustomEventRecorder::Send(category, Position, NULL, "");
}

void CustomEventRecorder::Send(FString category, FVector Position, TSharedPtr<FJsonObject> properties)
{
	CustomEventRecorder::Send(category, Position, properties, "");
}

void CustomEventRecorder::Send(FString category, FString dynamicObjectId)
{
	CustomEventRecorder::Send(category, cog->GetPlayerHMDPosition(), NULL, dynamicObjectId);
}

void CustomEventRecorder::Send(FString category, TSharedPtr<FJsonObject> properties, FString dynamicObjectId)
{
	CustomEventRecorder::Send(category, cog->GetPlayerHMDPosition(), properties, dynamicObjectId);
}

void CustomEventRecorder::Send(FString category, FVector Position, FString dynamicObjectId)
{
	CustomEventRecorder::Send(category, Position, NULL, dynamicObjectId);
}

void CustomEventRecorder::Send(UCustomEvent* customEvent)
{
	if (customEvent == NULL) { return; }
	TSharedPtr<FJsonObject> jsonObject = MakeShareable(new FJsonObject);
	if (customEvent->BoolProperties.Num() > 0 || customEvent->FloatProperties.Num() > 0 || customEvent->IntegerProperties.Num() > 0 || customEvent->StringProperties.Num() > 0)
	{
		//properties, combine into json object
		for (auto &Elem : customEvent->BoolProperties)
			jsonObject->SetBoolField(Elem.Key, Elem.Value);
		for (auto &Elem : customEvent->IntegerProperties)
			jsonObject->SetNumberField(Elem.Key, Elem.Value);
		for (auto &Elem : customEvent->FloatProperties)
			jsonObject->SetNumberField(Elem.Key, Elem.Value);
		for (auto &Elem : customEvent->StringProperties)
			jsonObject->SetStringField(Elem.Key, Elem.Value);
	}

	CustomEventRecorder::Send(customEvent->Category, customEvent->Position, jsonObject, customEvent->DynamicId);
}

void CustomEventRecorder::Send(FString category, FVector Position, TSharedPtr<FJsonObject> properties, FString dynamicObjectId)
{
	if (lastFrameCount >= GFrameCounter - 1)
	{
		if (lastFrameCount != GFrameCounter)
		{
			lastFrameCount = GFrameCounter;
			consecutiveFrame++;
			if (consecutiveFrame > 200)
			{
				CognitiveLog::Warning("Cognitive3D receiving Custom Events every frame. This is not a recommended method for implementation!\nPlease see docs.cognitive3d.com/unreal/customevents");
			}
		}
	}
	else
	{
		lastFrameCount = GFrameCounter;
		consecutiveFrame = 0;
	}

	if (!cog.IsValid() || !cog->HasStartedSession())
	{
		CognitiveLog::Warning("CustomEvent::Send - FAnalyticsProviderCognitiveVR is null!");
		return;
	}

	if (properties.Get() == NULL)
	{
		properties = MakeShareable(new FJsonObject);
	}

	TArray< TSharedPtr<FJsonValue> > ObjArray;
	TSharedPtr<FJsonValueArray> jsonArray = MakeShareable(new FJsonValueArray(ObjArray));

	double ts = Util::GetTimestamp();
	TArray< TSharedPtr<FJsonValue> > pos;
	pos.Add(MakeShareable(new FJsonValueNumber(-Position.X)));
	pos.Add(MakeShareable(new FJsonValueNumber(Position.Z)));
	pos.Add(MakeShareable(new FJsonValueNumber(Position.Y)));

	TSharedPtr<FJsonObject>eventObject = MakeShareable(new FJsonObject);
	eventObject->SetStringField("name", category);
	eventObject->SetNumberField("time", ts);
	if (dynamicObjectId != "")
		eventObject->SetStringField("dynamicId", dynamicObjectId);
	eventObject->SetArrayField("point", pos);
	if (properties.Get()->Values.Num() > 0)
	{
		eventObject->SetObjectField("properties", properties);
	}

	events.Add(eventObject);

	if (events.Num() > CustomEventBatchSize)
	{
		TrySendData();
	}
}

void CustomEventRecorder::TrySendData()
{
	if (cog->EnsureGetWorld() != NULL)
	{
		bool withinMinTimer = LastSendTime + MinTimer > UCognitiveVRBlueprints::GetSessionDuration();
		bool withinExtremeBatchSize = events.Num() < ExtremeBatchSize;

		if (withinMinTimer && withinExtremeBatchSize)
		{
			return;
		}
		SendData();
	}
}

void CustomEventRecorder::SendData(bool copyDataToCache)
{
	if (!cog.IsValid() || !cog->HasStartedSession())
	{
		CognitiveLog::Warning("CustomEvent::SendData - FAnalyticsProviderCognitiveVR is null!");
		return;
	}

	if (events.Num() == 0)
	{
		return;
	}

	LastSendTime = UCognitiveVRBlueprints::GetSessionDuration();

	TSharedPtr<FJsonObject>wholeObj = MakeShareable(new FJsonObject);
	TArray<TSharedPtr<FJsonValue>> dataArray;

	wholeObj->SetStringField("userid", cog->GetUserID());
	if (!cog->LobbyId.IsEmpty())
	{
		wholeObj->SetStringField("lobbyId", cog->LobbyId);
	}
	wholeObj->SetNumberField("timestamp", cog->GetSessionTimestamp());
	wholeObj->SetStringField("sessionid", cog->GetSessionID());
	wholeObj->SetNumberField("part", jsonEventPart);
	wholeObj->SetStringField("formatversion", "1.0");
	jsonEventPart++;

	for (int32 i = 0; i != events.Num(); ++i)
	{
		dataArray.Add(MakeShareable(new FJsonValueObject(events[i])));
	}

	wholeObj->SetArrayField("data", dataArray);

	FString OutputString;
	auto Writer = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&OutputString);
	FJsonSerializer::Serialize(wholeObj.ToSharedRef(), Writer);
	cog->network->NetworkCall("events", OutputString, copyDataToCache);

	events.Empty();
}