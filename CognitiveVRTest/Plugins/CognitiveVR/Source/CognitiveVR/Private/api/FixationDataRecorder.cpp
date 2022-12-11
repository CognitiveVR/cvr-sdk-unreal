/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#include "CognitiveVR/Private/api/FixationDataRecorder.h"

//called at module startup to create a default uobject of this type
UFixationDataRecorder::UFixationDataRecorder()
{
}

void UFixationDataRecorder::Initialize()
{
	FString ValueReceived;

	ValueReceived = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "FixationBatchSize", false);
	if (ValueReceived.Len() > 0)
	{
		int32 fixationLimit = FCString::Atoi(*ValueReceived);
		if (fixationLimit > 0)
		{
			FixationBatchSize = fixationLimit;
		}
	}

	ValueReceived = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "FixationExtremeLimit", false);
	if (ValueReceived.Len() > 0)
	{
		int32 parsedValue = FCString::Atoi(*ValueReceived);
		if (parsedValue > 0)
		{
			ExtremeBatchSize = parsedValue;
		}
	}

	ValueReceived = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "FixationMinTimer", false);
	if (ValueReceived.Len() > 0)
	{
		int32 parsedValue = FCString::Atoi(*ValueReceived);
		if (parsedValue > 0)
		{
			MinTimer = parsedValue;
		}
	}

	ValueReceived = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "FixationAutoTimer", false);
	if (ValueReceived.Len() > 0)
	{
		int32 parsedValue = FCString::Atoi(*ValueReceived);
		if (parsedValue > 0)
		{
			AutoTimer = parsedValue;
		}
	}

	cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	cog->OnRequestSend.AddDynamic(this, &UFixationDataRecorder::SendData);
	cog->OnSessionBegin.AddDynamic(this, &UFixationDataRecorder::StartSession);
	cog->OnPreSessionEnd.AddDynamic(this, &UFixationDataRecorder::PreSessionEnd);
	cog->OnPostSessionEnd.AddDynamic(this, &UFixationDataRecorder::PostSessionEnd);
}

void UFixationDataRecorder::StartSession()
{
	auto world = ACognitiveVRActor::GetCognitiveSessionWorld();
	if (world == nullptr)
	{
		GLog->Log("UFixationDataRecorder::StartSession world from ACognitiveVRActor is null!");
		return;
	}
	world->GetTimerManager().SetTimer(AutoSendHandle, FTimerDelegate::CreateUObject(this, &UFixationDataRecorder::SendData, false), AutoTimer, true);
}

void UFixationDataRecorder::RecordFixationEnd(const FFixation& fixation)
{
	Fixations.Add(fixation);
	if (Fixations.Num() > FixationBatchSize)
	{
		SendData(false);
	}
}

void UFixationDataRecorder::SendData(bool copyDataToCache)
{
	if (!cog.IsValid() || !cog->HasStartedSession()) { return; }
	if (cog->GetCurrentSceneVersionNumber().Len() == 0) { return; }
	if (!cog->GetCurrentSceneData().IsValid()) { return; }

	if (Fixations.Num() == 0) { return; }

	TSharedPtr<FJsonObject> wholeObj = MakeShareable(new FJsonObject);

	wholeObj->SetStringField("userid", cog->GetUserID());
	wholeObj->SetStringField("sessionid", cog->GetSessionID());
	wholeObj->SetNumberField("timestamp", cog->GetSessionTimestamp());
	wholeObj->SetNumberField("part", jsonPart);
	jsonPart++;
	wholeObj->SetStringField("formatversion", "1.0");

	TArray<TSharedPtr<FJsonValue>> dataArray;
	for (int32 i = 0; i != Fixations.Num(); ++i)
	{
		//write fixation to json
		TSharedPtr<FJsonObject>fixObj = MakeShareable(new FJsonObject);

		double d = (double)Fixations[i].StartMs / 1000.0;

		fixObj->SetNumberField("time", d);
		fixObj->SetNumberField("duration", Fixations[i].DurationMs);
		fixObj->SetNumberField("maxradius", Fixations[i].MaxRadius);

		if (Fixations[i].IsLocal)
		{
			TArray<TSharedPtr<FJsonValue>> posArray;
			TSharedPtr<FJsonValueNumber> JsonValue;
			JsonValue = MakeShareable(new FJsonValueNumber(-Fixations[i].LocalPosition.X)); //right
			posArray.Add(JsonValue);
			JsonValue = MakeShareable(new FJsonValueNumber(Fixations[i].LocalPosition.Z)); //up
			posArray.Add(JsonValue);
			JsonValue = MakeShareable(new FJsonValueNumber(Fixations[i].LocalPosition.Y));  //forward
			posArray.Add(JsonValue);

			fixObj->SetArrayField("p", posArray);

			fixObj->SetStringField("objectid", Fixations[i].DynamicObjectId);
		}
		else
		{
			TArray<TSharedPtr<FJsonValue>> posArray;
			TSharedPtr<FJsonValueNumber> JsonValue;
			JsonValue = MakeShareable(new FJsonValueNumber(-Fixations[i].WorldPosition.X)); //right
			posArray.Add(JsonValue);
			JsonValue = MakeShareable(new FJsonValueNumber(Fixations[i].WorldPosition.Z)); //up
			posArray.Add(JsonValue);
			JsonValue = MakeShareable(new FJsonValueNumber(Fixations[i].WorldPosition.Y));  //forward
			posArray.Add(JsonValue);

			fixObj->SetArrayField("p", posArray);
		}
		TSharedPtr<FJsonValueObject> snapshotValue;
		snapshotValue = MakeShareable(new FJsonValueObject(fixObj));
		dataArray.Add(snapshotValue);
	}

	wholeObj->SetArrayField("data", dataArray);


	FString OutputString;
	auto Writer = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&OutputString);
	FJsonSerializer::Serialize(wholeObj.ToSharedRef(), Writer);

	if (OutputString.Len() > 0)
	{
		cog->network->NetworkCall("fixations", OutputString, copyDataToCache);
	}
	Fixations.Empty();
	LastSendTime = UCognitiveVRBlueprints::GetSessionDuration();
}

void UFixationDataRecorder::PreSessionEnd()
{
	auto world = ACognitiveVRActor::GetCognitiveSessionWorld();
	if (world == nullptr) { return; }
	world->GetTimerManager().ClearTimer(AutoSendHandle);
}

void UFixationDataRecorder::PostSessionEnd()
{
	cog->OnRequestSend.RemoveDynamic(this, &UFixationDataRecorder::SendData);
	cog->OnPreSessionEnd.RemoveDynamic(this, &UFixationDataRecorder::PreSessionEnd);
	cog->OnSessionBegin.RemoveDynamic(this, &UFixationDataRecorder::StartSession);
	cog->OnPostSessionEnd.RemoveDynamic(this, &UFixationDataRecorder::PostSessionEnd);
	cog.Reset();
}