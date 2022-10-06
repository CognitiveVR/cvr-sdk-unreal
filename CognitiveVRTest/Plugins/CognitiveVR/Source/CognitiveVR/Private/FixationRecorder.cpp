// Fill out your copyright notice in the Description page of Project Settings.

#include "FixationRecorder.h"

UFixationRecorder* UFixationRecorder::instance;

// Sets default values for this component's properties
UFixationRecorder::UFixationRecorder()
{
	PrimaryComponentTick.bCanEverTick = true;

	FString ValueReceived;

	cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();

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
}

int32 UFixationRecorder::GetIndex(int32 offset)
{
	if (index + offset < 0)
		return (CachedEyeCaptureCount + index + offset) % CachedEyeCaptureCount;
	return (index + offset) % CachedEyeCaptureCount;
}

void UFixationRecorder::BeginPlay()
{
	if (HasBegunPlay()) { return; }
	Super::BeginPlay();
}

void UFixationRecorder::BeginSession()
{
	if (cog.IsValid())
	{
		//cog->EnsureGetWorld()->GetGameInstance()->GetTimerManager().SetTimer(AutoSendHandle, this, &UFixationRecorder::SendData, AutoTimer, true);
		for (int32 i = 0; i < CachedEyeCaptureCount; i++)
		{
			EyeCaptures.Add(FEyeCapture());
		}
		GEngine->GetAllLocalPlayerControllers(controllers);
	}
	else
	{
		CognitiveLog::Error("UFixationRecorder::BeginSession cannot find CognitiveVRProvider!");
	}
}

bool UFixationRecorder::IsGazeOutOfRange(FEyeCapture eyeCapture)
{
	if (!isFixating)
	{
		return true;
	}

	if (ActiveFixation.IsLocal)
	{
		if (eyeCapture.UseCaptureMatrix == false)
		{
			eyeCapture.SkipPositionForFixationAverage = true;
		}
		else if (eyeCapture.HitDynamicId != ActiveFixation.DynamicObjectId)
		{
			eyeCapture.SkipPositionForFixationAverage = true;
		}

		if (eyeCapture.SkipPositionForFixationAverage || eyeCapture.OffTransform) //use local fixation without new fixation point
		{
			FVector fixationWorldPosition = ActiveFixation.Transformation.TransformPosition(ActiveFixation.LocalPosition);
			FVector fixationDirection = (fixationWorldPosition - eyeCapture.HMDPosition);
			fixationDirection.Normalize();
			
			FVector eyeCaptureWorldPosition = ActiveFixation.Transformation.TransformPosition(eyeCapture.LocalPosition);
			FVector eyeCaptureDirection = (eyeCaptureWorldPosition - eyeCapture.HMDPosition);
			eyeCaptureDirection.Normalize();

			float rescale = 1;
			if (FocusSizeFromCenter != NULL)
			{
				const FVector2D ViewportSize = FVector2D(GEngine->GameViewport->Viewport->GetSizeXY());
				FVector2D viewport2d = FVector2D(eyeCapture.ScreenPos.X / ViewportSize.X, eyeCapture.ScreenPos.Y / ViewportSize.Y);
				float screenDist = FVector2D::Distance(viewport2d, FVector2D(0.5f, 0.5f));
				rescale = FocusSizeFromCenter->GetFloatValue(screenDist);
			}

			float _adjusteddotangle = FMath::Cos(FMath::DegreesToRadians(MaxFixationAngle * rescale));

			if (FVector::DotProduct(eyeCaptureDirection, fixationDirection) < _adjusteddotangle)
			{
				return true;
			}
		}
		else
		{
			FVector eyeCaptureWorldPosition = ActiveFixation.Transformation.TransformPosition(eyeCapture.LocalPosition);
			FVector activeFixationWorldPos = ActiveFixation.Transformation.TransformPosition(ActiveFixation.LocalPosition);
			FVector averageLocalPosition = FVector::ZeroVector;
			for (int32 i = 0; i < CachedEyeCapturePositions.Num(); i++)
			{
				averageLocalPosition += CachedEyeCapturePositions[i];
			}

			averageLocalPosition += eyeCapture.LocalPosition;
			averageLocalPosition /= (CachedEyeCapturePositions.Num() + 1);
			if (DebugDisplayFixations)
				DrawDebugSphere(world, activeFixationWorldPos, 16,8, FColor::Magenta);

			FVector fixationDirection = (activeFixationWorldPos - eyeCapture.HMDPosition);
			
			float mag = FMath::Sqrt(fixationDirection.X*fixationDirection.X + fixationDirection.Y * fixationDirection.Y + fixationDirection.Z * fixationDirection.Z);
			fixationDirection /= mag;
			
			FVector eyeCaptureDirection = (eyeCaptureWorldPosition - eyeCapture.HMDPosition);
			mag = FMath::Sqrt(eyeCaptureDirection.X*eyeCaptureDirection.X + eyeCaptureDirection.Y * eyeCaptureDirection.Y + eyeCaptureDirection.Z * eyeCaptureDirection.Z);
			eyeCaptureDirection /= mag;

			float rescale = 1;
			if (FocusSizeFromCenter != NULL)
			{
				const FVector2D ViewportSize = FVector2D(GEngine->GameViewport->Viewport->GetSizeXY());
				FVector2D viewport2d = FVector2D(eyeCapture.ScreenPos.X / ViewportSize.X, eyeCapture.ScreenPos.Y / ViewportSize.Y);
				float screenDist = FVector2D::Distance(viewport2d, FVector2D(0.5f, 0.5f));
				rescale = FocusSizeFromCenter->GetFloatValue(screenDist);
			}

			float _adjusteddotangle = FMath::Cos(FMath::DegreesToRadians(MaxFixationAngle * rescale));

			float dot = FVector::DotProduct(eyeCaptureDirection, fixationDirection);
			if (dot < _adjusteddotangle)
			{
				//out of range of fixation
				return true;
			}

			float distance = FVector::Dist(activeFixationWorldPos, eyeCapture.HMDPosition);
			float currentRadius = FMath::Atan(FMath::DegreesToRadians(MaxFixationAngle))*distance;
			ActiveFixation.MaxRadius = FMath::Max(ActiveFixation.MaxRadius, currentRadius);

			CachedEyeCapturePositions.Add(eyeCapture.LocalPosition);
			if (CachedEyeCapturePositions.Num() > 120)
				CachedEyeCapturePositions.RemoveAt(0);
			ActiveFixation.LocalPosition = averageLocalPosition;
		}
		ActiveFixation.LastInRange = eyeCapture.Time;
		return false;
	}
	else //world fixation
	{
		if (eyeCapture.UseCaptureMatrix == true)
		{
			eyeCapture.SkipPositionForFixationAverage = true;
		}

		if (eyeCapture.SkipPositionForFixationAverage)
		{
			float rescale = 1;
			const FVector2D ViewportSize = FVector2D(GEngine->GameViewport->Viewport->GetSizeXY());
			if (FocusSizeFromCenter)
			{
				FVector2D viewport2d = FVector2D(eyeCapture.ScreenPos.X / ViewportSize.X, eyeCapture.ScreenPos.Y / ViewportSize.Y);
				float screenDist = FVector2D::Distance(viewport2d, FVector2D(0.5f, 0.5f));
				rescale = FocusSizeFromCenter->GetFloatValue(screenDist);
			}

			FVector lookDir = eyeCapture.WorldPosition - eyeCapture.HMDPosition;
			lookDir.Normalize();
			FVector fixationDir = ActiveFixation.WorldPosition - eyeCapture.HMDPosition;
			fixationDir.Normalize();
			float angle = FMath::Cos(FMath::DegreesToRadians(MaxFixationAngle * rescale));
			if (FVector::DotProduct(lookDir, fixationDir) < angle)
			{
				return true;
			}
			else
			{
			}
		}
		else
		{
			//update average world position and test against that
			FVector averageWorldPos = FVector::ZeroVector;
			for (int32 i = 0; i < CachedEyeCapturePositions.Num(); i++)
			{
				averageWorldPos += CachedEyeCapturePositions[i];
			}
			averageWorldPos += eyeCapture.WorldPosition;
			averageWorldPos /= (CachedEyeCapturePositions.Num() + 1);

			float rescale = 1;
			const FVector2D ViewportSize = FVector2D(GEngine->GameViewport->Viewport->GetSizeXY());
			if (FocusSizeFromCenter)
			{
				FVector2D viewport2d = FVector2D(eyeCapture.ScreenPos.X / ViewportSize.X, eyeCapture.ScreenPos.Y / ViewportSize.Y);
				float screenDist = FVector2D::Distance(viewport2d, FVector2D(0.5f, 0.5f));
				rescale = FocusSizeFromCenter->GetFloatValue(screenDist);
			}

			//test look/fixation directions
			FVector lookDir = eyeCapture.WorldPosition - eyeCapture.HMDPosition;
			lookDir.Normalize();
			FVector fixationDir = ActiveFixation.WorldPosition - eyeCapture.HMDPosition;
			fixationDir.Normalize();

			float angle = FMath::Cos(FMath::DegreesToRadians(MaxFixationAngle * rescale));
			if (FVector::DotProduct(lookDir, fixationDir) < angle)
			{
				return true;
			}

			//add to cached capture positions
			float distance = FVector::Dist(ActiveFixation.WorldPosition, eyeCapture.HMDPosition);
			float currentRadius = FMath::Atan(FMath::DegreesToRadians(MaxFixationAngle))*distance;
			ActiveFixation.MaxRadius = FMath::Max(ActiveFixation.MaxRadius, currentRadius);

			CachedEyeCapturePositions.Add(eyeCapture.WorldPosition);
			ActiveFixation.WorldPosition = averageWorldPos;
		}
		ActiveFixation.LastInRange = eyeCapture.Time;
	}
	return false;
}
bool UFixationRecorder::IsGazeOffTransform(const FEyeCapture& eyeCapture)
{
	if (ActiveFixation.IsLocal)
	{
		if (!eyeCapture.UseCaptureMatrix)
		{
			return true;
		}
		if (eyeCapture.HitDynamicId != ActiveFixation.DynamicObjectId)
		{
			return true;
		}
	}

	return eyeCapture.OffTransform;
}

bool UFixationRecorder::CheckEndFixation(const FFixation& testFixation)
{
	if (EyeCaptures[index].Time > testFixation.LastInRange + SaccadeFixationEndMs)
	{
		if (WasOutOfDispersionLastFrame)
		{
			return true;
		}
	}
	if (EyeCaptures[index].Time > testFixation.LastEyesOpen + MaxBlinkMs)
	{
		return true;
	}
	if (EyeCaptures[index].Time > testFixation.LastNonDiscardedTime + MaxConsecutiveDiscardMs)
	{
		if (WasCaptureDiscardedLastFrame)
		{
			return true;
		}
	}
	if (testFixation.IsLocal)
	{
		if (EyeCaptures[index].Time > testFixation.LastOnTransform + MaxConsecutiveOffDynamicMs)
		{
			return true;
		}
	}
	return false;
}

#if defined TOBII_EYETRACKING_ACTIVE
int64 UFixationRecorder::GetEyeCaptureTimestamp(TSharedPtr<ITobiiEyeTracker, ESPMode::ThreadSafe> eyetracker)
{
	int32 t = eyetracker->GetCombinedGazeData().TimeStamp.ToUnixTimestamp();
	int64 ts = t;
	ts *= 1000;
	ts += eyetracker->GetCombinedGazeData().TimeStamp.GetMillisecond();
	return ts;
}

bool UFixationRecorder::AreEyesClosed(TSharedPtr<ITobiiEyeTracker, ESPMode::ThreadSafe> eyetracker)
{
	bool leftEyeClosed = eyetracker->GetLeftGazeData().EyeOpenness < 0.3f;
	bool rightEyeClosed = eyetracker->GetRightGazeData().EyeOpenness < 0.3f;

	if (leftEyeClosed && rightEyeClosed)
	{
		if (!eyesClosed)
		{
			for (int32 i = 0; i < CachedEyeCaptureCount; i++)
			{
				if (EyeCaptures[index].Time - PreBlinkDiscardMs > EyeCaptures[GetIndex(-i)].Time)
				{
					EyeCaptures[GetIndex(-i)].EyesClosed = true;
				}
				else
				{
					break;
				}
			}
		}
		eyesClosed = true;
		return true;
	}

	if (eyesClosed && !leftEyeClosed && !rightEyeClosed)
	{
		eyesClosed = false;
		EyeUnblinkTime = EyeCaptures[index].Time;
	}

	if (EyeUnblinkTime + BlinkEndWarmupMs > EyeCaptures[index].Time)
	{
		return true;
	}
	return false;
}

#elif defined SRANIPAL_1_2_API
bool UFixationRecorder::AreEyesClosed()
{
	float leftOpenness = 0;
	bool leftValid = USRanipal_FunctionLibrary_Eye::GetEyeOpenness(EyeIndex::LEFT, leftOpenness);

	float rightOpenness;
	bool rightValid = USRanipal_FunctionLibrary_Eye::GetEyeOpenness(EyeIndex::RIGHT, rightOpenness);

	if (leftValid && leftOpenness < 0.5f) { return true; }
	if (rightValid && rightOpenness < 0.5f) { return true; }

	return false;
}
int64 UFixationRecorder::GetEyeCaptureTimestamp()
{
	int64 ts = (int64)(Util::GetTimestamp() * 1000);
	return ts;
}
#elif defined SRANIPAL_1_3_API
bool UFixationRecorder::AreEyesClosed()
{
	float leftOpenness = 0;
	bool leftValid = SRanipalEye_Core::Instance()->GetEyeOpenness(EyeIndex::LEFT, leftOpenness);

	float rightOpenness;
	bool rightValid = SRanipalEye_Core::Instance()->GetEyeOpenness(EyeIndex::RIGHT, rightOpenness);

	if (leftValid && leftOpenness < 0.5f) { return true; }
	if (rightValid && rightOpenness < 0.5f) { return true; }

	return false;
}

int64 UFixationRecorder::GetEyeCaptureTimestamp()
{
	int64 ts = (int64)(Util::GetTimestamp() * 1000);
	return ts;
}
#elif defined VARJOEYETRACKER_API
bool UFixationRecorder::AreEyesClosed()
{
	FVarjoEyeTrackingData eyeTrackingData;
	UVarjoEyeTrackerFunctionLibrary::GetEyeTrackerGazeData(eyeTrackingData);
	if (eyeTrackingData.leftStatus == 0)
	{
		return true;
	}
	if (eyeTrackingData.rightStatus == 0)
	{
		return true;
	}
	return false;
}

int64 UFixationRecorder::GetEyeCaptureTimestamp()
{
	int64 ts = (int64)(Util::GetTimestamp() * 1000);
	return ts;
}
#elif defined PICOMOBILE_API
bool UFixationRecorder::AreEyesClosed()
{
	FVector Origin;
	FVector Dir;
	return !UPicoBlueprintFunctionLibrary::PicoGetEyeTrackingGazeRay(Origin, Dir);
}

int64 UFixationRecorder::GetEyeCaptureTimestamp()
{
	int64 ts = (int64)(Util::GetTimestamp() * 1000);
	return ts;
}
#elif defined HPGLIA_API
bool UFixationRecorder::AreEyesClosed()
{
	FEyeTracking eyeTrackingData;
	if (UHPGliaClient::GetEyeTracking(eyeTrackingData))
	{
		if (eyeTrackingData.LeftEyeOpenness > 0.4f) { return false; }
		if (eyeTrackingData.RightEyeOpenness > 0.4f) { return false; }
	}
	return true;
}

int64 UFixationRecorder::GetEyeCaptureTimestamp()
{
	int64 ts = (int64)(Util::GetTimestamp() * 1000);
	return ts;
}
#elif defined OPENXR_EYETRACKING
bool UFixationRecorder::AreEyesClosed()
{
	IEyeTracker const* const ET = GEngine ? GEngine->EyeTrackingDevice.Get() : nullptr;
	if (ET == NULL) { return false; }
	FEyeTrackerGazeData gazeData;
	ET->GetEyeTrackerGazeData(gazeData);
	if (gazeData.ConfidenceValue < 0.5f)
	{
		return true;
	}
	return false;
}
int64 UFixationRecorder::GetEyeCaptureTimestamp()
{
	int64 ts = (int64)(Util::GetTimestamp() * 1000);
	return ts;
}
#else

bool UFixationRecorder::AreEyesClosed()
{
	return false;
}

int64 UFixationRecorder::GetEyeCaptureTimestamp()
{
	int64 ts = (int64)(Util::GetTimestamp() * 1000);
	return ts;
}

#endif

void UFixationRecorder::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	if (!cog.IsValid()){return;}
	//don't record player position data before a session has begun
	if (!cog->HasStartedSession())
	{
		return;
	}

	if (!isFixating)
	{
		if (TryBeginLocalFixation())
		{
			ActiveFixation.IsLocal = true;
			isFixating = true;
		}
		else if (TryBeginFixation())
		{
			ActiveFixation.IsLocal = false;
			isFixating = true;
		}
	}
	else
	{
		if (ActiveFixation.IsLocal)
		{
			if (EyeCaptures[index].HitDynamicId != ActiveFixation.DynamicObjectId)
				EyeCaptures[index].SkipPositionForFixationAverage = true;
		}
		else
		{
			if (EyeCaptures[index].UseCaptureMatrix)
				EyeCaptures[index].SkipPositionForFixationAverage = true;
		}
		bool IsOutOfRange = IsGazeOutOfRange(EyeCaptures[index]);
		if (!IsOutOfRange)
		{

		}
		else
		{
			EyeCaptures[index].OutOfRange = true;
		}

		EyeCaptures[index].OffTransform = IsGazeOffTransform(EyeCaptures[index]);
		ActiveFixation.AddEyeCapture(EyeCaptures[index]);

		if (DebugDisplayFixations)
		{
			//update world position from transformed local position for visualization
			if (ActiveFixation.IsLocal)
			{
				FVector fixationWorldPos;
				fixationWorldPos = ActiveFixation.Transformation.TransformPosition(ActiveFixation.LocalPosition);
				if (IsOutOfRange)
					//DrawDebugBox(world, fixationWorldPos, FVector::OneVector * 15, FColor::Red, false);
					DrawDebugSphere(world, fixationWorldPos, 20, 8, FColor::Red);
				else
					//DrawDebugBox(world, fixationWorldPos, FVector::OneVector * 15, FColor::Green, false);
					DrawDebugSphere(world, fixationWorldPos, 20, 8, FColor::Green);
			}
			else
			{
				if (IsOutOfRange)
					DrawDebugBox(world, ActiveFixation.WorldPosition, FVector::OneVector * 15, FColor::Red, false);
				else
					DrawDebugBox(world, ActiveFixation.WorldPosition, FVector::OneVector * 15, FColor::Green, false);
			}
		}

		if (CheckEndFixation(ActiveFixation))
		{
			RecordFixationEnd(ActiveFixation);
			isFixating = false;
			CachedEyeCapturePositions.Empty();
		}
		WasOutOfDispersionLastFrame = IsOutOfRange;
	}

	WasCaptureDiscardedLastFrame = EyeCaptures[index].Discard;

	EyeCaptures[index].Discard = false;
	EyeCaptures[index].SkipPositionForFixationAverage = false;
	EyeCaptures[index].OffTransform = true;
	EyeCaptures[index].OutOfRange = false;
	EyeCaptures[index].HitDynamicId.Empty();

	//============== draw some rays from the HMD
	EyeCaptures[index].HMDPosition = cog->GetPlayerHMDPosition();
#if defined TOBII_EYETRACKING_ACTIVE
	auto eyetracker = ITobiiCore::GetEyeTracker();
	EyeCaptures[index].EyesClosed = AreEyesClosed(eyetracker);
	EyeCaptures[index].Time = GetEyeCaptureTimestamp(eyetracker);
	FVector Start = eyetracker->GetCombinedGazeData().WorldGazeOrigin;
	FVector End = eyetracker->GetCombinedGazeData().WorldGazeOrigin + eyetracker->GetCombinedGazeData().WorldGazeDirection * MaxFixationDistance;
#elif defined SRANIPAL_1_2_API
	EyeCaptures[index].EyesClosed = AreEyesClosed();
	EyeCaptures[index].Time = GetEyeCaptureTimestamp();

	FVector Start = FVector::ZeroVector;
	FVector LocalDirection = FVector::ZeroVector;
	FVector End = FVector::ZeroVector;

	if (USRanipal_FunctionLibrary_Eye::GetGazeRay(GazeIndex::COMBINE, Start, LocalDirection))
	{
		if (controllers.Num() == 0)
		{
			CognitiveLog::Info("FixationRecorder::TickComponent - no controllers");
			return;
		}

		FVector captureLocation = controllers[0]->PlayerCameraManager->GetCameraLocation();
		Start = captureLocation;

		FVector WorldDir = controllers[0]->PlayerCameraManager->GetActorTransform().TransformPositionNoScale(LocalDirection);
		End = captureLocation + WorldDir * MaxFixationDistance;
	}
	else
	{
		EyeCaptures[index].Discard = true;
	}
#elif defined SRANIPAL_1_3_API
	EyeCaptures[index].EyesClosed = AreEyesClosed();
	EyeCaptures[index].Time = GetEyeCaptureTimestamp();

	FVector Start = FVector::ZeroVector;
	FVector LocalDirection = FVector::ZeroVector;
	FVector End = FVector::ZeroVector;

	if (SRanipalEye_Core::Instance()->GetGazeRay(GazeIndex::COMBINE, Start, LocalDirection))
	{
		if (controllers.Num() == 0)
		{
			CognitiveLog::Info("FixationRecorder::TickComponent - no controllers");
			return;
		}

		FVector captureLocation = controllers[0]->PlayerCameraManager->GetCameraLocation();
		Start = captureLocation;

		FVector WorldDir = controllers[0]->PlayerCameraManager->GetActorTransform().TransformVectorNoScale(LocalDirection);
		End = captureLocation + WorldDir * MaxFixationDistance;
	}
	else
	{
		EyeCaptures[index].Discard = true;
}
#elif defined VARJOEYETRACKER_API
	EyeCaptures[index].EyesClosed = AreEyesClosed();
	EyeCaptures[index].Time = GetEyeCaptureTimestamp();

	FVector LocalStart = FVector::ZeroVector;
	FVector Start = FVector::ZeroVector;
	FVector WorldDirection = FVector::ZeroVector;
	FVector End = FVector::ZeroVector;
	float ignored = 0;

	FVarjoEyeTrackingData eyeTrackingData;

	if (UVarjoEyeTrackerFunctionLibrary::GetEyeTrackerGazeData(eyeTrackingData)) //if the data is valid
	{
		//the gaze transformed into world space
		UVarjoEyeTrackerFunctionLibrary::GetGazeRay(Start, WorldDirection, ignored);

		End = Start + WorldDirection * MaxFixationDistance;
	}
	else
	{
		EyeCaptures[index].Discard = true;
	}

#elif defined PICOMOBILE_API
	EyeCaptures[index].EyesClosed = AreEyesClosed();
	EyeCaptures[index].Time = GetEyeCaptureTimestamp();

	FVector Start = FVector::ZeroVector;
	FVector WorldDirection = FVector::ZeroVector;
	FVector End = FVector::ZeroVector;

	if (UPicoBlueprintFunctionLibrary::PicoGetEyeTrackingGazeRay(Start, WorldDirection))
	{
		End = Start + WorldDirection * MaxFixationDistance;
	}
	else
	{
		EyeCaptures[index].Discard = true;
	}

#elif defined HPGLIA_API
	EyeCaptures[index].EyesClosed = AreEyesClosed();
	EyeCaptures[index].Time = GetEyeCaptureTimestamp();

	FVector Start = FVector::ZeroVector;
	FVector WorldDirection = FVector::ZeroVector;
	FVector End = FVector::ZeroVector;

	FEyeTracking eyeTrackingData;
	if (UHPGliaClient::GetEyeTracking(eyeTrackingData))
	{
		if (eyeTrackingData.CombinedGazeConfidence < 0.4f) { EyeCaptures[index].Discard = true; }
		else
		{
			FVector dir = FVector(eyeTrackingData.CombinedGaze.X, eyeTrackingData.CombinedGaze.Y, eyeTrackingData.CombinedGaze.Z);
			WorldDirection = controllers[0]->PlayerCameraManager->GetActorTransform().TransformVectorNoScale(dir);

			FVector captureLocation = controllers[0]->PlayerCameraManager->GetCameraLocation();
			Start = captureLocation;
			End = captureLocation + WorldDirection * MaxFixationDistance;
		}
	}
	else
	{
		EyeCaptures[index].Discard = true;
	}
#elif defined OPENXR_EYETRACKING
	EyeCaptures[index].EyesClosed = AreEyesClosed();
	EyeCaptures[index].Time = GetEyeCaptureTimestamp();

	FVector Start = FVector::ZeroVector;
	FVector WorldDirection = FVector::ZeroVector;
	FVector End = FVector::ZeroVector;
	IEyeTracker const* const ET = GEngine ? GEngine->EyeTrackingDevice.Get() : nullptr;
	if (ET == NULL) { EyeCaptures[index].Discard = true; }
	else
	{
		EEyeTrackerStatus status = ET->GetEyeTrackerStatus();
		if (status != EEyeTrackerStatus::Tracking) { EyeCaptures[index].Discard = true; }
		else
		{
			FEyeTrackerGazeData gazeData;
			ET->GetEyeTrackerGazeData(gazeData);
			if (gazeData.ConfidenceValue < 0.4f) { EyeCaptures[index].Discard = true; }
			else
			{
				WorldDirection = gazeData.GazeDirection;
				//unclear if the OpenXR gaze direction is world or local
				#if defined OPENXR_LOCALSPACE
				WorldDirection = controllers[0]->PlayerCameraManager->GetActorTransform().TransformVectorNoScale(gazeData.GazeDirection);
				#endif
				Start = gazeData.GazeOrigin;
				End = Start + WorldDirection * MaxFixationDistance;
			}
		}
	}
#else
	EyeCaptures[index].EyesClosed = AreEyesClosed();
	EyeCaptures[index].Time = GetEyeCaptureTimestamp();

	EyeCaptures[index].Discard = false;
	CognitiveLog::Error("FixationRecorder::TickComponent - no eye tracking SDKs found!");

	FVector Start = controllers[0]->PlayerCameraManager->GetCameraLocation();
	FVector End = Start + controllers[0]->PlayerCameraManager->GetActorForwardVector() * MaxFixationDistance;
#endif

	if (EyeCaptures[index].EyesClosed)
	{
		EyeCaptures[index].SkipPositionForFixationAverage = true;
	}


	TSharedPtr<FC3DGazePoint> currentGazePoint = MakeShareable(new FC3DGazePoint);

	FCollisionQueryParams Params; // You can use this to customize various properties about the trace
								  //Params.AddIgnoredActor(GetOwner()); // Ignore the player's pawn

	FHitResult Hit; // The hit result gets populated by the line trace

	bool bHit = false;
	FCollisionQueryParams gazeparams = FCollisionQueryParams(FName(), true);
	bHit = world->LineTraceSingleByChannel(Hit, Start, End, ECollisionChannel::ECC_Visibility, gazeparams);
	if (bHit)
	{
		//hit BSP or actor
		currentGazePoint->WorldPosition = Hit.ImpactPoint;
		EyeCaptures[index].WorldPosition = Hit.ImpactPoint;
		UGameplayStatics::GetGameInstance(this)->GetFirstLocalPlayerController()->ProjectWorldLocationToScreen(EyeCaptures[index].WorldPosition, EyeCaptures[index].ScreenPos);

		EyeCaptures[index].UseCaptureMatrix = false;
		currentGazePoint->IsLocal = false;
		EyeCaptures[index].OffTransform = false;
		EyeCaptures[index].HitDynamicId.Empty();

		if (Hit.GetActor() != NULL)
		{
			UDynamicObject* dyn = Hit.GetActor()->FindComponentByClass<UDynamicObject>();
			if (dyn != NULL)
			{
				EyeCaptures[index].UseCaptureMatrix = true;
				EyeCaptures[index].CaptureMatrix = Hit.GetActor()->GetActorTransform();
				EyeCaptures[index].HitDynamicId = dyn->GetObjectId()->Id;
				EyeCaptures[index].OffTransform = false;
				EyeCaptures[index].LocalPosition = Hit.GetActor()->GetActorTransform().InverseTransformPosition(Hit.ImpactPoint);
				//IMPROVEMENT possible issue with non-uniform scale. should use FMatrix instead?
				//https://stackoverflow.com/questions/53887451/incorrect-results-of-simple-coordinate-transformation-in-ue4
				//display stuff

				currentGazePoint->LocalPosition = EyeCaptures[index].LocalPosition;
				currentGazePoint->Parent = dyn;
			}
		}
	}
	else
	{
		EyeCaptures[index].SkipPositionForFixationAverage = true;
		EyeCaptures[index].UseCaptureMatrix = false;
		EyeCaptures[index].LocalPosition = FVector::ZeroVector;
		EyeCaptures[index].WorldPosition = End; //direction of gaze 100m out
		UGameplayStatics::GetGameInstance(this)->GetFirstLocalPlayerController()->ProjectWorldLocationToScreen(End, EyeCaptures[index].ScreenPos);
		currentGazePoint->WorldPosition = End;
	}

	recentEyePositions.Add(currentGazePoint);
	if (recentEyePositions.Num() > 200)
	{
		recentEyePositions.RemoveAt(0);
	}

	index = (index + 1) % CachedEyeCaptureCount;
}

bool UFixationRecorder::TryBeginLocalFixation()
{
	int32 sampleCount = 0;
	TArray<FEyeCapture*> usedCaptures;
	TArray<FString> hitDynamicIds;
	int64 firstOnTransformTime = 0;
	int64 firstSampleTime = LONG_MAX;
	int64 lastSampleTime = 0;

	//add relevant eye capture samples to array
	for (int32 i = 0; i < CachedEyeCaptureCount; i++)
	{
		if (EyeCaptures[GetIndex(i)].Discard || EyeCaptures[GetIndex(i)].EyesClosed) { return false; }
		if (!EyeCaptures[GetIndex(i)].UseCaptureMatrix) { return false; }
		if (EyeCaptures[GetIndex(i)].SkipPositionForFixationAverage) { return false; }
		firstSampleTime = FMath::Min(firstSampleTime, EyeCaptures[GetIndex(i)].Time);
		lastSampleTime = FMath::Max(lastSampleTime, EyeCaptures[GetIndex(i)].Time);
		sampleCount++;
		usedCaptures.Add(&EyeCaptures[GetIndex(i)]);
		if (firstOnTransformTime < 1)
			firstOnTransformTime = EyeCaptures[GetIndex(i)].Time;
		if (EyeCaptures[GetIndex(i)].UseCaptureMatrix)
		{
			hitDynamicIds.Add(EyeCaptures[GetIndex(i)].HitDynamicId);
		}
		if (EyeCaptures[index].Time + MinFixationMs < EyeCaptures[GetIndex(i)].Time) { break; }
	}

	if (sampleCount < 2)
	{
		return false;
	}

	if (usedCaptures.Num() >= 2)
	{
		if ((usedCaptures[usedCaptures.Num() - 1]->Time - usedCaptures[0]->Time) < MinFixationMs) { return false; }
	}
	else
	{
		//don't have enough eye tracking samples
		return false;
	}

	//duration of first sample to last sample
	int64 duration = lastSampleTime - firstSampleTime;
	if (duration < MinFixationMs)
	{
		return false;
	}

	if (EyeCaptures[index].Time - firstOnTransformTime > MaxConsecutiveOffDynamicMs)
	{
		return false;
	}


	//================================= CALCULATE HIT DYNAMIC IDS
	TMap<FString, int32> hitCounts;
	FVector averageLocalPosition = FVector::ZeroVector;
	FVector averageWorldPosition = FVector::ZeroVector;

	int32 anyHitTransformCount = 0;
	for (auto& c : usedCaptures)
	{
		if (c->UseCaptureMatrix)
		{
			if (hitCounts.Contains(c->HitDynamicId))
			{
				hitCounts[c->HitDynamicId] += 1;
			}
			else
			{
				hitCounts.Add(c->HitDynamicId, 1);
			}
			anyHitTransformCount++;
		}
	}

	//escape if no eye captures are using dynamic object transform matrix (this is possibly redundant)
	if (anyHitTransformCount == 0)
	{
		return false;
	}

	//======= figure out most used DynamicObjectId
	int32 useCount = 0;
	FString mostUsedId;

	for (auto& t : hitCounts)
	{
		if (t.Value > useCount)
		{
			mostUsedId = t.Key;
			useCount = t.Value;
		}
	}

	if (mostUsedId.IsEmpty())
	{
		//most used dynamic object id is none! something is wrong somehow
		CognitiveLog::Error("UFixationRecorder::TryBeginLocalFixation most used dynamic object is null! should be impossible");
		return false;
	}

	//the first eye capture that hits the dynamic object
	FEyeCapture* referenceCapture = NULL;

	for (auto& c : usedCaptures)
	{
		if (c->HitDynamicId != mostUsedId) { continue; }
		if (!c->UseCaptureMatrix) { continue; }
		referenceCapture = c;
		break;
	}
	if (referenceCapture == NULL)
	{
		return false;
	}

	//======== average positions and check if fixations are within radius
	
	int32 usedAveragePositionCount = 0;
	for (auto& c : usedCaptures)
	{
		if (c->HitDynamicId != mostUsedId) { continue; }
		if (!c->UseCaptureMatrix) { continue; }
		usedAveragePositionCount++;
		averageLocalPosition += c->LocalPosition;
	}
	averageLocalPosition /= usedAveragePositionCount;
	averageWorldPosition = referenceCapture->CaptureMatrix.TransformPosition(averageLocalPosition);

	const FVector2D ViewportSize = FVector2D(GEngine->GameViewport->Viewport->GetSizeXY());

	float rescale = 1;
	bool withinRadius = true;

	//check that all samples are nearby
	for (auto& c : usedCaptures)
	{
		if (c->HitDynamicId != mostUsedId) { continue; }
		if (!c->UseCaptureMatrix) { continue; }

		if (FocusSizeFromCenter != NULL)
		{
			FVector2D viewport2d = FVector2D(c->ScreenPos.X / ViewportSize.X, c->ScreenPos.Y / ViewportSize.Y);
			float screenDist = FVector2D::Distance(viewport2d, FVector2D(0.5f, 0.5f));
			rescale = FocusSizeFromCenter->GetFloatValue(screenDist);
		}
		float adjusteddotangle = FMath::Cos(FMath::DegreesToRadians(MaxFixationAngle * rescale));

		FVector lookDir = c->CaptureMatrix.TransformPosition(c->LocalPosition) - c->HMDPosition;
		lookDir.Normalize();
		FVector fixationDir = averageWorldPosition - c->HMDPosition;
		fixationDir.Normalize();

		if (FVector::DotProduct(lookDir, fixationDir) < adjusteddotangle)
		{
			withinRadius = false;
			break;
		}
	}

	if (withinRadius)
	{
		ActiveFixation.LocalPosition = averageLocalPosition;
		ActiveFixation.WorldPosition = averageWorldPosition;
		ActiveFixation.DynamicObjectId = mostUsedId;

		float distance = FVector::Dist(ActiveFixation.WorldPosition, EyeCaptures[index].HMDPosition);
		float opposite = FMath::Atan(FMath::DegreesToRadians(MaxFixationAngle)) * distance;

		ActiveFixation.StartMs = EyeCaptures[index].Time;
		ActiveFixation.LastInRange = ActiveFixation.StartMs;
		ActiveFixation.LastEyesOpen = ActiveFixation.StartMs;
		ActiveFixation.LastNonDiscardedTime = ActiveFixation.StartMs;
		ActiveFixation.LastOnTransform = ActiveFixation.StartMs;
		ActiveFixation.StartDistance = distance;
		ActiveFixation.MaxRadius = opposite;
		ActiveFixation.IsLocal = true;
		ActiveFixation.Transformation = referenceCapture->CaptureMatrix;

		for (auto& c : usedCaptures)
		{
			if (c->SkipPositionForFixationAverage) { continue; }
			if (c->UseCaptureMatrix && c->HitDynamicId == ActiveFixation.DynamicObjectId)
			{
				c->SkipPositionForFixationAverage = true;
				CachedEyeCapturePositions.Add(c->LocalPosition);
			}
		}

		for (auto& c : usedCaptures)
		{
			if (c->UseCaptureMatrix && c->HitDynamicId == ActiveFixation.DynamicObjectId)
			{
				ActiveFixation.AddEyeCapture(*c);
				break;
			}
		}
		recentFixationPoints.Empty(10);
		recentFixationPoints.Add(ActiveFixation);
		WasOutOfDispersionLastFrame = false;
		return true;
	}
	return false;
}

bool UFixationRecorder::TryBeginFixation()
{
	FVector averageWorldPos = FVector::ZeroVector;
	int32 averageWorldSamples = 0;
	int32 sampleCount = 0;

	int64 firstSampleTime = LONG_MAX;
	int64 lastSampleTime = 0;

	for (int32 i = 0; i < CachedEyeCaptureCount; i++)
	{
		if (EyeCaptures[GetIndex(i)].Discard || EyeCaptures[GetIndex(i)].EyesClosed) { return false; }
		if (EyeCaptures[GetIndex(i)].UseCaptureMatrix) { return false; }
		if (EyeCaptures[GetIndex(i)].SkipPositionForFixationAverage) { return false; }
		firstSampleTime = FMath::Min(firstSampleTime, EyeCaptures[GetIndex(i)].Time);
		lastSampleTime = FMath::Max(lastSampleTime, EyeCaptures[GetIndex(i)].Time);
		sampleCount++;
		averageWorldPos += EyeCaptures[GetIndex(i)].WorldPosition;
		averageWorldSamples++;
		if (EyeCaptures[index].Time + MinFixationMs < EyeCaptures[GetIndex(i)].Time) { break; }
	}
	if (averageWorldSamples < 2)
	{
		return false;
	}

	//duration of first sample to last sample
	int64 duration = lastSampleTime - firstSampleTime;
	if (duration < MinFixationMs)
	{
		return false;
	}

	averageWorldPos /= averageWorldSamples;

	bool withinRadius = true;

	const FVector2D ViewportSize = FVector2D(GEngine->GameViewport->Viewport->GetSizeXY());

	for (int32 i = 0; i < sampleCount; i++)
	{
		float rescale = 1;
		if (FocusSizeFromCenter)
		{
			FVector2D viewport2d = FVector2D(EyeCaptures[GetIndex(i)].ScreenPos.X / ViewportSize.X, EyeCaptures[GetIndex(i)].ScreenPos.Y / ViewportSize.Y);
			float screenDist = FVector2D::Distance(viewport2d, FVector2D(0.5f, 0.5f));
			rescale = FocusSizeFromCenter->GetFloatValue(screenDist);
		}

		float angle = FMath::Cos(FMath::DegreesToRadians(MaxFixationAngle * rescale));

		FVector lookDir = (EyeCaptures[GetIndex(i)].HMDPosition - EyeCaptures[GetIndex(i)].WorldPosition);
		lookDir.Normalize();
		FVector fixationDir = EyeCaptures[GetIndex(i)].HMDPosition - averageWorldPos;
		fixationDir.Normalize();

		if (FVector::DotProduct(lookDir, fixationDir) < angle)
		{
			withinRadius = false;
			break;
		}
	}

	if (withinRadius)
	{
		ActiveFixation.WorldPosition = averageWorldPos;
		float distance = FVector::Dist(ActiveFixation.WorldPosition, EyeCaptures[index].HMDPosition);
		float opposite = FMath::Atan(FMath::DegreesToRadians(MaxFixationAngle)) * distance;

		ActiveFixation.StartMs = EyeCaptures[index].Time;
		ActiveFixation.LastInRange = ActiveFixation.StartMs;
		ActiveFixation.StartDistance = distance;
		ActiveFixation.MaxRadius = opposite;
		ActiveFixation.LastEyesOpen = EyeCaptures[index].Time;
		ActiveFixation.LastNonDiscardedTime = EyeCaptures[index].Time;
		ActiveFixation.LastInRange = EyeCaptures[index].Time;

		for (int32 i = 0; i < sampleCount; i++)
		{
			if (EyeCaptures[GetIndex(i)].SkipPositionForFixationAverage) { continue; }
			EyeCaptures[GetIndex(i)].SkipPositionForFixationAverage = true;
			CachedEyeCapturePositions.Add(EyeCaptures[GetIndex(i)].WorldPosition);
		}

		recentFixationPoints.Add(ActiveFixation);
		if (recentFixationPoints.Num() > 10)
		{
			recentFixationPoints.RemoveAt(0);
		}
		return true;
	}
	return false;
}

void UFixationRecorder::RecordFixationEnd(const FFixation& fixation)
{
	//write fixation to json
	TSharedPtr<FJsonObject>fixObj = MakeShareable(new FJsonObject);

	double d = (double)fixation.StartMs / 1000.0;

	fixObj->SetNumberField("time", d);
	fixObj->SetNumberField("duration", fixation.DurationMs);
	fixObj->SetNumberField("maxradius", fixation.MaxRadius);

	if (fixation.IsLocal)
	{
		TArray<TSharedPtr<FJsonValue>> posArray;
		TSharedPtr<FJsonValueNumber> JsonValue;
		JsonValue = MakeShareable(new FJsonValueNumber(-fixation.LocalPosition.X)); //right
		posArray.Add(JsonValue);
		JsonValue = MakeShareable(new FJsonValueNumber(fixation.LocalPosition.Z)); //up
		posArray.Add(JsonValue);
		JsonValue = MakeShareable(new FJsonValueNumber(fixation.LocalPosition.Y));  //forward
		posArray.Add(JsonValue);

		fixObj->SetArrayField("p", posArray);

		fixObj->SetStringField("objectid", fixation.DynamicObjectId);
	}
	else
	{
		TArray<TSharedPtr<FJsonValue>> posArray;
		TSharedPtr<FJsonValueNumber> JsonValue;
		JsonValue = MakeShareable(new FJsonValueNumber(-fixation.WorldPosition.X)); //right
		posArray.Add(JsonValue);
		JsonValue = MakeShareable(new FJsonValueNumber(fixation.WorldPosition.Z)); //up
		posArray.Add(JsonValue);
		JsonValue = MakeShareable(new FJsonValueNumber(fixation.WorldPosition.Y));  //forward
		posArray.Add(JsonValue);

		fixObj->SetArrayField("p", posArray);
	}

	Fixations.Add(fixObj);
	if (Fixations.Num() > FixationBatchSize)
	{
		SendData();
	}

	if (DebugDisplayFixations)
	{
		if (fixation.IsLocal)
		{
			FVector fixationWorldPos;
			fixationWorldPos = fixation.Transformation.TransformPosition(fixation.LocalPosition);
			DrawDebugSphere(world, fixationWorldPos, 10, 8, FColor::Cyan, false, 5);
		}
		else
			DrawDebugBox(world, fixation.WorldPosition, FVector::OneVector * 15, FColor::Cyan, false, 5);
	}
}

void UFixationRecorder::SendData(bool copyDataToCache)
{
	if (!cog.IsValid() || !cog->HasStartedSession()) { return; }
	if (cog->GetCurrentSceneVersionNumber().Len() == 0) { return; }
	if (!cog->GetCurrentSceneData().IsValid()) { return; }

	if (Fixations.Num() == 0) { return; }

	TSharedPtr<FJsonObject> wholeObj = MakeShareable(new FJsonObject);

	wholeObj->SetStringField("userid", cog->GetUserID());
	wholeObj->SetStringField("sessionid", cog->GetSessionID());
	wholeObj->SetNumberField("timestamp", cog->GetSessionTimestamp());
	wholeObj->SetNumberField("part", jsonFixationPart);
	jsonFixationPart++;
	wholeObj->SetStringField("formatversion", "1.0");

	TArray<TSharedPtr<FJsonValue>> dataArray;
	for (int32 i = 0; i != Fixations.Num(); ++i)
	{
		TSharedPtr<FJsonValueObject> snapshotValue;
		snapshotValue = MakeShareable(new FJsonValueObject(Fixations[i]));
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

void UFixationRecorder::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	SendData();
	instance = NULL;
}

void UFixationRecorder::EndSession()
{
	cog.Reset();
}

float UFixationRecorder::GetDPIScale()
{
	FVector2D viewportSize;
	GEngine->GameViewport->GetViewportSize(viewportSize);

	int32 X = FGenericPlatformMath::FloorToInt(viewportSize.X);
	int32 Y = FGenericPlatformMath::FloorToInt(viewportSize.Y);

	return GetDefault<UUserInterfaceSettings>(UUserInterfaceSettings::StaticClass())->GetDPIScaleBasedOnSize(FIntPoint(X, Y));
}

UFixationRecorder* UFixationRecorder::GetFixationRecorder()
{
	if (instance == NULL)
	{
		for (TObjectIterator<UFixationRecorder> Itr; Itr; ++Itr)
		{
			UWorld* tempWorld = Itr->GetWorld();
			if (tempWorld == NULL) { continue; }
			if (tempWorld->WorldType != EWorldType::PIE && tempWorld->WorldType != EWorldType::Game) { continue; } //editor world. skip
			instance = *Itr;
			instance->world = tempWorld;
			break;
		}
	}
	return instance;
}

FVector2D UFixationRecorder::GetEyePositionScreen()
{
	return CurrentEyePositionScreen;
}

TArray<FFixation> UFixationRecorder::GetRecentFixationPoints()
{
	return recentFixationPoints;
}

TArray<TSharedPtr<FC3DGazePoint>> UFixationRecorder::GetRecentEyePositions()
{
	return recentEyePositions;
}