#pragma once

#include "Cognitive3D/Private/util/util.h"
#include "Cognitive3D/Public/Cognitive3D.h"
#if PLATFORM_ANDROID
#include "Android/AndroidPlatformMisc.h"
#endif
#include "BatteryLevel.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UBatteryLevel : public UActorComponent
{
	GENERATED_BODY()

	public:
		UBatteryLevel();

	private:
		virtual void BeginPlay() override;
		float Interval = 1;
		FTimerHandle IntervalHandle;
		void EndInterval();

		UFUNCTION()
		void OnSessionBegin();
		UFUNCTION()
		void OnSessionEnd();
		virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

};