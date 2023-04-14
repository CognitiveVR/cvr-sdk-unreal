#include "CognitiveVR/Private/HMDOrientation.h"

UHMDOrientation::UHMDOrientation()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UHMDOrientation::BeginPlay()
{
	if (HasBegunPlay()) { return; }
	Super::BeginPlay();

	GEngine->GetAllLocalPlayerControllers(controllers);
}

void UHMDOrientation::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	currentTime += DeltaTime;

	if (currentTime > Interval)
	{
		currentTime = 0;
		RecordYaw();
		RecordPitch();
	}
}

void UHMDOrientation::RecordYaw()
{
	FRotator hmdRotation = controllers[0]->PlayerCameraManager->GetCameraRotation();
	float yaw = hmdRotation.Yaw;
	auto cognitive = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	if (cognitive.IsValid() && cognitive->HasStartedSession())
	{
		cognitive->sensors->RecordSensor("c3d.hmd.yaw", yaw);
	}
}

void UHMDOrientation::RecordPitch()
{
	FRotator hmdRotation = controllers[0]->PlayerCameraManager->GetCameraRotation();
	float pitch = hmdRotation.Pitch;
	auto cognitive = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	if (cognitive.IsValid() && cognitive->HasStartedSession())
	{
		cognitive->sensors->RecordSensor("c3d.hmd.pitch", pitch);
	}
}
