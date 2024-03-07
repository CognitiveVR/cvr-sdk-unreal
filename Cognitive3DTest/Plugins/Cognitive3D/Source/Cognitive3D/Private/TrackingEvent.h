#pragma once

#include "Cognitive3D/Private/C3DUtil/Util.h"
#include "Cognitive3D/Public/Cognitive3D.h"
#include "TrackingEvent.generated.h"

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