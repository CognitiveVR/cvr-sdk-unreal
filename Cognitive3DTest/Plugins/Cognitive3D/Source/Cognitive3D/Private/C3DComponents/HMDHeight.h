#pragma once

#include "Cognitive3D/Private/C3DUtil/Util.h"
#include "Cognitive3D/Public/Cognitive3D.h"
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

		TArray<float> SampledHeights;
		FTimerHandle IntervalHandle;
		float IntervalDuration = 1;
		int32 NumberOfSamples = 60;
		int32 IntermediateSampleCount = 10;
		float GetMedianHeight();
		//
		EHMDWornState::Type WornState = EHMDWornState::NotWorn;
};