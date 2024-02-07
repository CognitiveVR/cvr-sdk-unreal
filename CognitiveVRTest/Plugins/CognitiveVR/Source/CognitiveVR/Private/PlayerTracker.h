// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "C3DCommonTypes.h"
#include "Components/SceneComponent.h"
#include "CognitiveVR/Public/CognitiveVR.h"
#include "CognitiveVR/Private/util/util.h"
//#include "Engine/SceneCapture2D.h"
//#include "Engine/Texture.h"
//#include "Engine/Texture2D.h"
#include "SceneView.h"
//#include "Engine/TextureRenderTarget2D.h"
#include "CognitiveVR/Public/DynamicObject.h"
#include "Runtime/HeadMountedDisplay/Public/IXRTrackingSystem.h"
#include "Widgets/Text/STextBlock.h"
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
#include "SRanipalEye_Core.h"
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
#if defined WAVEVR_EYETRACKING
#include "Public/Eye/WaveVREyeManager.h"
#endif
#include "DrawDebugHelpers.h"
#include "CognitiveVR/Private/api/GazeDataRecorder.h"
#include "PlayerTracker.generated.h"

class FAnalyticsProviderCognitiveVR;
class UCognitiveVRBlueprints;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class COGNITIVEVR_API UPlayerTracker : public UActorComponent
{
	GENERATED_BODY()

private:
	float currentTime = 0;
	TSharedPtr<FAnalyticsProviderCognitiveVR> cog;

	FVector GetWorldGazeEnd(FVector start);
	FVector LastDirection;
	TArray<APlayerController*, FDefaultAllocator> controllers;

public:

	const float PlayerSnapshotInterval = 0.1;

	UPlayerTracker();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, Category = "CognitiveVR Analytics")
		bool DebugDisplayGaze = false;


	float GetLastSendTime();
	int32 GetPartNumber();
	int32 GetDataPoints();
};
