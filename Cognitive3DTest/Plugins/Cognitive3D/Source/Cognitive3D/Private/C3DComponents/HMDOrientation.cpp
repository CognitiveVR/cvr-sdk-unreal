/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/

#include "Cognitive3D/Private/C3DComponents/HMDOrientation.h"
#include "Cognitive3D/Private/C3DApi/SensorRecorder.h"
#include "Cognitive3D/Public/Cognitive3DActor.h"

UHMDOrientation::UHMDOrientation()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UHMDOrientation::BeginPlay()
{
	if (HasBegunPlay()) { return; }
	Super::BeginPlay();

	auto cognitive = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (cognitive.IsValid())
	{
		cognitive->OnSessionBegin.AddDynamic(this, &UHMDOrientation::OnSessionBegin);
		cognitive->OnPreSessionEnd.AddDynamic(this, &UHMDOrientation::OnSessionEnd);
		if (cognitive->HasStartedSession())
		{
			OnSessionBegin();
		}
	}
}

void UHMDOrientation::OnSessionBegin()
{
	auto world = ACognitive3DActor::GetCognitiveSessionWorld();
	if (world == nullptr) { return; }
	world->GetTimerManager().SetTimer(IntervalHandle, FTimerDelegate::CreateUObject(this, &UHMDOrientation::EndInterval), IntervalDuration, true);
}

void UHMDOrientation::EndInterval()
{
	RecordYaw();
	RecordPitch();
}

void UHMDOrientation::RecordYaw()
{
	auto cognitive = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (cognitive.IsValid())
	{
		FRotator rot;
		if (cognitive->TryGetPlayerHMDLocalRotation(rot))
		{
			float yaw = rot.Yaw;
			cognitive->sensors->RecordSensor("c3d.hmd.yaw", yaw);
		}
	}
}

void UHMDOrientation::RecordPitch()
{
	auto cognitive = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (cognitive.IsValid())
	{
		FRotator rot;
		if (cognitive->TryGetPlayerHMDLocalRotation(rot))
		{
			float pitch = rot.Pitch;
			cognitive->sensors->RecordSensor("c3d.hmd.pitch", pitch);
		}
	}
}

void UHMDOrientation::OnSessionEnd()
{
	auto world = ACognitive3DActor::GetCognitiveSessionWorld();
	if (world == nullptr) { return; }
	world->GetTimerManager().ClearTimer(IntervalHandle);
}

void UHMDOrientation::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	auto cognitive = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (cognitive.IsValid())
	{
		cognitive->OnSessionBegin.RemoveDynamic(this, &UHMDOrientation::OnSessionBegin);
		cognitive->OnPreSessionEnd.RemoveDynamic(this, &UHMDOrientation::OnSessionEnd);
		if (cognitive->HasStartedSession())
		{
			OnSessionEnd();
		}
	}
}