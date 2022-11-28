#include "CognitiveVR/Private/FramerateSensor.h"

UFramerateSensor::UFramerateSensor()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UFramerateSensor::BeginPlay()
{
	if (HasBegunPlay()) { return; }
	Super::BeginPlay();
	lastTime = Util::GetTimestamp();
}

void UFramerateSensor::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	aggregateFramerate += 1 / FApp::GetDeltaTime();
	numSamples++;
	if (Util::GetTimestamp() >= lastTime + framerateRecordIntervalInSeconds)
	{
		UFramerateSensor::SendFramerateAsSensor();
		numSamples = 0;
		aggregateFramerate = 0;
		lastTime = Util::GetTimestamp();
	}
}

void UFramerateSensor::SendFramerateAsSensor()
{
	float averageFramerate = aggregateFramerate / numSamples;
	auto cognitive = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	if (cognitive.IsValid() && cognitive->HasStartedSession())
	{
		cognitive->sensors->RecordSensor("FPS", averageFramerate);
	}
}
