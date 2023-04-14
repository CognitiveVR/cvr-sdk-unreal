#include "CognitiveVR/Private/FramerateSensor.h"

UFramerateSensor::UFramerateSensor()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UFramerateSensor::BeginPlay()
{
	if (HasBegunPlay()) { return; }
	Super::BeginPlay();
}

void UFramerateSensor::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	intervalFrameCount++;
	currentTime += DeltaTime;
	deltaTimes.Add(DeltaTime);

	if (currentTime > FramerateTrackingInterval)
	{
		IntervalEnd();
	}
}

void UFramerateSensor::IntervalEnd()
{
	float framesPerSecond = intervalFrameCount / currentTime;

	int32 lowerCount5Percent = FMath::CeilToInt(deltaTimes.Num() * 0.05f);
	int32 lowerCount1Percent = FMath::CeilToInt(deltaTimes.Num() * 0.01f);
	deltaTimes.Sort();
	Algo::Reverse(deltaTimes);

	float lowerTotal5Percent = 0;
	for (int32 i = 0; i < lowerCount5Percent;i++)
	{
		lowerTotal5Percent += deltaTimes[i];
	}
	float lowerTotal1Percent = 0;
	for (int32 i = 0; i < lowerCount1Percent;i++)
	{
		lowerTotal1Percent += deltaTimes[i];
	}

	float min5Percent = lowerTotal5Percent / (float)lowerCount5Percent;
	float min1Percent = lowerTotal1Percent / (float)lowerCount1Percent;
	float finalLow5Percent = 1.0f / min5Percent;
	float finalLow1Percent = 1.0f / min1Percent;
	
	auto cognitive = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	if (cognitive.IsValid() && cognitive->HasStartedSession())
	{
		cognitive->sensors->RecordSensor("c3d.fps.avg", framesPerSecond);
		cognitive->sensors->RecordSensor("c3d.fps.5pl", finalLow5Percent);
		cognitive->sensors->RecordSensor("c3d.fps.1pl", finalLow1Percent);
	}

	intervalFrameCount = 0;
	currentTime = 0;
	deltaTimes.Empty();
}
