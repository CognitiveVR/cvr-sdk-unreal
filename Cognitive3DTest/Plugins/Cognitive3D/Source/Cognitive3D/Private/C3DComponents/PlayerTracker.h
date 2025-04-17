/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/

#pragma once

#include "C3DCommonTypes.h"
#include "Components/SceneComponent.h"
#include "Cognitive3D/Public/Cognitive3D.h"
#include "Cognitive3D/Private/C3DUtil/Util.h"
//#include "Engine/SceneCapture2D.h"
//#include "Engine/Texture.h"
//#include "Engine/Texture2D.h"
#include "SceneView.h"
//#include "Engine/TextureRenderTarget2D.h"
#include "Runtime/HeadMountedDisplay/Public/IXRTrackingSystem.h"
#include "Widgets/Text/STextBlock.h"
#if defined INCLUDE_TOBII_PLUGIN
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
#if defined INCLUDE_VARJO_PLUGIN
#include "VarjoEyeTrackerFunctionLibrary.h"
#endif
#if defined INCLUDE_PICOMOBILE_PLUGIN
#include "PicoBlueprintFunctionLibrary.h"
#endif
#if defined INCLUDE_HPGLIA_PLUGIN
#include "HPGliaClient.h"
#endif
#if defined INCLUDE_OCULUS_PLUGIN
#include "Runtime/EyeTracker/Public/IEyeTracker.h"
#include "Runtime/EyeTracker/Public/IEyeTrackerModule.h"
#endif
#if defined OPENXR_EYETRACKING
#include "Runtime/EyeTracker/Public/IEyeTracker.h"
#include "Runtime/EyeTracker/Public/IEyeTrackerModule.h"
#endif
#if defined WAVEVR_EYETRACKING
#include "Public/Eye/WaveVREyeManager.h"
#endif
#include "DrawDebugHelpers.h"
#include "Cognitive3D/Private/C3DApi/GazeDataRecorder.h"
#include "PlayerTracker.generated.h"

class FAnalyticsProviderCognitive3D;
class UCognitive3DBlueprints;
class UDynamicObject;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class COGNITIVE3D_API UPlayerTracker : public UActorComponent
{
	GENERATED_BODY()

private:
	float currentTime = 0;
	TSharedPtr<FAnalyticsProviderCognitive3D> cog;

	FVector GetWorldGazeEnd(FVector start);
	FVector LastDirection;
	TArray<APlayerController*, FDefaultAllocator> controllers;

public:

	const float PlayerSnapshotInterval = 0.1;

	UPlayerTracker();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, Category = "Cognitive3D Analytics")
		bool DebugDisplayGaze = false;

	UPROPERTY(EditAnywhere, Category = "Cognitive3D Analytics")
		bool RecordGazeHit = true;

	float GetLastSendTime();
	int32 GetPartNumber();
	int32 GetDataPoints();
};
