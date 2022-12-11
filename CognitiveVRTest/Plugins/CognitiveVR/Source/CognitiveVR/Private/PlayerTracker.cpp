// Fill out your copyright notice in the Description page of Project Settings.

//#include "CognitiveVR.h"
#include "PlayerTracker.h"
//#include "CognitiveVRSettings.h"
//#include "Util.h"


int32 UPlayerTracker::jsonGazePart = 1;

UPlayerTracker::UPlayerTracker()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UPlayerTracker::BeginPlay()
{
	//if (HasBegunPlay()) { return; }
	Super::BeginPlay();

	cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();

	auto cognitiveActor = ACognitiveVRActor::GetCognitiveVRActor();
	if (cognitiveActor != GetOwner())
	{
		UnregisterComponent();
		return;
	}

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

	if (!cog.IsValid())
	{
		CognitiveLog::Error("UPlayerTracker::BeginPlay has invalid cognitive provider. should be impossible??");
		return;
	}

	GEngine->GetAllLocalPlayerControllers(controllers);
	cog->OnRequestSend.AddDynamic(this, &UPlayerTracker::SendData);
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
#elif defined OPENXR_EYETRACKING

	FRotator captureRotation = controllers[0]->PlayerCameraManager->GetCameraRotation();
	FVector End = start + captureRotation.Vector() * 10000.0f;
	IEyeTracker const* const ET = GEngine ? GEngine->EyeTrackingDevice.Get() : nullptr;
	if (ET)
	{
		FEyeTrackerGazeData gazeData;
		ET->GetEyeTrackerGazeData(gazeData);
#if defined OPENXR_LOCALSPACE
		LastDirection = controllers[0]->PlayerCameraManager->GetActorTransform().TransformVectorNoScale(gazeData.GazeDirection);
#endif
		LastDirection = gazeData.GazeDirection;
		End = start + LastDirection * 10000.0f;
	}
	return End;
#elif defined WAVEVR_EYETRACKING
	WaveVREyeManager* pEyeManager = WaveVREyeManager::GetInstance();
	FVector End;
	if (pEyeManager != nullptr)
	{
		//is this world direction or local direction?
		if (pEyeManager->GetCombindedEyeDirectionNormalized(LastDirection))
		{
			LastDirection = controllers[0]->PlayerCameraManager->GetActorTransform().TransformVectorNoScale(LastDirection);
		}
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
	if (FloorHit.GetActor() != NULL)
	{
		DidHitFloor = true;
		FloorHitPosition = FloorHit.ImpactPoint;
	}

	bool hitDynamic = false;
	if (bHit)
	{
		FVector gaze = Hit.ImpactPoint;

		if (Hit.GetActor() != NULL)
		{
			UActorComponent* hitActorComponent = Hit.GetActor()->GetComponentByClass(UDynamicObject::StaticClass());
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
		SendData(false);
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
		SendData(false);
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

void UPlayerTracker::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	cog->OnRequestSend.RemoveDynamic(this, &UPlayerTracker::SendData);
}