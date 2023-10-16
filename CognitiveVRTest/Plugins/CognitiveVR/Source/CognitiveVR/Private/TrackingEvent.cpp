#include "CognitiveVR/Private/TrackingEvent.h"

UTrackingEvent::UTrackingEvent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UTrackingEvent::BeginPlay()
{
	if (HasBegunPlay()) { return; }
	Super::BeginPlay();

	auto cognitive = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
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
	auto world = ACognitiveVRActor::GetCognitiveSessionWorld();
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
	auto cognitive = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	if (!cognitive.IsValid())
	{
		return;
	}
	if (!cognitive->HasStartedSession())
	{
		return;
	}

	FXRMotionControllerData data;
	UHeadMountedDisplayFunctionLibrary::GetMotionControllerData(GetWorld(), EControllerHand::Right, data);
	if (data.bValid)
	{
		if (IsTrackingRightController && data.TrackingStatus == ETrackingStatus::NotTracked)
		{
			//send an event
			TWeakPtr<FAnalyticsProviderCognitiveVR> provider = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider();
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

	UHeadMountedDisplayFunctionLibrary::GetMotionControllerData(GetWorld(), EControllerHand::Left, data);
	if (data.bValid)
	{
		if (IsTrackingLeftController && data.TrackingStatus == ETrackingStatus::NotTracked)
		{
			//send an event
			TWeakPtr<FAnalyticsProviderCognitiveVR> provider = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider();
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
