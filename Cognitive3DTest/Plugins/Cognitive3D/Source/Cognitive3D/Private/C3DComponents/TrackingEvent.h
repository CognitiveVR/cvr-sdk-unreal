/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/

#pragma once

#include "Cognitive3D/Private/C3DUtil/Util.h"
#include "TrackingEvent.generated.h"

class FAnalyticsProviderCognitive3D;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UTrackingEvent : public UActorComponent
{
	GENERATED_BODY()

	public:
		UTrackingEvent();

	private:
		virtual void BeginPlay() override;

		UFUNCTION()
		void OnSessionBegin();

		UFUNCTION()
		void EnableTick();

		virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		UPROPERTY()
		FTimerHandle IntervalHandle;

		UPROPERTY()
		bool bCanTick;

		bool IsTrackingLeftController = true;
		bool IsTrackingRightController = true;
};