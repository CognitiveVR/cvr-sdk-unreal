// Fill out your copyright notice in the Description page of Project Settings.

//#include "CognitiveVR.h"
#include "PlayerTracker.h"
//#include "CognitiveVRSettings.h"
//#include "Util.h"

UPlayerTracker* UPlayerTracker::instance;

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
	instance = this;

	UWorld* world = GetWorld();
	if (world == NULL) { GLog->Log("get world from player tracker is null!"); return; }

	if (world->WorldType != EWorldType::PIE && world->WorldType != EWorldType::Game) { return; } //editor world. skip

	cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	if (cog.IsValid())
	{
		cog->SetWorld(world);
		Super::BeginPlay();
		GEngine->GetAllLocalPlayerControllers(controllers);
#if defined HPGLIA_API
		cog->GetWorld()->GetGameInstance()->GetTimerManager().SetTimer(AutoSendHandle, this, &UPlayerTracker::TickSensors1000MS, 1, true);
		cog->GetWorld()->GetGameInstance()->GetTimerManager().SetTimer(AutoSendHandle, this, &UPlayerTracker::TickSensors100MS, 0.1, true);
#endif
	}
	else
	{
		GLog->Log("UPlayerTracker::BeginPlay cannot find CognitiveVRProvider!");
	}

	if (!PauseHandle.IsValid())
	{
		PauseHandle = FCoreDelegates::ApplicationWillEnterBackgroundDelegate.AddUObject(this, &UPlayerTracker::HandleApplicationWillEnterBackground);
	}
}

void UPlayerTracker::HandleApplicationWillEnterBackground()
{
	cog->localCache->SerializeToFile();
}

FVector UPlayerTracker::GetWorldGazeEnd(FVector start)
{
#if defined TOBII_EYETRACKING_ACTIVE
	auto eyetracker = ITobiiCore::GetEyeTracker();
	FVector End = start + eyetracker->GetCombinedGazeData().WorldGazeDirection * 100000.0f;
	return End;
#elif defined SRANIPAL_1_2_API
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
#elif defined SRANIPAL_1_3_API
	FVector End = FVector::ZeroVector;
	FVector TempStart = FVector::ZeroVector;
	FVector LocalDirection = FVector::ZeroVector;
	if (SRanipalEye_Core::Instance()->GetGazeRay(GazeIndex::COMBINE, TempStart, LocalDirection))
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
#elif defined PICOMOBILE_API
	FVector Start = FVector::ZeroVector;
	FVector WorldDirection = FVector::ZeroVector;
	FVector End = FVector::ZeroVector;
	
	if (UPicoBlueprintFunctionLibrary::PicoGetEyeTrackingGazeRay(Start, WorldDirection))
	{
		End = Start + WorldDirection * 10000.0f;
	}
	return End;
#elif defined HPGLIA_API
	FVector End = FVector::ZeroVector;
	FVector TempStart = controllers[0]->PlayerCameraManager->GetCameraLocation();
	FVector LocalDirection = FVector::ZeroVector;

	FEyeTracking eyeTrackingData;
	if (UHPGliaClient::GetEyeTracking(eyeTrackingData))
	{
		if (eyeTrackingData.CombinedGazeConfidence > 0.4f)
		{
			FVector dir = FVector(eyeTrackingData.CombinedGaze.X, eyeTrackingData.CombinedGaze.Y, eyeTrackingData.CombinedGaze.Z);
			LastDirection = controllers[0]->PlayerCameraManager->GetActorTransform().TransformVectorNoScale(dir);
			End = TempStart + LastDirection * 100000.0f;
			return End;
		}
	}
	End = TempStart + LastDirection * 100000.0f;
	return End;
#else
	FRotator captureRotation = controllers[0]->PlayerCameraManager->GetCameraRotation();
	FVector End = start + captureRotation.Vector() * 10000.0f;
	return End;
#endif
}

void UPlayerTracker::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	if (!cog.IsValid()) { return; }
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

	
	double time = Util::GetTimestamp();
	FString objectid = "";

	if (controllers.Num() == 0)
	{
		CognitiveLog::Info("UPlayerTracker::TickComponent--------------------------no controllers. skip");
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
				if (hitDynamicObject != NULL && hitDynamicObject->GetObjectId().IsValid())
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
		BuildSnapshot(captureLocation, gaze, captureRotation, time, DidHitFloor, FloorHitPosition, objectid);

		if (DebugDisplayGaze)
			DrawDebugSphere(GetWorld(), gaze, 3, 3, FColor::White, false, 0.2);
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
	JsonValue = MakeShareable(new FJsonValueNumber(-position.X)); //right
	posArray.Add(JsonValue);
	JsonValue = MakeShareable(new FJsonValueNumber(position.Z)); //up
	posArray.Add(JsonValue);
	JsonValue = MakeShareable(new FJsonValueNumber(position.Y));  //forward
	posArray.Add(JsonValue);

	snapObj->SetArrayField("p", posArray);

	if (objectId != "")
	{
		snapObj->SetStringField("o", objectId);
	}

	TArray<TSharedPtr<FJsonValue>> gazeArray;
	JsonValue = MakeShareable(new FJsonValueNumber(-gaze.X));
	gazeArray.Add(JsonValue);
	JsonValue = MakeShareable(new FJsonValueNumber(gaze.Z));
	gazeArray.Add(JsonValue);
	JsonValue = MakeShareable(new FJsonValueNumber(gaze.Y));
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
		JsonValue = MakeShareable(new FJsonValueNumber(-floorHitPos.X)); //right
		floorArray.Add(JsonValue);
		JsonValue = MakeShareable(new FJsonValueNumber(floorHitPos.Z)); //up
		floorArray.Add(JsonValue);
		JsonValue = MakeShareable(new FJsonValueNumber(floorHitPos.Y));  //forward
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
	JsonValue = MakeShareable(new FJsonValueNumber(-position.X)); //right
	posArray.Add(JsonValue);
	JsonValue = MakeShareable(new FJsonValueNumber(position.Z)); //up
	posArray.Add(JsonValue);
	JsonValue = MakeShareable(new FJsonValueNumber(position.Y));  //forward
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
		JsonValue = MakeShareable(new FJsonValueNumber(-floorHitPos.X)); //right
		floorArray.Add(JsonValue);
		JsonValue = MakeShareable(new FJsonValueNumber(floorHitPos.Z)); //up
		floorArray.Add(JsonValue);
		JsonValue = MakeShareable(new FJsonValueNumber(floorHitPos.Y));  //forward
		floorArray.Add(JsonValue);

		snapObj->SetArrayField("f", floorArray);
	}

	snapshots.Add(snapObj);
	if (snapshots.Num() > GazeBatchSize)
	{
		SendData();
	}
}

void UPlayerTracker::SendData(bool copyDataToCache)
{
	if (!cog.IsValid() || !cog->HasStartedSession()) { return; }
	if (cog->GetCurrentSceneVersionNumber().Len() == 0) { return; }
	if (!cog->GetCurrentSceneData().IsValid()){return;}

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
	wholeObj->SetNumberField("part", jsonGazePart);
	wholeObj->SetStringField("formatversion", "1.0");
	jsonGazePart++;

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
		TSharedPtr<FJsonValueObject> snapshotValue;
		snapshotValue = MakeShareable(new FJsonValueObject(snapshots[i]));
		dataArray.Add(snapshotValue);
	}

	wholeObj->SetNumberField("interval", PlayerSnapshotInterval);

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

#if defined HPGLIA_API
void UPlayerTracker::TickSensors1000MS()
{
	if (cog->HasStartedSession() == false) { return; }
	int32 OutHeartRate = 0;
	if (UHPGliaClient::GetHeartRate(OutHeartRate) && OutHeartRate != LastHeartRate)
	{
		cog->sensors->RecordSensor("HP.HeartRate", (float)OutHeartRate);
		LastHeartRate = OutHeartRate;
	}

	FCognitiveLoad CognitiveLoad;
	if (UHPGliaClient::GetCognitiveLoad(CognitiveLoad))
	{
		if (!FMath::IsNearlyEqual(CognitiveLoad.CognitiveLoad, LastCognitiveLoad))
		{
			cog->sensors->RecordSensor("HP.CognitiveLoad", CognitiveLoad.CognitiveLoad);
			cog->sensors->RecordSensor("HP.CognitiveLoad.Confidence", CognitiveLoad.StandardDeviation);
			LastCognitiveLoad = CognitiveLoad.CognitiveLoad;
		}
	}
}

void UPlayerTracker::TickSensors100MS()
{
	if (cog->HasStartedSession() == false) { return; }
	FEyeTracking data;
	if (UHPGliaClient::GetEyeTracking(data))
	{
		if (data.LeftPupilDilationConfidence > 0.5 && data.LeftPupilDilation > 1.5f && !FMath::IsNearlyEqual(data.LeftPupilDilation, LastLeftPupilDiamter))
		{
			cog->sensors->RecordSensor("HP.Left Pupil Diameter", data.LeftPupilDilation);
			LastLeftPupilDiamter = data.LeftPupilDilation;
		}
		if (data.RightPupilDilationConfidence > 0.5 && data.RightPupilDilation > 1.5f && !FMath::IsNearlyEqual(data.RightPupilDilation, LastRightPupilDiamter))
		{
			cog->sensors->RecordSensor("HP.Right Pupil Diameter", data.RightPupilDilation);
			LastRightPupilDiamter = data.RightPupilDilation;
		}
	}
}
#endif

void UPlayerTracker::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (!cog.IsValid())
	{
		GLog->Log("Cognitive3D UPlayerTracker::EndPlay exiting editor");
		return;
	}

	Super::EndPlay(EndPlayReason);

	FString reason;
	bool shouldEndSession = true;
	switch (EndPlayReason)
	{
	case EEndPlayReason::Destroyed: reason = "destroyed";
		//this should normally never be destroyed. 4.19 bug - this is called instead of level transition
		cog->FlushEvents();
		//shouldEndSession = false;
		break;
	case EEndPlayReason::EndPlayInEditor: reason = "end PIE";
		break;
	case EEndPlayReason::LevelTransition: reason = "level transition";
		//this is called correctly in 4.24. possibly earlier versions
		cog->FlushEvents();
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
			//Q: this why is endplay on gaze recorder not on core? A: core isn't an actor and doesn't have EndPlay to call
			cog->EndSession();
		}
		cog.Reset();
	}
	instance = NULL;
}

UPlayerTracker* UPlayerTracker::GetPlayerTracker()
{
	if (instance == NULL)
	{
		for (TObjectIterator<UPlayerTracker> Itr; Itr; ++Itr)
		{
			UWorld* tempWorld = Itr->GetWorld();
			if (tempWorld == NULL) { continue; }
			if (tempWorld->WorldType != EWorldType::PIE && tempWorld->WorldType != EWorldType::Game) { continue; } //editor world. skip
			instance = *Itr;
			break;
		}
	}

	return instance;
}