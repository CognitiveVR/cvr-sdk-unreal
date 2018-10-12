/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#include "Private/api/customevent.h"
#include "PlayerTracker.h"
#include "CognitiveVRSettings.h"

//using namespace cognitivevrapi;

cognitivevrapi::CustomEvent::CustomEvent(FAnalyticsProviderCognitiveVR* cvr)
{
	cog = cvr;
	FString ValueReceived;

	CustomEventBatchSize = cog->GetCognitiveSettings()->CustomEventBatchSize;
	ExtremeBatchSize = cog->GetCognitiveSettings()->CustomEventExtremeLimit;
	MinTimer = cog->GetCognitiveSettings()->CustomEventMinTimer;
	AutoTimer = cog->GetCognitiveSettings()->CustomEventAutoTimer;
	cog->GetWorld()->GetGameInstance()->GetTimerManager().SetTimer(AutoSendHandle, FTimerDelegate::CreateRaw(this, &CustomEvent::SendData), AutoTimer, false);
}

void cognitivevrapi::CustomEvent::Send(FString category)
{
	CustomEvent::Send(category, cog->GetPlayerHMDPosition(), NULL, "");
}

void cognitivevrapi::CustomEvent::Send(FString category, TSharedPtr<FJsonObject> properties)
{
	CustomEvent::Send(category, cog->GetPlayerHMDPosition(), properties, "");
}

void cognitivevrapi::CustomEvent::Send(FString category, FVector Position)
{
	CustomEvent::Send(category, Position, NULL, "");
}

void cognitivevrapi::CustomEvent::Send(FString category, FVector Position, TSharedPtr<FJsonObject> properties)
{
	CustomEvent::Send(category, Position, properties, "");
}

void cognitivevrapi::CustomEvent::Send(FString category, FString dynamicObjectId)
{
	CustomEvent::Send(category, cog->GetPlayerHMDPosition(), NULL, dynamicObjectId);
}

void cognitivevrapi::CustomEvent::Send(FString category, TSharedPtr<FJsonObject> properties, FString dynamicObjectId)
{
	CustomEvent::Send(category, cog->GetPlayerHMDPosition(), properties, dynamicObjectId);
}

void cognitivevrapi::CustomEvent::Send(FString category, FVector Position, FString dynamicObjectId)
{
	CustomEvent::Send(category, Position, NULL, dynamicObjectId);
}

void cognitivevrapi::CustomEvent::Send(FString category, FVector Position, TSharedPtr<FJsonObject> properties, FString dynamicObjectId)
{
	if (properties.Get() == NULL)
	{
		properties = MakeShareable(new FJsonObject);
	}

	TArray< TSharedPtr<FJsonValue> > ObjArray;
	TSharedPtr<FJsonValueArray> jsonArray = MakeShareable(new FJsonValueArray(ObjArray));

	double ts = Util::GetTimestamp();

	TArray<APlayerController*, FDefaultAllocator> controllers;
	GEngine->GetAllLocalPlayerControllers(controllers);

	if (controllers.Num() == 0 || controllers[0]->GetPawn() == NULL)
	{
		CognitiveLog::Warning("Transaction. local player controller does not have pawn. skip transaction on scene explorer");
		return;
	}

	TArray< TSharedPtr<FJsonValue> > pos;
	pos.Add(MakeShareable(new FJsonValueNumber((int32)-Position.X)));
	pos.Add(MakeShareable(new FJsonValueNumber((int32)Position.Z)));
	pos.Add(MakeShareable(new FJsonValueNumber((int32)Position.Y)));

	FJsonObject* eventObject = new FJsonObject;
	eventObject->SetStringField("name", category);
	eventObject->SetNumberField("time", ts);
	if (dynamicObjectId != "")
		eventObject->SetStringField("dynamicId", dynamicObjectId);
	eventObject->SetArrayField("point", pos);
	if (properties.Get()->Values.Num() > 0)
	{
		eventObject->SetObjectField("properties", properties);
	}

	TSharedPtr<FJsonObject>snapObj = MakeShareable(eventObject);
	events.Add(snapObj);

	if (events.Num() > CustomEventBatchSize)
	{
		TrySendData();
	}
}

void cognitivevrapi::CustomEvent::TrySendData()
{
	if (cog->GetWorld() != NULL)
	{
		bool withinMinTimer = LastSendTime + MinTimer > cog->GetWorld()->GetRealTimeSeconds();
		bool withinExtremeBatchSize = events.Num() < ExtremeBatchSize;

		if (withinMinTimer && withinExtremeBatchSize)
		{
			return;
		}
	}
}

void cognitivevrapi::CustomEvent::SendData()
{
	if (cog == NULL || !cog->HasStartedSession())
	{
		CognitiveLog::Warning("CustomEvent::Send - FAnalyticsProviderCognitiveVR is null!");
		return;
	}

	if (events.Num() == 0)
	{
		cog->GetWorld()->GetGameInstance()->GetTimerManager().SetTimer(AutoSendHandle, FTimerDelegate::CreateRaw(this, &CustomEvent::SendData), AutoTimer, false);
		return;
	}

	if (cog->GetWorld() != NULL)
	{
		LastSendTime = cog->GetWorld()->GetRealTimeSeconds();
	}

	cog->GetWorld()->GetGameInstance()->GetTimerManager().SetTimer(AutoSendHandle, FTimerDelegate::CreateRaw(this, &CustomEvent::SendData), AutoTimer, false);

	TSharedPtr<FJsonObject>wholeObj = MakeShareable(new FJsonObject);
	TArray<TSharedPtr<FJsonValue>> dataArray;

	wholeObj->SetStringField("userid", cog->GetUserID());
	if (!cog->LobbyId.IsEmpty())
	{
		wholeObj->SetStringField("lobbyId", cog->LobbyId);
	}
	wholeObj->SetNumberField("timestamp", (int32)cog->GetSessionTimestamp());
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
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(wholeObj.ToSharedRef(), Writer);
	cog->network->NetworkCall("events", OutputString);

	events.Empty();
}