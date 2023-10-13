#pragma once

#include "CognitiveVR/Private/util/util.h"
#include "CognitiveVR/Public/CognitiveVR.h"
#include "TrackingEvent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UTrackingEvent : public UActorComponent
{
	GENERATED_BODY()

	public:
		UTrackingEvent();

	private:
		virtual void BeginPlay() override;
		void OnSessionBegin();
		void EnableTick();
		virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
		FTimerHandle IntervalHandle;

		bool IsTrackingLeftController = true;
		bool IsTrackingRightController = true;
};