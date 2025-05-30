/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/


#include "HMDEvents.h"
#include "DrawDebugHelpers.h"
#include "Interfaces/IPluginManager.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Cognitive3D/Private/C3DApi/CustomEventRecorder.h"
#include "Cognitive3D/Public/Cognitive3DActor.h"

// Sets default values for this component's properties
UHMDEvents::UHMDEvents()
{
	PrimaryComponentTick.bCanEverTick = false;
}


// Called when the game starts
void UHMDEvents::BeginPlay()
{
	if (HasBegunPlay()) { return; }
	Super::BeginPlay();

	auto cognitive = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (cognitive.IsValid())
	{
		cognitive->OnSessionBegin.AddDynamic(this, &UHMDEvents::OnSessionBegin);
		cognitive->OnPreSessionEnd.AddDynamic(this, &UHMDEvents::OnSessionEnd);
		if (cognitive->HasStartedSession())
		{
			OnSessionBegin();
		}
	}
}


void UHMDEvents::OnSessionBegin()
{
	auto world = ACognitive3DActor::GetCognitiveSessionWorld();
	if (world == nullptr) { return; }
	world->GetTimerManager().SetTimer(IntervalHandle, FTimerDelegate::CreateUObject(this, &UHMDEvents::EndInterval), IntervalDuration, true);
}

void UHMDEvents::EndInterval()
{
	auto cognitive = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (cognitive.IsValid())
	{
		EHMDWornState::Type currentWornState;
		cognitive->TryGetHMDWornState(currentWornState);

		if (currentWornState != WornState)
		{
			if (currentWornState == EHMDWornState::Worn)
			{
				cognitive->customEventRecorder->Send("c3d.User equipped headset");
			}
			else if (currentWornState == EHMDWornState::NotWorn)
			{
				cognitive->customEventRecorder->Send("c3d.User removed headset");
			}
			else if (currentWornState == EHMDWornState::Unknown)
			{
				cognitive->customEventRecorder->Send("c3d.HMD State Unknown");
			}
			WornState = currentWornState;
		}
	}
}

void UHMDEvents::OnSessionEnd()
{
	auto world = ACognitive3DActor::GetCognitiveSessionWorld();
	if (world == nullptr) { return; }
	world->GetTimerManager().ClearTimer(IntervalHandle);
}

