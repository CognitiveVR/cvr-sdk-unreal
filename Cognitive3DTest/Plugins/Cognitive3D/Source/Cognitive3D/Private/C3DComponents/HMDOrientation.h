/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/

#pragma once

#include "Cognitive3D/Private/C3DUtil/Util.h"
#include "Cognitive3D/Public/Cognitive3D.h"
#include "HMDOrientation.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UHMDOrientation : public UActorComponent
{
	GENERATED_BODY()

	public:
		UHMDOrientation();

	private:
		virtual void BeginPlay() override;
		UFUNCTION()
		void OnSessionBegin();
		UFUNCTION()
		void OnSessionEnd();
		FTimerHandle IntervalHandle;
		void EndInterval();
		virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
		float IntervalDuration = 1f;
		void RecordYaw();
		void RecordPitch();

};