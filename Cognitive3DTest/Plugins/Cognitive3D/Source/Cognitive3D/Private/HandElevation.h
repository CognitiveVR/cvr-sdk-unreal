#pragma once

#include "Cognitive3D/Private/util/util.h"
#include "Cognitive3D/Public/Cognitive3D.h"
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