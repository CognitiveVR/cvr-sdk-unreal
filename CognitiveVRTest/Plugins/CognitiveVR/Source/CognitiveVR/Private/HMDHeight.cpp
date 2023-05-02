#include "CognitiveVR/Private/HMDHeight.h"

UHMDHeight::UHMDHeight()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UHMDHeight::BeginPlay()
{
	if (HasBegunPlay()) { return; }
	Super::BeginPlay();

	auto cognitive = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
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
	auto world = ACognitiveVRActor::GetCognitiveSessionWorld();
	if (world == nullptr) { return; }
	world->GetTimerManager().SetTimer(IntervalHandle, FTimerDelegate::CreateUObject(this, &UHMDHeight::EndInterval), IntervalDuration, true);
	CurrentSampleCount = 0;
	HMDHeight = 0;
}

void UHMDHeight::EndInterval()
{
	auto cognitive = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	if (cognitive.IsValid())
	{
		//get hmd height
		auto hmdpos = cognitive->GetPlayerHMDPosition();
		HMDHeight = FMath::Max(HMDHeight,hmdpos.Z);
		
		//TODO record average or median hmd height instead of maximum

		CurrentSampleCount++;
		if (CurrentSampleCount > NumberOfSamples)
		{
			cognitive->OnSessionBegin.RemoveDynamic(this, &UHMDHeight::OnSessionBegin);
			cognitive->SetSessionProperty("c3d.hmdHeight", HMDHeight);
		}
	}
}

void UHMDHeight::OnSessionEnd()
{
	auto world = ACognitiveVRActor::GetCognitiveSessionWorld();
	if (world == nullptr) { return; }
	world->GetTimerManager().ClearTimer(IntervalHandle);
}

void UHMDHeight::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	auto cognitive = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
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