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
		UPROPERTY(EditAnywhere)
			float framerateRecordIntervalInSeconds = 2;

	private:
		virtual void BeginPlay() override;
		virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
		TSharedPtr<FAnalyticsProviderCognitiveVR> provider;
		float aggregateFramerate = 0;
		float numSamples = 0;
		double lastTime;
		void SendFramerateAsSensor();

};