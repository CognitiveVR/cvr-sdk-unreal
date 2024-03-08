/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/

#pragma once

#include "Cognitive3D/Private/C3DUtil/Util.h"
#include "Cognitive3D/Public/Cognitive3D.h"
#include "FramerateSensor.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UFramerateSensor : public UActorComponent
{
	GENERATED_BODY()

	public:
		UFramerateSensor();

	private:
		virtual void BeginPlay() override;
		virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
		float FramerateTrackingInterval = 1;

		UFUNCTION()
		void OnSessionBegin();
		int32 intervalFrameCount = 0;
		float currentTime = 0;
		TArray<float> deltaTimes;
		void EndInterval();

};