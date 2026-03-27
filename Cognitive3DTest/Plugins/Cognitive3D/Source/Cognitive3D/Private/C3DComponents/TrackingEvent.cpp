/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/

#include "Cognitive3D/Private/C3DComponents/TrackingEvent.h"
#include "Cognitive3D/Public/Cognitive3DProvider.h"
#include "Cognitive3D/Public/Cognitive3D.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Cognitive3D/Private/C3DApi/CustomEventRecorder.h"
#include "Cognitive3D/Public/DynamicObject.h"
#include "Cognitive3D/Public/Cognitive3DActor.h"
#include "Engine/World.h"
#include "TimerManager.h"

UTrackingEvent::UTrackingEvent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UTrackingEvent::BeginPlay()
{
	if (HasBegunPlay()) { return; }
	Super::BeginPlay();

	auto cognitive = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (cognitive.IsValid())
	{
		cognitive->OnSessionBegin.AddDynamic(this, &UTrackingEvent::OnSessionBegin);
		if (cognitive->HasStartedSession())
		{
			OnSessionBegin();
			
		}
	}
}

void UTrackingEvent::OnSessionBegin()
{
	auto world = ACognitive3DActor::GetCognitiveSessionWorld();
	if (world == nullptr) { return; }
	float DelaySeconds = 1.0f;
	bCanTick = false;
	world->GetTimerManager().SetTimer(IntervalHandle, this, &UTrackingEvent::EnableTick, DelaySeconds, false);
}


void UTrackingEvent::EnableTick()
{
	bCanTick = true;
}

void UTrackingEvent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	if (!bCanTick)
	{
		return;
	}
	auto cognitive = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (!cognitive.IsValid())
	{
		return;
	}
	if (!cognitive->HasStartedSession())
	{
		return;
	}

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 7
	FXRMotionControllerState data;
	EXRControllerPoseType poseType = EXRControllerPoseType::Grip;
	EXRSpaceType spaceType = EXRSpaceType::UnrealWorldSpace;
	UHeadMountedDisplayFunctionLibrary::GetMotionControllerState(GetWorld(), spaceType, EControllerHand::Right, poseType, data);
#else
	FXRMotionControllerData data;
	UHeadMountedDisplayFunctionLibrary::GetMotionControllerData(GetWorld(), EControllerHand::Right, data);
#endif
	if (data.bValid)
	{
		if (IsTrackingRightController && data.TrackingStatus == ETrackingStatus::NotTracked)
		{
			//send an event
			TWeakPtr<FAnalyticsProviderCognitive3D> provider = FAnalyticsCognitive3D::Get().GetCognitive3DProvider();
			if (provider.IsValid())
			{
				TWeakObjectPtr<UDynamicObject> object = cognitive->GetControllerDynamic(false);
				if (object != nullptr)
				{
					FVector controllerPosition = object.Get()->GetComponentLocation();
					cognitive->customEventRecorder->Send("c3d.Right Controller Lost tracking",controllerPosition);
				}
				else
				{
					cognitive->customEventRecorder->Send("c3d.Right Controller Lost tracking");
				}
			}
			IsTrackingRightController = false;
		}
		else if (!IsTrackingRightController && data.TrackingStatus != ETrackingStatus::NotTracked)
		{
			//reset internal tracking state
			IsTrackingRightController = true;
		}
	}

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 7
	//data, spaceType and poseType reused from above
	UHeadMountedDisplayFunctionLibrary::GetMotionControllerState(GetWorld(), spaceType, EControllerHand::Left, poseType, data);
#else
	UHeadMountedDisplayFunctionLibrary::GetMotionControllerData(GetWorld(), EControllerHand::Left, data);
#endif
	if (data.bValid)
	{
		if (IsTrackingLeftController && data.TrackingStatus == ETrackingStatus::NotTracked)
		{
			//send an event
			TWeakPtr<FAnalyticsProviderCognitive3D> provider = FAnalyticsCognitive3D::Get().GetCognitive3DProvider();
			if (provider.IsValid())
			{
				TWeakObjectPtr<UDynamicObject> object = cognitive->GetControllerDynamic(false);
				if (object != nullptr)
				{
					FVector controllerPosition = object.Get()->GetComponentLocation();
					cognitive->customEventRecorder->Send("c3d.Left Controller Lost tracking", controllerPosition);
				}
				else
				{
					cognitive->customEventRecorder->Send("c3d.Left Controller Lost tracking");
				}
			}
			IsTrackingLeftController = false;
		}
		else if (!IsTrackingLeftController && data.TrackingStatus != ETrackingStatus::NotTracked)
		{
			//reset internal tracking state
			IsTrackingLeftController = true;
		}
	}
}
