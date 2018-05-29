/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#include "Private/api/customevent.h"
#include "PlayerTracker.h"

using namespace cognitivevrapi;

CustomEvent::CustomEvent(FAnalyticsProviderCognitiveVR* cvr)
{
	cog = cvr;
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
}

void CustomEvent::Send(FString category)
{
	CustomEvent::Send(category, cog->GetPlayerHMDPosition(), NULL);
}

void CustomEvent::Send(FString category, TSharedPtr<FJsonObject> properties)
{

	CustomEvent::Send(category, cog->GetPlayerHMDPosition(), properties);
}

void CustomEvent::Send(FString category, FVector Position)
{

	CustomEvent::Send(category, Position, NULL);
}

void CustomEvent::Send(FString category, FVector Position, TSharedPtr<FJsonObject> properties)
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

	//UPlayerTracker* up = controllers[0]->GetPawn()->FindComponentByClass<UPlayerTracker>();

	TArray< TSharedPtr<FJsonValue> > pos;
	pos.Add(MakeShareable(new FJsonValueNumber((int32)-Position.X)));
	pos.Add(MakeShareable(new FJsonValueNumber((int32)Position.Z)));
	pos.Add(MakeShareable(new FJsonValueNumber((int32)Position.Y)));

	FJsonObject* eventObject = new FJsonObject;
	eventObject->SetStringField("name", category);
	eventObject->SetNumberField("time", ts);
	eventObject->SetArrayField("point", pos);
	if (properties.Get()->Values.Num() > 0)
	{
		eventObject->SetObjectField("properties", properties);
	}

	TSharedPtr<FJsonObject>snapObj = MakeShareable(eventObject);
	events.Add(snapObj);

	if (events.Num() > CustomEventBatchSize)
	{
		SendData();
	}
}

void CustomEvent::SendData()
{
	if (cog == NULL || !cog->HasStartedSession())
	{
		CognitiveLog::Warning("CustomEvent::Send - FAnalyticsProviderCognitiveVR is null!");
		return;
	}

	if (events.Num() == 0)
	{
		return;
	}

	TSharedPtr<FJsonObject>wholeObj = MakeShareable(new FJsonObject);
	TArray<TSharedPtr<FJsonValue>> dataArray;

	wholeObj->SetStringField("userid", cog->GetUserID());
	wholeObj->SetNumberField("timestamp", (int32)cog->GetSessionTimestamp());
	wholeObj->SetStringField("sessionid", cog->GetSessionID());
	wholeObj->SetNumberField("part", jsonEventPart);
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