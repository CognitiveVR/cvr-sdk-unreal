#pragma once

#include <time.h>
#include "CognitiveVR/Public/CognitiveVR.h"
#include "FramerateSensor.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UFramerateSensor : public UActorComponent
{
	GENERATED_BODY()

	public:
		virtual void BeginPlay() override;
		virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
		UFramerateSensor();
		TSharedPtr<FAnalyticsProviderCognitiveVR> provider;
		float aggregateFramerate = 0;
		float numSamples = 0;
		time_t lastTime;
		UPROPERTY(EditAnywhere)
			float framerateRecordIntervalInSeconds = 2;
		void SendFramerateAsSensor();
};