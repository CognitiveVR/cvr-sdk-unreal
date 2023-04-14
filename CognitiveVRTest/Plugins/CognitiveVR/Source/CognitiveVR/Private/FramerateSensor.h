#pragma once

#include "CognitiveVR/Private/util/util.h"
#include "CognitiveVR/Public/CognitiveVR.h"
#include "FramerateSensor.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UFramerateSensor : public UActorComponent
{
	GENERATED_BODY()

	public:
		UFramerateSensor();

	private:
		virtual void BeginPlay() override;
		virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
		TSharedPtr<FAnalyticsProviderCognitiveVR> provider;
		
		float FramerateTrackingInterval = 1;

		//the number of frames in the interval
		int32 intervalFrameCount = 0;
		float currentTime = 0;
		TArray<float> deltaTimes;
		void IntervalEnd();

};