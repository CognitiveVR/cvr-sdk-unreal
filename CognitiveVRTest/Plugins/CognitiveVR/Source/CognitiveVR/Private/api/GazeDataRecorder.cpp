/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#include "CognitiveVR/Private/api/GazeDataRecorder.h"

//called at module startup to create a default uobject of this type
UGazeDataRecorder::UGazeDataRecorder()
{
	cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
}

void UGazeDataRecorder::StartSession()
{
	snapshots.Empty();
	jsonPart = 1;

	FString ValueReceived;

	//gaze batch size
	ValueReceived = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "GazeBatchSize", false);
	if (ValueReceived.Len() > 0)
	{
		int32 sensorLimit = FCString::Atoi(*ValueReceived);
		if (sensorLimit > 0)
		{
			GazeBatchSize = sensorLimit;
		}
	}

	auto world = ACognitiveVRActor::GetCognitiveSessionWorld();
	if (world == nullptr)
	{
		GLog->Log("UGazeDataRecorder::StartSession world from ACognitiveVRActor is null!");
		return;
	}
	//world->GetTimerManager().SetTimer(AutoSendHandle, FTimerDelegate::CreateRaw(this, &UGazeDataRecorder::SendData, false), AutoTimer, true);
}

void UGazeDataRecorder::BuildSnapshot(FVector position, FVector gaze, FRotator rotation, double timestamp, bool didHitFloor, FVector floorHitPos, FString objectId)
{
	FGazeData data;
	data.Time = timestamp;
	data.UseFloor = didHitFloor;
	data.FloorPosition = floorHitPos;
	if (!objectId.IsEmpty())
	{
		data.DynamicObjectId = objectId;
		data.UseDynamicId = true;
	}
	data.HMDPosition = position;
	data.HMDRotation = rotation;
	data.GazePosition = gaze;
	data.UseGaze = true;

	snapshots.Add(data);
	if (snapshots.Num() > GazeBatchSize)
	{
		SendData(false);
	}
}

void UGazeDataRecorder::BuildSnapshot(FVector position, FRotator rotation, double timestamp, bool didHitFloor, FVector floorHitPos)
{
	FGazeData data;
	data.Time = timestamp;
	data.UseFloor = didHitFloor;
	data.FloorPosition = floorHitPos;
	data.UseDynamicId = false;
	data.HMDPosition = position;
	data.HMDRotation = rotation;
	data.UseGaze = false;

	snapshots.Add(data);
	if (snapshots.Num() > GazeBatchSize)
	{
		SendData(false);
	}
}

void UGazeDataRecorder::SendData(bool copyDataToCache)
{
	if (!cog.IsValid() || !cog->HasStartedSession()) { return; }
	if (cog->GetCurrentSceneVersionNumber().Len() == 0) { return; }
	if (!cog->GetCurrentSceneData().IsValid()) { return; }

	//GAZE
	FJsonObject newProps = cog->GetNewSessionProperties();

	if (snapshots.Num() == 0 && newProps.Values.Num() == 0 && cog->ForceWriteSessionMetadata == false) { return; }

	TSharedPtr<FJsonObject>wholeObj = MakeShareable(new FJsonObject);
	TArray<TSharedPtr<FJsonValue>> dataArray;

	wholeObj->SetStringField("userid", cog->GetUserID());
	if (!cog->LobbyId.IsEmpty())
	{
		wholeObj->SetStringField("lobbyId", cog->LobbyId);
	}
	wholeObj->SetNumberField("timestamp", cog->GetSessionTimestamp());
	wholeObj->SetStringField("sessionid", cog->GetSessionID());
	wholeObj->SetNumberField("part", jsonPart);
	wholeObj->SetStringField("formatversion", "1.0");
	jsonPart++;

	FName DeviceName(NAME_None);
	FString DeviceNameString = "unknown";

	//get HMDdevice name on beginplay and cache
	if (GEngine->XRSystem.IsValid())
	{
		DeviceName = GEngine->XRSystem->GetSystemName();
		DeviceNameString = Util::GetDeviceName(DeviceName.ToString());
	}

	wholeObj->SetStringField("formatversion", "1.0");
	wholeObj->SetStringField("hmdtype", DeviceNameString);

	for (int32 i = 0; i != snapshots.Num(); ++i)
	{
		TSharedPtr<FJsonObject>snapObj = MakeShareable(new FJsonObject);

		snapObj->SetNumberField("time", snapshots[i].Time);

		//positions
		TArray<TSharedPtr<FJsonValue>> posArray;
		TSharedPtr<FJsonValueNumber> JsonValue;
		JsonValue = MakeShareable(new FJsonValueNumber(-snapshots[i].HMDPosition.X)); //right
		posArray.Add(JsonValue);
		JsonValue = MakeShareable(new FJsonValueNumber(snapshots[i].HMDPosition.Z)); //up
		posArray.Add(JsonValue);
		JsonValue = MakeShareable(new FJsonValueNumber(snapshots[i].HMDPosition.Y));  //forward
		posArray.Add(JsonValue);

		snapObj->SetArrayField("p", posArray);

		if (snapshots[i].UseDynamicId)
		{
			snapObj->SetStringField("o", snapshots[i].DynamicObjectId);
		}

		if (snapshots[i].UseGaze)
		{
			TArray<TSharedPtr<FJsonValue>> gazeArray;
			JsonValue = MakeShareable(new FJsonValueNumber(-snapshots[i].GazePosition.X));
			gazeArray.Add(JsonValue);
			JsonValue = MakeShareable(new FJsonValueNumber(snapshots[i].GazePosition.Z));
			gazeArray.Add(JsonValue);
			JsonValue = MakeShareable(new FJsonValueNumber(snapshots[i].GazePosition.Y));
			gazeArray.Add(JsonValue);
			snapObj->SetArrayField("g", gazeArray);
		}

		//rotation
		TArray<TSharedPtr<FJsonValue>> rotArray;

		FQuat quat;
		FRotator adjustedRot = snapshots[i].HMDRotation;
		adjustedRot.Yaw -= 90;
		quat = adjustedRot.Quaternion();

		JsonValue = MakeShareable(new FJsonValueNumber(quat.Y));
		rotArray.Add(JsonValue);
		JsonValue = MakeShareable(new FJsonValueNumber(quat.Z));
		rotArray.Add(JsonValue);
		JsonValue = MakeShareable(new FJsonValueNumber(quat.X));
		rotArray.Add(JsonValue);
		JsonValue = MakeShareable(new FJsonValueNumber(quat.W));
		rotArray.Add(JsonValue);

		snapObj->SetArrayField("r", rotArray);

		if (snapshots[i].UseFloor)
		{
			//floor position
			TArray<TSharedPtr<FJsonValue>> floorArray;
			JsonValue = MakeShareable(new FJsonValueNumber(-snapshots[i].FloorPosition.X)); //right
			floorArray.Add(JsonValue);
			JsonValue = MakeShareable(new FJsonValueNumber(snapshots[i].FloorPosition.Z)); //up
			floorArray.Add(JsonValue);
			JsonValue = MakeShareable(new FJsonValueNumber(snapshots[i].FloorPosition.Y));  //forward
			floorArray.Add(JsonValue);
			snapObj->SetArrayField("f", floorArray);
		}

		TSharedPtr<FJsonValueObject> snapshotValue;
		snapshotValue = MakeShareable(new FJsonValueObject(snapObj));
		dataArray.Add(snapshotValue);
	}

	//TODO move the PlayerSnapshotInterval const somewhere actually accessible
	wholeObj->SetNumberField("interval", 0.1f);

	wholeObj->SetArrayField("data", dataArray);

	if (cog->ForceWriteSessionMetadata)
	{
		cog->ForceWriteSessionMetadata = false;
		FJsonObject allProps = cog->GetAllSessionProperties();
		if (allProps.Values.Num() > 0)
		{
			TSharedPtr<FJsonObject> sessionValue;
			sessionValue = MakeShareable(new FJsonObject(allProps));
			wholeObj->SetObjectField("properties", sessionValue);
		}
	}
	else if (newProps.Values.Num() > 0)
	{
		TSharedPtr<FJsonObject> sessionValue;
		sessionValue = MakeShareable(new FJsonObject(newProps));

		wholeObj->SetObjectField("properties", sessionValue);
	}

	FString OutputString;
	auto Writer = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&OutputString);
	FJsonSerializer::Serialize(wholeObj.ToSharedRef(), Writer);

	if (OutputString.Len() > 0)
	{
		cog->network->NetworkCall("gaze", OutputString, copyDataToCache);
	}
	snapshots.Empty();
	LastSendTime = UCognitiveVRBlueprints::GetSessionDuration();
}

void UGazeDataRecorder::PreSessionEnd()
{
	//auto world = ACognitiveVRActor::GetCognitiveSessionWorld();
	//if (world == nullptr) { return; }
	//world->GetTimerManager().ClearTimer(AutoSendHandle);
}

void UGazeDataRecorder::PostSessionEnd()
{
	
}