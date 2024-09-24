/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/
#include "Cognitive3D/Private/C3DApi/FixationDataRecorder.h"
#include "Cognitive3D/Public/Cognitive3DBlueprints.h"
#include "Cognitive3D/Private/C3DNetwork/Network.h"

FFixationDataRecorder::FFixationDataRecorder()
{
	cog = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
}

void FFixationDataRecorder::StartSession()
{
	jsonPart = 1;
	Fixations.Empty();

	FString ValueReceived;

	ValueReceived = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/Cognitive3D.Cognitive3DSettings", "FixationBatchSize", false);
	if (ValueReceived.Len() > 0)
	{
		int32 fixationLimit = FCString::Atoi(*ValueReceived);
		if (fixationLimit > 0)
		{
			FixationBatchSize = fixationLimit;
		}
	}

	ValueReceived = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/Cognitive3D.Cognitive3DSettings", "FixationAutoTimer", false);
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
		GLog->Log("UFixationDataRecorder::StartSession world from ACognitive3DActor is null!");
		return;
	}
	world->GetTimerManager().SetTimer(AutoSendHandle, FTimerDelegate::CreateRaw(this, &FFixationDataRecorder::SendData, false), AutoTimer, true);
}

void FFixationDataRecorder::RecordFixationEnd(const FFixation& fixation)
{
	Fixations.Add(fixation);
	if (Fixations.Num() > FixationBatchSize)
	{
		SendData(false);
	}
}

void FFixationDataRecorder::SendData(bool copyDataToCache)
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
	LastSendTime = UCognitive3DBlueprints::GetSessionDuration();
}

void FFixationDataRecorder::PreSessionEnd()
{
	auto world = ACognitive3DActor::GetCognitiveSessionWorld();
	if (world == nullptr) { return; }
	world->GetTimerManager().ClearTimer(AutoSendHandle);
}

void FFixationDataRecorder::PostSessionEnd()
{
	
}