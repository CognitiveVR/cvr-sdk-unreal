#include "CognitiveVR/Private/FramerateSensor.h"

UFramerateSensor::UFramerateSensor()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UFramerateSensor::BeginPlay()
{
	lastTime = time(NULL);
}

void UFramerateSensor::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	aggregateFramerate += 1 / FApp::GetDeltaTime();
	numSamples++;
	if (time(NULL) >= lastTime + framerateRecordIntervalInSeconds)
	{
		UFramerateSensor::SendFramerateAsSensor();
		numSamples = 0;
		aggregateFramerate = 0;
		lastTime = time(NULL);
	}
}

void UFramerateSensor::SendFramerateAsSensor()
{
	provider = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	float averageFramerate = aggregateFramerate / numSamples;
	provider->sensors->RecordSensor("FPS", averageFramerate);
}
