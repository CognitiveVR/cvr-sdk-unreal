/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/

#include "Cognitive3D/Private/C3DComponents/BatteryLevel.h"

UBatteryLevel::UBatteryLevel()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UBatteryLevel::BeginPlay()
{
	if (HasBegunPlay()) { return; }
	Super::BeginPlay();

	auto cognitive = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (cognitive.IsValid())
	{
		cognitive->OnSessionBegin.AddDynamic(this, &UBatteryLevel::OnSessionBegin);
		cognitive->OnPreSessionEnd.AddDynamic(this, &UBatteryLevel::OnSessionEnd);
		if (cognitive->HasStartedSession())
		{
			OnSessionBegin();
		}
	}
}

void UBatteryLevel::OnSessionBegin()
{
	auto world = ACognitive3DActor::GetCognitiveSessionWorld();
	if (world == nullptr) { return; }
	world->GetTimerManager().SetTimer(IntervalHandle, FTimerDelegate::CreateUObject(this, &UBatteryLevel::EndInterval), Interval, true);
}

void UBatteryLevel::EndInterval()
{
	auto cognitive = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (cognitive.IsValid())
	{
		int32 batteryLevel = 0;
#if PLATFORM_ANDROID
		batteryLevel = FAndroidMisc::GetBatteryState().Level;
#endif
		cognitive->sensors->RecordSensor("HMD Battery Level", (float)batteryLevel);
	}
}
void UBatteryLevel::OnSessionEnd()
{
	auto world = ACognitive3DActor::GetCognitiveSessionWorld();
	if (world == nullptr) { return; }
	world->GetTimerManager().ClearTimer(IntervalHandle);
}

void UBatteryLevel::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	auto cognitive = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (cognitive.IsValid())
	{
		cognitive->OnSessionBegin.RemoveDynamic(this, &UBatteryLevel::OnSessionBegin);
		cognitive->OnPreSessionEnd.RemoveDynamic(this, &UBatteryLevel::OnSessionEnd);
		if (cognitive->HasStartedSession())
		{
			OnSessionEnd();
		}
	}
}