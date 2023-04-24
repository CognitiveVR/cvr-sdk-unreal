#pragma once

#include "CognitiveVR/Private/util/util.h"
#include "CognitiveVR/Public/CognitiveVR.h"
#include "HandElevation.generated.h"

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