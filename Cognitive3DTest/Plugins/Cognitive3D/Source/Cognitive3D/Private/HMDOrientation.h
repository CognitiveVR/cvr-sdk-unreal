#pragma once

#include "Cognitive3D/Private/util/util.h"
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
		float IntervalDuration = 0.1f;
		void RecordYaw();
		void RecordPitch();

};