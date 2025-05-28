// Fill out your copyright notice in the Description page of Project Settings.


#include "C3DComponents/HMDRecenter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Cognitive3D/Public/Cognitive3DActor.h"
#include "Cognitive3D/Private/C3DApi/CustomEventRecorder.h"

// Sets default values for this component's properties
UHMDRecenter::UHMDRecenter()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UHMDRecenter::BeginPlay()
{
	Super::BeginPlay();

	auto cognitive = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (cognitive.IsValid())
	{
		cognitive->OnSessionBegin.AddDynamic(this, &UHMDRecenter::OnSessionBegin);
		cognitive->OnPreSessionEnd.AddDynamic(this, &UHMDRecenter::OnSessionEnd);
		if (cognitive->HasStartedSession())
		{
			OnSessionBegin();
		}
	}
}


// Called every frame
void UHMDRecenter::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UHMDRecenter::OnSessionBegin()
{
	auto world = ACognitive3DActor::GetCognitiveSessionWorld();
	if (world == nullptr) { return; }

	bool bFoundVRNotifications = false;
	VRNotifications = GetOwner()->FindComponentByClass<UVRNotificationsComponent>();
	if (!VRNotifications)
	{
		UE_LOG(LogTemp, Warning, TEXT("No UVRNotificationsComponent found on %s"), *GetOwner()->GetName());
		VRNotifications = CreateDefaultSubobject<UVRNotificationsComponent>(TEXT("VRNotificationComponent"));
		if (!VRNotifications)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to create UVRNotificationsComponent"));
		}
		else
		{
			bFoundVRNotifications = true;
		}
	}
	else
	{
		bFoundVRNotifications = true;
	}

	if (bFoundVRNotifications)
	{
		FScriptDelegate Delegate;
		Delegate.BindUFunction(this, FName(TEXT("HandleRecenter")));
		VRNotifications->HMDRecenteredDelegate.Add(Delegate);

		FScriptDelegate ControllerDelegate;
		ControllerDelegate.BindUFunction(this, FName(TEXT("HandleControllerRecenter")));
		VRNotifications->VRControllerRecenteredDelegate.Add(ControllerDelegate);
	}

}

void UHMDRecenter::OnSessionEnd()
{
	auto world = ACognitive3DActor::GetCognitiveSessionWorld();
	if (world == nullptr) { return; }

	if (!VRNotifications)
	{
		return;
	}

	VRNotifications->HMDRecenteredDelegate.Remove(this, FName(TEXT("HandleRecenter")));
	VRNotifications->VRControllerRecenteredDelegate.Remove(this, FName(TEXT("HandleControllerRecenter")));
}

void UHMDRecenter::HandleRecenter()
{
	if (!bCanRecenter)
	{
		return; // we already handled one recenter very recently
	}
	bCanRecenter = false;

	auto cognitive = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	//get player position in world space
	cognitive->TryGetPlayerHMDPosition(HMDWorldPos);
	TSharedPtr<FJsonObject> properties = MakeShareable(new FJsonObject());
	properties->SetStringField("Recenter Type", TEXT("HMD Recentered"));
	properties->SetStringField("HMD Position", FString::Printf(TEXT("%f, %f, %f"), HMDWorldPos.X, HMDWorldPos.Y, HMDWorldPos.Z));
	cognitive->customEventRecorder->Send("c3d.User recentered", properties);

	// reset the gate after 0.2 seconds
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(
			DebounceHandle,
			[this]()
			{
				bCanRecenter = true;
			},
			0.2f,
			/*bLoop=*/ false);
	}
}

void UHMDRecenter::HandleControllerRecenter()
{
	if (!bCanRecenter)
	{
		return; // we already handled one recenter very recently
	}
	bCanRecenter = false;

	auto cognitive = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	//get player position in world space
	cognitive->TryGetPlayerHMDPosition(HMDWorldPos);
	TSharedPtr<FJsonObject> properties = MakeShareable(new FJsonObject());
	properties->SetStringField("Recenter Type", TEXT("Controller Recentered"));
	properties->SetStringField("HMD Position", FString::Printf(TEXT("%f, %f, %f"), HMDWorldPos.X, HMDWorldPos.Y, HMDWorldPos.Z));
	cognitive->customEventRecorder->Send("c3d.User recentered", properties);

	// reset the gate after 0.2 seconds
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(
			DebounceHandle,
			[this]()
			{
				bCanRecenter = true;
			},
			0.2f,
			/*bLoop=*/ false);
	}
}

