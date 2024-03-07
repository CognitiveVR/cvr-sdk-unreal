#include "Cognitive3D/Private/HMDHeight.h"
#include "HeadMountedDisplayTypes.h"

UHMDHeight::UHMDHeight()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UHMDHeight::BeginPlay()
{
	if (HasBegunPlay()) { return; }
	Super::BeginPlay();

	auto cognitive = IAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (cognitive.IsValid())
	{
		cognitive->OnSessionBegin.AddDynamic(this, &UHMDHeight::OnSessionBegin);
		cognitive->OnPreSessionEnd.AddDynamic(this, &UHMDHeight::OnSessionEnd);
		if (cognitive->HasStartedSession())
		{
			OnSessionBegin();
		}
	}
}

void UHMDHeight::OnSessionBegin()
{
	auto world = ACognitive3DActor::GetCognitiveSessionWorld();
	if (world == nullptr) { return; }
	world->GetTimerManager().SetTimer(IntervalHandle, FTimerDelegate::CreateUObject(this, &UHMDHeight::EndInterval), IntervalDuration, true);
	SampledHeights.Empty();
}

void UHMDHeight::EndInterval()
{
	auto cognitive = IAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (cognitive.IsValid())
	{
		//get hmd height
		FVector hmdpos;
		if (cognitive->TryGetPlayerHMDPosition(hmdpos))
		{
			SampledHeights.Add(hmdpos.Z);

			//record median hmd height while still collecting samples
			if (SampledHeights.Num() % IntermediateSampleCount == 0)
			{
				cognitive->SetParticipantProperty("hmdHeight", GetMedianHeight());
			}

			//record a final median hmd height
			if (SampledHeights.Num() > NumberOfSamples)
			{
				cognitive->SetParticipantProperty("hmdHeight", GetMedianHeight());
				
				//when complete, clear timer
				auto world = ACognitive3DActor::GetCognitiveSessionWorld();
				if (world == nullptr) { return; }
				world->GetTimerManager().ClearTimer(IntervalHandle);
			}
		}
	}
}

float UHMDHeight::GetMedianHeight()
{
	int32 index = SampledHeights.Num() / 2;
	SampledHeights.Sort();
	return SampledHeights[index];
}

void UHMDHeight::OnSessionEnd()
{
	auto world = ACognitive3DActor::GetCognitiveSessionWorld();
	if (world == nullptr) { return; }
	world->GetTimerManager().ClearTimer(IntervalHandle);
}

void UHMDHeight::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	auto cognitive = IAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (cognitive.IsValid())
	{
		cognitive->OnSessionBegin.RemoveDynamic(this, &UHMDHeight::OnSessionBegin);
		cognitive->OnPreSessionEnd.RemoveDynamic(this, &UHMDHeight::OnSessionEnd);
		if (cognitive->HasStartedSession())
		{
			OnSessionEnd();
		}
	}
}