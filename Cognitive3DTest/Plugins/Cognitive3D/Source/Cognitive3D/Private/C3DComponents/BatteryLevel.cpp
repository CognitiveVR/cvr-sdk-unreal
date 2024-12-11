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
		int32 batteryState = 1;
		FString batteryStateString = "";
		bool isRunningOnBattery = false;
#if PLATFORM_ANDROID
		batteryLevel = FAndroidMisc::GetBatteryState().Level;
		batteryState = FAndroidMisc::GetBatteryState().State;
		isRunningOnBattery = FAndroidMisc::IsRunningOnBattery();
		switch (batteryState) {
		case 1:
			batteryStateString = "Unknown";
			break;
		case 2:
			batteryStateString = "Charging";
			break;
		case 3:
			batteryStateString = "Discharging";
			break;
		case 4:
			batteryStateString = "Not Charging";
			break;
		case 5:
			batteryStateString = "Full";
			break;
		}
#endif
		cognitive->sensors->RecordSensor("HMD Battery Level", (float)batteryLevel);
		cognitive->SetSessionProperty("HMD Battery Status Name", batteryStateString);
		batteryState -= 1; //to remove +1 offset from constructor in AndroidPlatformMisc.h
		cognitive->sensors->RecordSensor("HMD Battery Status", (float)batteryState);
		if (isRunningOnBattery)
		{
			cognitive->SetSessionProperty("c3d.hmd.RunningOnBattery", FString("Yes"));
		}
		else
		{
			cognitive->SetSessionProperty("c3d.hmd.RunningOnBattery", FString("No"));
		}
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