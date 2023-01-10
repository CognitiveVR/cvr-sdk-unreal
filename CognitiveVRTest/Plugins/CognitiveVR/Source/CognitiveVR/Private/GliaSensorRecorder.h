// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "C3DCommonTypes.h"
#include "Components/SceneComponent.h"
#include "CognitiveVR/Public/CognitiveVR.h"
#include "CognitiveVR/Private/util/util.h"
#include "CognitiveVR/Public/DynamicObject.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Runtime/HeadMountedDisplay/Public/IXRTrackingSystem.h"
#include "Widgets/Text/STextBlock.h"

#if defined HPGLIA_API
#include "HPGliaClient.h"
#endif
#include "DrawDebugHelpers.h"
#include "GliaSensorRecorder.generated.h"

class FAnalyticsProviderCognitiveVR;
class UCognitiveVRBlueprints;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class COGNITIVEVR_API UGliaSensorRecorder : public UActorComponent
{
	GENERATED_BODY()

private:
#if defined HPGLIA_API
	int32 LastHeartRate = -1;
	float LastCognitiveLoad = -1;
	float LastLeftPupilDiamter = -1;
	float LastRightPupilDiamter = -1;
	FTimerHandle AutoSendHandle100MS;
	FTimerHandle AutoSendHandle1000MS;

	void TickSensors1000MS();
	void TickSensors100MS();
#endif

	TSharedPtr<FAnalyticsProviderCognitiveVR> cog;
	UFUNCTION()
	void StartListenForTimers();
	UFUNCTION()
	void StopListenForTimers();

public:
	UGliaSensorRecorder();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};
