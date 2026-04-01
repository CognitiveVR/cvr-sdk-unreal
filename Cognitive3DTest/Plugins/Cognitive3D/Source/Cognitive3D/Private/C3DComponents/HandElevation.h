/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/

#pragma once

#include "Cognitive3D/Private/C3DUtil/Util.h"
#include "Components/ActorComponent.h"
#include "TimerManager.h"
#include "Engine/EngineTypes.h"
#include "HandElevation.generated.h"

class FAnalyticsProviderCognitive3D;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UHandElevation : public UActorComponent
{
	GENERATED_BODY()

	public:
		UHandElevation();

	private:
		float IntervalDuration = 1;

		virtual void BeginPlay() override;
		UFUNCTION()
		void OnSessionBegin();
		UFUNCTION()
		void OnSessionEnd();

		FTimerHandle IntervalHandle;
		void EndInterval();

		virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

};
