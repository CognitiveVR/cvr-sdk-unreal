/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/


#pragma once

#include "Cognitive3D/Public/Cognitive3D.h"
#include "Kismet/GameplayStatics.h"
#include "Cognitive3D/Public/Cognitive3DActor.h"
#include "Cognitive3D/Public/DynamicObject.h"
#include "Cognitive3D/Private/Fixations.h"
#include "Cognitive3D/Private/EyeCapture.h"
#include "SceneView.h"
#include "Engine/LocalPlayer.h"
#include "DrawDebugHelpers.h"
#if defined TOBII_EYETRACKING_ACTIVE
#include "TobiiTypes.h"
#include "ITobiiCore.h"
#include "ITobiiEyetracker.h"
#endif
#if defined SRANIPAL_1_2_API
#include "SRanipal_Eye.h"
#include "ViveSR_Enums.h"
#include "SRanipal_Eyes_Enums.h"
#include "SRanipal_FunctionLibrary_Eye.h"
#endif
#if defined SRANIPAL_1_3_API
#include "SRanipalEye.h"
#include "ViveSR_Enums.h"
#include "SRanipalEye_FunctionLibrary.h"
#endif
#if defined VARJOEYETRACKER_API
#include "VarjoEyeTrackerFunctionLibrary.h"
#endif
#if defined PICOMOBILE_API
#include "PicoBlueprintFunctionLibrary.h"
#endif
#if defined HPGLIA_API
#include "HPGliaClient.h"
#endif
#if defined OPENXR_EYETRACKING
#include "Runtime/EyeTracker/Public/IEyeTracker.h"
#include "Runtime/EyeTracker/Public/IEyeTrackerModule.h"
#endif
#include "Runtime/Engine/Classes/Engine/UserInterfaceSettings.h" //for getting ui dpi for active session view
#include "DrawDebugHelpers.h"
#include "FixationRecorder.generated.h"

class UCognitive3DBlueprints;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class COGNITIVE3D_API UFixationRecorder : public UActorComponent
{
	GENERATED_BODY()

private:
	float currentTime = 0;
	TArray<FEyeCapture> EyeCaptures;

	int32 index;
	//number of samples to hold on to before registering a fixation
	int32 CachedEyeCaptureCount = 100;
	int32 GetIndex(int32 offset);

	TSharedPtr<IAnalyticsProviderCognitive3D> cog;

	bool WasCaptureDiscardedLastFrame = false;
	bool WasOutOfDispersionLastFrame = false;

	FFixation ActiveFixation;
	bool IsGazeOutOfRange(FEyeCapture eyeCapture);
	bool IsGazeOffTransform(const FEyeCapture& eyeCapture);
	bool CheckEndFixation(const FFixation& testFixation);

	TArray<APlayerController*, FDefaultAllocator> controllers;

#if defined TOBII_EYETRACKING_ACTIVE
	bool AreEyesClosed(TSharedPtr<ITobiiEyeTracker, ESPMode::ThreadSafe> eyetracker);
	int64 GetEyeCaptureTimestamp(TSharedPtr<ITobiiEyeTracker, ESPMode::ThreadSafe> eyetracker);
#elif defined OPENXR_EYETRACKING
	bool AreEyesClosed();
	int64 GetEyeCaptureTimestamp();
#elif defined SRANIPAL_1_2_API
	bool AreEyesClosed();
	int64 GetEyeCaptureTimestamp();
#elif defined SRANIPAL_1_3_API
	bool AreEyesClosed();
	int64 GetEyeCaptureTimestamp();
#elif defined VARJOEYETRACKER_API
	bool AreEyesClosed();
	int64 GetEyeCaptureTimestamp();
#elif defined PICOMOBILE_API
	bool AreEyesClosed();
	int64 GetEyeCaptureTimestamp();
#elif defined HPGLIA_API
	bool AreEyesClosed();
	int64 GetEyeCaptureTimestamp();
#elif defined WAVEVR_EYETRACKING
	bool AreEyesClosed();
	int64 GetEyeCaptureTimestamp();
#else
	bool AreEyesClosed();
	int64 GetEyeCaptureTimestamp();
#endif

	bool hasEyeTrackingSDK = true;

	bool isFixating;
	TArray<FVector> CachedEyeCapturePositions;

	bool TryBeginLocalFixation();
	bool TryBeginFixation();

	bool eyesClosed;
	int64 EyeUnblinkTime;

	FVector2D CurrentEyePositionScreen;

	TArray<FFixation> recentFixationPoints;
	TArray<TSharedPtr<FC3DGazePoint>> recentEyePositions;

	UFUNCTION()
		void BeginSession();
	UFUNCTION()
		void OnPreSessionEnd();

public:

	UPROPERTY(EditAnywhere, Category = "Cognitive3D Analytics")
		float MaxFixationDistance = 10000;

	//configurable fixation variables
	/* the time that gaze must be within the max fixation angle before a fixation occurs */
	UPROPERTY(EditAnywhere, Category = "Cognitive3D Analytics")
		int32 MinFixationMs = 60;
	/* the angle that a number of gaze samples must fall within to start a fixation event */
	UPROPERTY(EditAnywhere, Category = "Cognitive3D Analytics")
		float MaxFixationAngle = 1;

	/* the maximum amount of time that can be assigned as a single 'blink'. if eyes are closed for longer than this, assume that the user is conciously closing their eyes */
	UPROPERTY(EditAnywhere, Category = "Cognitive3D Analytics")
		int32 MaxBlinkMs = 400;
	/* when a blink occurs, ignore gaze preceding the blink up to this far back in time */
	UPROPERTY(EditAnywhere, Category = "Cognitive3D Analytics")
		int32 PreBlinkDiscardMs = 20;
	/* after a blink has ended, ignore gaze up to this long afterwards */
	UPROPERTY(EditAnywhere, Category = "Cognitive3D Analytics")
		int32 BlinkEndWarmupMs = 100;

	/* amount of saccades that must be consecutive before a fixation ends */
	UPROPERTY(EditAnywhere, Category = "Cognitive3D Analytics")
		int32 SaccadeFixationEndMs = 10;
	/* the amount of time gaze can be discarded before a fixation is ended. gaze can be discarded if eye tracking values are outside of expected ranges */
	UPROPERTY(EditAnywhere, Category = "Cognitive3D Analytics")
		int32 MaxConsecutiveDiscardMs = 10;
	/* amount of time gaze can be off the transform before fixation ends. mostly useful when fixation is right on the edge of a dynamic object */
	UPROPERTY(EditAnywhere, Category = "Cognitive3D Analytics")
		int32 MaxConsecutiveOffDynamicMs = 500;

	/*increases the size of the fixation angle as gaze gets toward the edge of the viewport. this is used to reduce the number of incorrectly ended fixations because of hardware limits at the edge of the eye tracking field of view*/
	UPROPERTY(EditAnywhere, Category = "Cognitive3D Analytics")
		UCurveFloat* FocusSizeFromCenter;

	UPROPERTY(EditAnywhere, Category = "Cognitive3D Analytics")
		bool DebugDisplayFixations = false;

	UFixationRecorder();
	virtual void BeginPlay() override;
	void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction);
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;


	UFUNCTION(BlueprintCallable, Category = "Cognitive3D Analytics")
		FVector2D GetEyePositionScreen();

	UFUNCTION(BlueprintPure, Category = "Cognitive3D Analytics")
		static float GetDPIScale();

	//returns the last 50 fixations in x,y,z world space, with w as the radius of the fixation
	TArray<FFixation> GetRecentFixationPoints();

	//returns the last 50 eye positions in x,y,z world space, to be used for drawing saccade lines on screen space
	TArray<TSharedPtr<FC3DGazePoint>> GetRecentEyePositions();

	bool IsFixating() { return isFixating; }

	float GetLastSendTime();
	int32 GetPartNumber();
	int32 GetDataPoints();

};
