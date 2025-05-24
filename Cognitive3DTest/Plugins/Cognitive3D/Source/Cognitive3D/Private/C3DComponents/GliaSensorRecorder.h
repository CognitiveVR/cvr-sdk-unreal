/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/

#pragma once

#include "C3DCommonTypes.h"
#include "Components/SceneComponent.h"
#include "Cognitive3D/Public/Cognitive3D.h"
#include "Cognitive3D/Private/C3DUtil/Util.h"
#include "Cognitive3D/Public/DynamicObject.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Runtime/HeadMountedDisplay/Public/IXRTrackingSystem.h"
#include "Widgets/Text/STextBlock.h"

#if defined INCLUDE_HPGLIA_PLUGIN
#include "HPGliaClient.h"
#endif
#include "DrawDebugHelpers.h"
#include "GliaSensorRecorder.generated.h"

class FAnalyticsProviderCognitive3D;
class UCognitive3DBlueprints;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class COGNITIVE3D_API UGliaSensorRecorder : public UActorComponent
{
	GENERATED_BODY()

private:
#if defined INCLUDE_HPGLIA_PLUGIN
	int32 LastHeartRate = -1;
	float LastCognitiveLoad = -1;
	float LastLeftPupilDiamter = -1;
	float LastRightPupilDiamter = -1;
	FTimerHandle AutoSendHandle100MS;
	FTimerHandle AutoSendHandle1000MS;

	void TickSensors1000MS();
	void TickSensors100MS();
#endif

	TSharedPtr<FAnalyticsProviderCognitive3D> cog;
	UFUNCTION()
	void StartListenForTimers();
	UFUNCTION()
	void StopListenForTimers();

public:
	UGliaSensorRecorder();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};
