#pragma once

#include "CognitiveVR/Private/util/util.h"
#include "CognitiveVR/Public/CognitiveVR.h"
#include "HMDHeight.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UHMDHeight : public UActorComponent
{
	GENERATED_BODY()

	public:
		UHMDHeight();

	private:
		virtual void BeginPlay() override;
		virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

		UFUNCTION()
		void OnSessionBegin();
		UFUNCTION()
		void OnSessionEnd();

		void EndInterval();

		FTimerHandle IntervalHandle;
		float IntervalDuration = 1;
		int32 NumberOfSamples = 60;
		int32 CurrentSampleCount = 0;
		float HMDHeight = 0;
};