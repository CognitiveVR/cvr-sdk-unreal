// Fill out your copyright notice in the Description page of Project Settings.

#include "CognitiveVR.h"
#include "PlayerTracker.h"
//#include "CognitiveVRSettings.h"
#include "Util.h"

// Sets default values for this component's properties
UPlayerTracker::UPlayerTracker()
{
	PrimaryComponentTick.bCanEverTick = true;

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
}

void UPlayerTracker::BeginPlay()
{
	if (HasBegunPlay()) { return; }
	UWorld* world = GetWorld();
	if (GetWorld() == NULL) { GLog->Log("get world from player tracker is null!"); return; } //somehow world is null from playertracker

	if (world->WorldType != EWorldType::PIE && world->WorldType != EWorldType::Game) { return; } //editor world. skip

	cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider();
	if (cog.IsValid())
	{
		cog->SetWorld(world);
		Super::BeginPlay();
		GEngine->GetAllLocalPlayerControllers(controllers);
	}
	else
	{
		GLog->Log("UPlayerTracker::BeginPlay cannot find CognitiveVRProvider!");
	}
}

FVector UPlayerTracker::GetWorldGazeEnd(FVector start)
{
#if defined TOBII_EYETRACKING_ACTIVE
	auto eyetracker = ITobiiCore::GetEyeTracker();
	FVector End = start + eyetracker->GetCombinedGazeData().WorldGazeDirection * 100000.0f;
	return End;
#elif defined SRANIPAL_API
	FVector End = FVector::ZeroVector;
	FVector TempStart = FVector::ZeroVector;
	FVector LocalDirection = FVector::ZeroVector;

	if (USRanipal_FunctionLibrary_Eye::GetGazeRay(GazeIndex::COMBINE, TempStart, LocalDirection))
	{
		FVector WorldDir = controllers[0]->PlayerCameraManager->GetActorTransform().TransformVectorNoScale(LocalDirection);
		End = start + WorldDir * 100000.0f;
		LastDirection = WorldDir;
		return End;
	}
	End = start + LastDirection * 100000.0f;
	return End;
#elif defined VARJOEYETRACKER_API
	FVector Start = start;
	FVector WorldDirection = FVector::ZeroVector;
	FVector End = FVector::ZeroVector;
	float ignored = 0;

	FVarjoEyeTrackingData data;

	if (UVarjoEyeTrackerFunctionLibrary::GetEyeTrackerGazeData(data)) //if the data is valid
	{
		//the gaze transformed into world space
		UVarjoEyeTrackerFunctionLibrary::GetGazeRay(Start, WorldDirection, ignored);

		End = start + WorldDirection * 10000.0f;
		LastDirection = WorldDirection;
		return End;
	}
	End = start + LastDirection * 100000.0f;
	return End;
#else
	FRotator captureRotation = controllers[0]->PlayerCameraManager->GetCameraRotation();
	FVector End = start + captureRotation.Vector() * 10000.0f;
	return End;
#endif
}

void UPlayerTracker::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	if (!cog->HasStartedSession())
	{
		//don't record player position data before a session has begun
		return;
	}

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	currentTime += DeltaTime;
	if (currentTime < PlayerSnapshotInterval)
	{
		return;
	}

	currentTime -= PlayerSnapshotInterval;

	
	double time = cognitivevrapi::Util::GetTimestamp();
	FString objectid = "";

	if (controllers.Num() == 0)
	{
		cognitivevrapi::CognitiveLog::Info("UPlayerTracker::TickComponent--------------------------no controllers. skip");
		return;
	}

	FVector captureLocation = controllers[0]->PlayerCameraManager->GetCameraLocation();
	FRotator captureRotation = controllers[0]->PlayerCameraManager->GetCameraRotation();

	//look at dynamic object
	FCollisionQueryParams Params; // You can use this to customize various properties about the trace
	Params.AddIgnoredActor(GetOwner()); // Ignore the player's pawn


	FHitResult Hit; // The hit result gets populated by the line trace
	FHitResult FloorHit; // The hit result gets populated by the line trace

	FVector Start = captureLocation;
	FVector End = GetWorldGazeEnd(Start);
	
	FCollisionObjectQueryParams params = FCollisionObjectQueryParams();
	params.AddObjectTypesToQuery(ECC_WorldStatic);
	params.AddObjectTypesToQuery(ECC_WorldDynamic);

	bool bHit = false;
	FCollisionQueryParams gazeparams = FCollisionQueryParams(FName(), true);
	bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECollisionChannel::ECC_Visibility, gazeparams);

	GetWorld()->LineTraceSingleByObjectType(FloorHit, captureLocation, FVector(0, 0, -1000), params);
	
	bool DidHitFloor = false;
	FVector FloorHitPosition;
	if (FloorHit.Actor.IsValid())
	{
		DidHitFloor = true;
		FloorHitPosition = FloorHit.ImpactPoint;
	}

	bool hitDynamic = false;
	if (bHit)
	{
		FVector gaze = Hit.ImpactPoint;

		if (Hit.Actor.IsValid())
		{
			UActorComponent* hitActorComponent = Hit.Actor.Get()->GetComponentByClass(UDynamicObject::StaticClass());
			if (hitActorComponent != NULL)
			{
				UDynamicObject* hitDynamicObject = Cast<UDynamicObject>(hitActorComponent);
				if (hitDynamicObject != NULL && hitDynamicObject->TrackGaze && hitDynamicObject->GetObjectId().IsValid())
				{
					hitDynamic = true;

					FVector localHitPosition = hitDynamicObject->GetOwner()->GetActorTransform().InverseTransformPosition(Hit.ImpactPoint);

					//localHitPosition *= hitDynamicObject->GetOwner()->GetActorTransform().GetScale3D();

					objectid = hitDynamicObject->GetObjectId()->Id;
					gaze.X = localHitPosition.X;
					gaze.Y = localHitPosition.Y;
					gaze.Z = localHitPosition.Z;
				}
			}
			else
			{
				//hit an actor that is not a dynamic object
			}
		}
		else
		{
			//hit some csg or something that is not an actor
		}
		//DrawDebugSphere(GetWorld(), gaze, 3, 3, FColor::Cyan, false, 0.2);
		BuildSnapshot(captureLocation, gaze, captureRotation, time, DidHitFloor, FloorHitPosition, objectid);
	}
	else
	{
		//hit nothing. use position and rotation only
		BuildSnapshot(captureLocation, captureRotation, time, DidHitFloor, FloorHitPosition);
	}
}

void UPlayerTracker::BuildSnapshot(FVector position, FVector gaze, FRotator rotation, double time, bool didHitFloor, FVector floorHitPos, FString objectId)
{
	TSharedPtr<FJsonObject>snapObj = MakeShareable(new FJsonObject);

	snapObj->SetNumberField("time", time);

	//positions
	TArray<TSharedPtr<FJsonValue>> posArray;
	TSharedPtr<FJsonValueNumber> JsonValue;
	JsonValue = MakeShareable(new FJsonValueNumber(-(int32)position.X)); //right
	posArray.Add(JsonValue);
	JsonValue = MakeShareable(new FJsonValueNumber((int32)position.Z)); //up
	posArray.Add(JsonValue);
	JsonValue = MakeShareable(new FJsonValueNumber((int32)position.Y));  //forward
	posArray.Add(JsonValue);

	snapObj->SetArrayField("p", posArray);

	if (objectId != "")
	{
		snapObj->SetStringField("o", objectId);
	}

	TArray<TSharedPtr<FJsonValue>> gazeArray;
	JsonValue = MakeShareable(new FJsonValueNumber(-(int32)gaze.X));
	gazeArray.Add(JsonValue);
	JsonValue = MakeShareable(new FJsonValueNumber((int32)gaze.Z));
	gazeArray.Add(JsonValue);
	JsonValue = MakeShareable(new FJsonValueNumber((int32)gaze.Y));
	gazeArray.Add(JsonValue);

	snapObj->SetArrayField("g", gazeArray);

	//rotation
	TArray<TSharedPtr<FJsonValue>> rotArray;

	FQuat quat;
	FRotator adjustedRot = rotation;
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

	if (didHitFloor)
	{
		//floor position
		TArray<TSharedPtr<FJsonValue>> floorArray;
		JsonValue = MakeShareable(new FJsonValueNumber(-(int32)floorHitPos.X)); //right
		floorArray.Add(JsonValue);
		JsonValue = MakeShareable(new FJsonValueNumber((int32)floorHitPos.Z)); //up
		floorArray.Add(JsonValue);
		JsonValue = MakeShareable(new FJsonValueNumber((int32)floorHitPos.Y));  //forward
		floorArray.Add(JsonValue);

		snapObj->SetArrayField("f", floorArray);
	}

	snapshots.Add(snapObj);
	if (snapshots.Num() > GazeBatchSize)
	{
		SendData();
	}
}

void UPlayerTracker::BuildSnapshot(FVector position, FRotator rotation, double time, bool didHitFloor, FVector floorHitPos)
{
	TSharedPtr<FJsonObject>snapObj = MakeShareable(new FJsonObject);

	snapObj->SetNumberField("time", time);

	//positions
	TArray<TSharedPtr<FJsonValue>> posArray;
	TSharedPtr<FJsonValueNumber> JsonValue;
	JsonValue = MakeShareable(new FJsonValueNumber(-(int32)position.X)); //right
	posArray.Add(JsonValue);
	JsonValue = MakeShareable(new FJsonValueNumber((int32)position.Z)); //up
	posArray.Add(JsonValue);
	JsonValue = MakeShareable(new FJsonValueNumber((int32)position.Y));  //forward
	posArray.Add(JsonValue);

	snapObj->SetArrayField("p", posArray);

	//rotation
	TArray<TSharedPtr<FJsonValue>> rotArray;

	FQuat quat;
	FRotator adjustedRot = rotation;
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

	if (didHitFloor)
	{
		//floor position
		TArray<TSharedPtr<FJsonValue>> floorArray;
		JsonValue = MakeShareable(new FJsonValueNumber(-(int32)floorHitPos.X)); //right
		floorArray.Add(JsonValue);
		JsonValue = MakeShareable(new FJsonValueNumber((int32)floorHitPos.Z)); //up
		floorArray.Add(JsonValue);
		JsonValue = MakeShareable(new FJsonValueNumber((int32)floorHitPos.Y));  //forward
		floorArray.Add(JsonValue);

		snapObj->SetArrayField("f", floorArray);
	}

	snapshots.Add(snapObj);
	if (snapshots.Num() > GazeBatchSize)
	{
		SendData();
	}
}

void UPlayerTracker::SendData()
{
	if (!cog.IsValid() || !cog->HasStartedSession()) { return; }
	if (cog->GetCurrentSceneVersionNumber().Len() == 0) { return; }
	if (!cog->GetCurrentSceneData().IsValid()){return;}

	//GAZE
	FJsonObject props = cog->GetSessionProperties();

	if (snapshots.Num() == 0 && props.Values.Num() == 0) { return; }

	TSharedPtr<FJsonObject>wholeObj = MakeShareable(new FJsonObject);
	TArray<TSharedPtr<FJsonValue>> dataArray;

	wholeObj->SetStringField("userid", cog->GetUserID());
	if (!cog->LobbyId.IsEmpty())
	{
		wholeObj->SetStringField("lobbyId", cog->LobbyId);
	}
	wholeObj->SetNumberField("timestamp", (int32)cog->GetSessionTimestamp());
	wholeObj->SetStringField("sessionid", cog->GetSessionID());
	wholeObj->SetNumberField("part", jsonGazePart);
	wholeObj->SetStringField("formatversion", "1.0");
	jsonGazePart++;

	FName DeviceName(NAME_None);
	FString DeviceNameString = "unknown";

	//TODO get HMDdevice name on beginplay and cache
	if (GEngine->XRSystem.IsValid())
	{
		DeviceName = GEngine->XRSystem->GetSystemName();
		DeviceNameString = cognitivevrapi::Util::GetDeviceName(DeviceName.ToString());
	}

	wholeObj->SetStringField("formatversion", "1.0");
	wholeObj->SetStringField("hmdtype", DeviceNameString);

	for (int32 i = 0; i != snapshots.Num(); ++i)
	{
		TSharedPtr<FJsonValueObject> snapshotValue;
		snapshotValue = MakeShareable(new FJsonValueObject(snapshots[i]));
		dataArray.Add(snapshotValue);
	}

	wholeObj->SetNumberField("interval", PlayerSnapshotInterval);

	wholeObj->SetArrayField("data", dataArray);

	if (props.Values.Num() > 0)
	{
		TSharedPtr<FJsonObject> sessionValue;
		sessionValue = MakeShareable(new FJsonObject(props));

		wholeObj->SetObjectField("properties", sessionValue);
	}

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(wholeObj.ToSharedRef(), Writer);

	if (OutputString.Len() > 0)
	{
		cog->network->NetworkCall("gaze", OutputString);
	}
	snapshots.Empty();
}


void UPlayerTracker::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	FString reason;
	bool shouldEndSession = true;
	switch (EndPlayReason)
	{
	case EEndPlayReason::Destroyed: reason = "destroyed";
		//this should never be destroyed, only when changing level. pro tips - doesn't actually use the 'level transition' reason for endplay
		shouldEndSession = false;
		break;
	case EEndPlayReason::EndPlayInEditor: reason = "end PIE";
		break;
	case EEndPlayReason::LevelTransition: reason = "level transition";
		shouldEndSession = false;
		break;
	case EEndPlayReason::Quit: reason = "quit";
		break;
	case EEndPlayReason::RemovedFromWorld: reason = "removed from world";
		break;
	default:
		reason = "default";
		break;
	}

	if (cog.IsValid())
	{
		if (shouldEndSession)
		{
			//this will send gaze and event data to scene explorer from THIS playertracker
			SendData();

			//TODO this why is endplay on gaze recorder not on core?
			//cognitivevr manager can't find playercontroller0, but will send events to dash and dynamics+sensors to SE
			cog->EndSession();
		}
	}
	Super::EndPlay(EndPlayReason);
}
