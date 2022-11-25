#include "CognitiveVR/Private/FramerateSensor.h"

UFramerateSensor::UFramerateSensor()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UFramerateSensor::BeginPlay()
{
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
	FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin()->sensors->RecordSensor("FPS", averageFramerate);
}
