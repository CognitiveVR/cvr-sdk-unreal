#include "Cognitive3D/Private/C3DComponents/HandElevation.h"

UHandElevation::UHandElevation()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UHandElevation::BeginPlay()
{
	if (HasBegunPlay()) { return; }
	Super::BeginPlay();

	auto cognitive = IAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (cognitive.IsValid())
	{
		cognitive->OnSessionBegin.AddDynamic(this, &UHandElevation::OnSessionBegin);
		cognitive->OnPreSessionEnd.AddDynamic(this, &UHandElevation::OnSessionEnd);
		if (cognitive->HasStartedSession())
		{
			OnSessionBegin();
		}
	}
}

void UHandElevation::OnSessionBegin()
{
	auto world = ACognitive3DActor::GetCognitiveSessionWorld();
	if (world == nullptr) { return; }
	world->GetTimerManager().SetTimer(IntervalHandle, FTimerDelegate::CreateUObject(this, &UHandElevation::EndInterval), IntervalDuration, true);
}

void UHandElevation::EndInterval()
{
	auto cognitive = IAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (!cognitive.IsValid() || !cognitive->HasStartedSession()) { return; }

	FVector hmdpos;
	if (!cognitive->TryGetPlayerHMDPosition(hmdpos))
	{
		return;
	}
	float hmdHeight = hmdpos.Z;

	TWeakObjectPtr<UDynamicObject> object = cognitive->GetControllerDynamic(false);
	if (object != nullptr)
	{
		float handHeight = object.Get()->GetComponentLocation().Z;
		float elevationFromHead = FMath::CeilToInt((handHeight - hmdHeight)) / 100.0f;
		cognitive->sensors->RecordSensor("c3d.controller.left.height.fromHMD", elevationFromHead);
	}
	object = cognitive->GetControllerDynamic(true);
	if (object != nullptr)
	{
		float handHeight = object.Get()->GetComponentLocation().Z;
		float elevationFromHead = FMath::CeilToInt((handHeight - hmdHeight)) / 100.0f;
		cognitive->sensors->RecordSensor("c3d.controller.right.height.fromHMD", elevationFromHead);
	}
}

void UHandElevation::OnSessionEnd()
{
	auto world = ACognitive3DActor::GetCognitiveSessionWorld();
	if (world == nullptr) { return; }
	world->GetTimerManager().ClearTimer(IntervalHandle);
}

void UHandElevation::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	auto cognitive = IAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (cognitive.IsValid())
	{
		cognitive->OnSessionBegin.RemoveDynamic(this, &UHandElevation::OnSessionBegin);
		cognitive->OnPreSessionEnd.RemoveDynamic(this, &UHandElevation::OnSessionEnd);
	}
}