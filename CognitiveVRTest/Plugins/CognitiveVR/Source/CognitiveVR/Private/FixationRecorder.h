// Fill out your copyright notice in the Description page of Project Settings.


#pragma once

#include "CognitiveVR/Public/CognitiveVR.h"
#include "Kismet/GameplayStatics.h"
#include "CognitiveVR/Public/DynamicObject.h"
#include "CognitiveVR/Private/Fixations.h"
#include "CognitiveVR/Private/EyeCapture.h"
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

class UCognitiveVRBlueprints;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class COGNITIVEVR_API UFixationRecorder : public UActorComponent
{
	GENERATED_BODY()

private:
	float currentTime = 0;

	TArray<TSharedPtr<FJsonObject>> Fixations;
	TArray<FEyeCapture> EyeCaptures;

	int32 index;
	//number of samples to hold on to before registering a fixation
	int32 CachedEyeCaptureCount = 100;
	int32 GetIndex(int32 offset);

	TSharedPtr<FAnalyticsProviderCognitiveVR> cog;

	UWorld* world;
	static UFixationRecorder* instance;

	bool WasCaptureDiscardedLastFrame = false;
	bool WasOutOfDispersionLastFrame = false;

	FFixation ActiveFixation;
	bool IsGazeOutOfRange(FEyeCapture eyeCapture);
	bool IsGazeOffTransform(const FEyeCapture& eyeCapture);
	bool CheckEndFixation(const FFixation& testFixation);
	void RecordFixationEnd(const FFixation& fixation);

#if defined TOBII_EYETRACKING_ACTIVE
	bool AreEyesClosed(TSharedPtr<ITobiiEyeTracker, ESPMode::ThreadSafe> eyetracker);
	int64 GetEyeCaptureTimestamp(TSharedPtr<ITobiiEyeTracker, ESPMode::ThreadSafe> eyetracker);
#elif defined OPENXR_EYETRACKING
	IEyeTrackerModule& eyeTrackingModule = IEyeTrackerModule::Get();
	TSharedPtr< class IEyeTracker, ESPMode::ThreadSafe > eyeTracker;
	bool AreEyesClosed();
	int64 GetEyeCaptureTimestamp();
#elif defined SRANIPAL_1_2_API
	bool AreEyesClosed();
	int64 GetEyeCaptureTimestamp();
	TArray<APlayerController*, FDefaultAllocator> controllers;
#elif defined SRANIPAL_1_3_API
	bool AreEyesClosed();
	int64 GetEyeCaptureTimestamp();
	TArray<APlayerController*, FDefaultAllocator> controllers;
#elif defined VARJOEYETRACKER_API
	bool AreEyesClosed();
	int64 GetEyeCaptureTimestamp();
#elif defined PICOMOBILE_API
	bool AreEyesClosed();
	int64 GetEyeCaptureTimestamp();
#elif defined HPGLIA_API
	bool AreEyesClosed();
	int64 GetEyeCaptureTimestamp();
	TArray<APlayerController*, FDefaultAllocator> controllers;
#else
	bool AreEyesClosed();
	int64 GetEyeCaptureTimestamp();
#endif

	bool isFixating;
	TArray<FVector> CachedEyeCapturePositions;

	bool TryBeginLocalFixation();
	bool TryBeginFixation();

	bool eyesClosed;
	int64 EyeUnblinkTime;


	int32 FixationBatchSize = 64;
	int32 jsonFixationPart = 0;
	int32 AutoTimer = 2;
	int32 MinTimer = 2;
	int32 ExtremeBatchSize = 64;
	float LastSendTime = -60;
	FTimerHandle AutoSendHandle;
	FVector2D CurrentEyePositionScreen;

	TArray<FFixation> recentFixationPoints;
	TArray<TSharedPtr<FC3DGazePoint>> recentEyePositions;

public:

	UPROPERTY(EditAnywhere)
		float MaxFixationDistance = 10000;

	//configurable fixation variables
	/* the time that gaze must be within the max fixation angle before a fixation occurs */
	UPROPERTY(EditAnywhere)
		int32 MinFixationMs = 60;
	/* the angle that a number of gaze samples must fall within to start a fixation event */
	UPROPERTY(EditAnywhere)
		float MaxFixationAngle = 1;

	/* the maximum amount of time that can be assigned as a single 'blink'. if eyes are closed for longer than this, assume that the user is conciously closing their eyes */
	UPROPERTY(EditAnywhere)
		int32 MaxBlinkMs = 400;
	/* when a blink occurs, ignore gaze preceding the blink up to this far back in time */
	UPROPERTY(EditAnywhere)
		int32 PreBlinkDiscardMs = 20;
	/* after a blink has ended, ignore gaze up to this long afterwards */
	UPROPERTY(EditAnywhere)
		int32 BlinkEndWarmupMs = 100;

	/* amount of saccades that must be consecutive before a fixation ends */
	UPROPERTY(EditAnywhere)
		int32 SaccadeFixationEndMs = 10;
	/* the amount of time gaze can be discarded before a fixation is ended. gaze can be discarded if eye tracking values are outside of expected ranges */
	UPROPERTY(EditAnywhere)
		int32 MaxConsecutiveDiscardMs = 10;
	/* amount of time gaze can be off the transform before fixation ends. mostly useful when fixation is right on the edge of a dynamic object */
	UPROPERTY(EditAnywhere)
		int32 MaxConsecutiveOffDynamicMs = 500;

	/*increases the size of the fixation angle as gaze gets toward the edge of the viewport. this is used to reduce the number of incorrectly ended fixations because of hardware limits at the edge of the eye tracking field of view*/
	UPROPERTY(EditAnywhere)
		UCurveFloat* FocusSizeFromCenter;

	UPROPERTY(EditAnywhere)
		bool DebugDisplayFixations = false;

	virtual void BeginPlay() override;
	void BeginSession();

	UFixationRecorder();

	void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction);

	void SendData();

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	void EndSession();

	UFUNCTION(BlueprintPure, Category = "CognitiveVR Analytics")
		static UFixationRecorder* GetFixationRecorder();

	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics")
		FVector2D GetEyePositionScreen();

	UFUNCTION(BlueprintPure, Category = "CognitiveVR Analytics")
		static float GetDPIScale();

	//returns the last 50 fixations in x,y,z world space, with w as the radius of the fixation
	TArray<FFixation> GetRecentFixationPoints();

	//returns the last 50 eye positions in x,y,z world space, to be used for drawing saccade lines on screen space
	TArray<TSharedPtr<FC3DGazePoint>> GetRecentEyePositions();

	float GetLastSendTime() { return LastSendTime; }
	int32 GetPartNumber() { return jsonFixationPart; }
	bool IsFixating() { return isFixating; }
	int32 GetDataPoints() { return Fixations.Num(); }
};
