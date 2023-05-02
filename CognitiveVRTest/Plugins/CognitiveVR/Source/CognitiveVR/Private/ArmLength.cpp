#include "CognitiveVR/Private/ArmLength.h"

UArmLength::UArmLength()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UArmLength::BeginPlay()
{
	if (HasBegunPlay()) { return; }
	Super::BeginPlay();

	auto cognitive = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	if (cognitive.IsValid())
	{
		cognitive->OnSessionBegin.AddDynamic(this, &UArmLength::OnSessionBegin);
		cognitive->OnPreSessionEnd.AddDynamic(this, &UArmLength::OnSessionEnd);
		if (cognitive->HasStartedSession())
		{
			OnSessionBegin();
		}
	}
}

void UArmLength::OnSessionBegin()
{
	auto world = ACognitiveVRActor::GetCognitiveSessionWorld();
	if (world == nullptr) { return; }
	world->GetTimerManager().SetTimer(IntervalHandle, FTimerDelegate::CreateUObject(this, &UArmLength::EndInterval), IntervalDuration, true);
	CurrentSampleCount = 0;
	ArmLength = 0;
}

void UArmLength::EndInterval()
{
	auto cognitive = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	if (cognitive.IsValid())
	{
		//get arm length

		auto hmdpos = cognitive->GetPlayerHMDPosition();
		FVector shoulderPos = hmdpos + FVector(0, 0, -EyeToShoulderHeight);

		TWeakObjectPtr<UDynamicObject> object = cognitive->GetControllerDynamic(false);
		if (object != nullptr)
		{
			auto handPos = object.Get()->GetComponentLocation();
			ArmLength = FMath::Max(ArmLength, FVector::Distance(handPos, shoulderPos));
		}
		object = cognitive->GetControllerDynamic(true);
		if (object != nullptr)
		{
			auto handPos = object.Get()->GetComponentLocation();
			ArmLength = FMath::Max(ArmLength, FVector::Distance(handPos, shoulderPos));
		}

		CurrentSampleCount++;
		if (CurrentSampleCount > NumberOfSamples)
		{
			cognitive->OnSessionBegin.RemoveDynamic(this, &UArmLength::OnSessionBegin);
			cognitive->SetSessionProperty("c3d.armlength", ArmLength);
		}
	}
}

void UArmLength::OnSessionEnd()
{
	auto world = ACognitiveVRActor::GetCognitiveSessionWorld();
	if (world == nullptr) { return; }
	world->GetTimerManager().ClearTimer(IntervalHandle);
}

void UArmLength::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	auto cognitive = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	if (cognitive.IsValid())
	{
		cognitive->OnSessionBegin.RemoveDynamic(this, &UArmLength::OnSessionBegin);
		cognitive->OnPreSessionEnd.RemoveDynamic(this, &UArmLength::OnSessionEnd);
		if (cognitive->HasStartedSession())
		{
			OnSessionEnd();
		}
	}
}