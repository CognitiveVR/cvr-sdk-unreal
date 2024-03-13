/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/

#include "PlayerTracker.h"
#include "HeadMountedDisplayFunctionLibrary.h"

UPlayerTracker::UPlayerTracker()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UPlayerTracker::BeginPlay()
{
	Super::BeginPlay();

	cog = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();

	auto cognitiveActor = ACognitive3DActor::GetCognitive3DActor();
	if (cognitiveActor != GetOwner())
	{
		UnregisterComponent();
		return;
	}

	if (!cog.IsValid())
	{
		return;
	}

	GEngine->GetAllLocalPlayerControllers(controllers);
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
	FVector End = start + captureRotation.Vector() * 100000.0f;
	IEyeTracker const* const ET = GEngine ? GEngine->EyeTrackingDevice.Get() : nullptr;
	if (ET != nullptr)
	{
		FEyeTrackerGazeData gazeData;
		if (ET->GetEyeTrackerGazeData(gazeData))
		{
			LastDirection = gazeData.GazeDirection;
			End = start + LastDirection * 100000.0f;
		}
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
	if (cog->CurrentTrackingSceneId.IsEmpty()) { return; }

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	currentTime += DeltaTime;
	if (currentTime < PlayerSnapshotInterval)
	{
		return;
	}

	currentTime -= PlayerSnapshotInterval;

	
	double timestamp = FUtil::GetTimestamp();
	FString objectid = "";

	if (controllers.Num() == 0)
	{
		FCognitiveLog::Info("UPlayerTracker::TickComponent--------------------------no controllers. skip");
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
					FVector localHitPosition = hitDynamicObject->GetOwner()->GetActorTransform().InverseTransformPosition(Hit.ImpactPoint);

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
		cog->gazeDataRecorder->BuildSnapshot(captureLocation, gaze, captureRotation, timestamp, DidHitFloor, FloorHitPosition, objectid);

		if (DebugDisplayGaze)
			DrawDebugSphere(GetWorld(), gaze, 3, 3, FColor::White, false, 0.2);
	}
	else
	{
		//hit nothing. use position and rotation only
		cog->gazeDataRecorder->BuildSnapshot(captureLocation, captureRotation, timestamp, DidHitFloor, FloorHitPosition);
	}

	cog->OnCognitiveInterval.Broadcast();
}

void UPlayerTracker::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

float UPlayerTracker::GetLastSendTime()
{
	if (!cog.IsValid()) { return 0; }
	if (cog->gazeDataRecorder == nullptr) { return 0; }
	return cog->gazeDataRecorder->GetLastSendTime();
}
int32 UPlayerTracker::GetPartNumber()
{
	if (!cog.IsValid()) { return 0; }
	if (cog->gazeDataRecorder == nullptr) { return 0; }
	return cog->gazeDataRecorder->GetPartNumber();
}
int32 UPlayerTracker::GetDataPoints()
{
	if (!cog.IsValid()) { return 0; }
	if (cog->gazeDataRecorder == nullptr) { return 0; }
	return cog->gazeDataRecorder->GetDataPoints();
}