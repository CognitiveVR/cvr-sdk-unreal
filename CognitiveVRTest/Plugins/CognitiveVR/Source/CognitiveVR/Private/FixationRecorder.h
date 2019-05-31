// Fill out your copyright notice in the Description page of Project Settings.


#pragma once

#include "CognitiveVR.h"
#include "DynamicObject.h"
#include "Fixations.h"
#include "EyeCapture.h"
#include "SceneView.h"
#if defined TOBII_EYETRACKING_ACTIVE
#include "TobiiTypes.h"
#include "ITobiiCore.h"
#include "ITobiiEyetracker.h"
#endif
#if defined SRANIPAL_API
#include "SRanipal_Eye.h"
#include "ViveSR_Enums.h"
#include "SRanipal_Eyes_Enums.h"
#include "SRanipal_FunctionLibrary_Eye.h"
#endif
#include "FixationRecorder.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class COGNITIVEVR_API UFixationRecorder : public UActorComponent
{
	GENERATED_BODY()

private:
	float currentTime = 0;

	TArray<TSharedPtr<FJsonObject>> Fixations;
	TArray<FEyeCapture> EyeCaptures;

	int32 jsonGazePart = 1;
	int32 index;
	//number of samples to hold on to before registering a fixation
	int32 CachedEyeCaptureCount = 100;
	int32 GetIndex(int32 offset);

	TSharedPtr<FAnalyticsProviderCognitiveVR> cog;

	UWorld* world;

	FFixation ActiveFixation;
	bool IsGazeOutOfRange(FEyeCapture eyeCapture);
	bool IsGazeOffTransform(FEyeCapture eyeCapture);
	bool CheckEndFixation(FFixation testFixation);
	void RecordFixationEnd(FFixation fixation);

#if defined TOBII_EYETRACKING_ACTIVE
	bool AreEyesClosed(TSharedPtr<ITobiiEyeTracker, ESPMode::ThreadSafe> eyetracker);
	int64 GetEyeCaptureTimestamp(TSharedPtr<ITobiiEyeTracker, ESPMode::ThreadSafe> eyetracker);
#elif defined SRANIPAL_API
	bool AreEyesClosed();
	int64 GetEyeCaptureTimestamp();
	ViveSR::anipal::Eye::EyeData* data;
	TArray<APlayerController*, FDefaultAllocator> controllers;
#else
	bool AreEyesClosed();
	int64 GetEyeCaptureTimestamp();
#endif

	bool IsFixating;
	TArray<FVector> CachedEyeCapturePositions;
	UDynamicObject* FixationTransform;

	bool TryBeginLocalFixation();
	bool TryBeginFixation();

	bool eyesClosed;
	int64 EyeUnblinkTime;


	int32 FixationBatchSize = 64;
	int32 jsonFixationPart = 0;
	int32 AutoTimer = 2;
	int32 MinTimer = 2;
	int32 ExtremeBatchSize = 64;
	int32 LastSendTime = -60;
	FTimerHandle AutoSendHandle;


public:

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

	virtual void BeginPlay() override;

	UFixationRecorder();

	void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction);

	void SendData();
};
