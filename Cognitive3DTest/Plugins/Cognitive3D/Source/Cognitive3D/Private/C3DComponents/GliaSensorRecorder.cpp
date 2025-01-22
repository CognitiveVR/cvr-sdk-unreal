/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/

//#include "Cognitive3D.h"
#include "GliaSensorRecorder.h"
//#include "Cognitive3DSettings.h"
//#include "Util.h"

// Sets default values for this component's properties
UGliaSensorRecorder::UGliaSensorRecorder()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UGliaSensorRecorder::BeginPlay()
{
	if (HasBegunPlay()) { return; }
	Super::BeginPlay();

	//listen for event or find controller immediately if session already started
	cog = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (cog->HasStartedSession())
	{
		StartListenForTimers();
	}
	cog->OnSessionBegin.AddDynamic(this, &UGliaSensorRecorder::StartListenForTimers);
	cog->OnPreSessionEnd.AddDynamic(this, &UGliaSensorRecorder::StopListenForTimers);
}

void UGliaSensorRecorder::StartListenForTimers()
{
#if defined INCLUDE_HPGLIA_PLUGIN
	GetWorld()->GetTimerManager().SetTimer(AutoSendHandle1000MS, this, &UGliaSensorRecorder::TickSensors1000MS, 1, true);
	GetWorld()->GetTimerManager().SetTimer(AutoSendHandle100MS, this, &UGliaSensorRecorder::TickSensors100MS, 0.1, true);
#endif
}

void UGliaSensorRecorder::StopListenForTimers()
{
#if defined INCLUDE_HPGLIA_PLUGIN
	GetWorld()->GetTimerManager().ClearTimer(AutoSendHandle1000MS);
	GetWorld()->GetTimerManager().ClearTimer(AutoSendHandle100MS);
#endif
}

void UGliaSensorRecorder::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	TSharedPtr<FAnalyticsProviderCognitive3D> cogProvider = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	cogProvider->OnSessionBegin.RemoveDynamic(this, &UGliaSensorRecorder::StartListenForTimers);
	cogProvider->OnPreSessionEnd.RemoveDynamic(this, &UGliaSensorRecorder::StopListenForTimers);
	StopListenForTimers();

	Super::EndPlay(EndPlayReason);
}

#if defined INCLUDE_HPGLIA_PLUGIN
void UGliaSensorRecorder::TickSensors1000MS()
{
	int32 OutHeartRate = 0;
	if (UHPGliaClient::GetHeartRate(OutHeartRate) && OutHeartRate != LastHeartRate)
	{
		cog->sensors->RecordSensor("HP.HeartRate", (float)OutHeartRate);
		LastHeartRate = OutHeartRate;
	}

	FCognitiveLoad CognitiveLoad;
	if (UHPGliaClient::GetCognitiveLoad(CognitiveLoad))
	{
		if (!FMath::IsNearlyEqual(CognitiveLoad.CognitiveLoad, LastCognitiveLoad))
		{
			cog->sensors->RecordSensor("HP.CognitiveLoad", CognitiveLoad.CognitiveLoad);
			cog->sensors->RecordSensor("HP.CognitiveLoad.Confidence", CognitiveLoad.StandardDeviation);
			LastCognitiveLoad = CognitiveLoad.CognitiveLoad;
		}
	}
}

void UGliaSensorRecorder::TickSensors100MS()
{
	FEyeTracking data;
	if (UHPGliaClient::GetEyeTracking(data))
	{
		if (data.LeftPupilDilationConfidence > 0.5 && data.LeftPupilDilation > 1.5f && !FMath::IsNearlyEqual(data.LeftPupilDilation, LastLeftPupilDiamter))
		{
			cog->sensors->RecordSensor("HP.Left Pupil Diameter", data.LeftPupilDilation);
			LastLeftPupilDiamter = data.LeftPupilDilation;
		}
		if (data.RightPupilDilationConfidence > 0.5 && data.RightPupilDilation > 1.5f && !FMath::IsNearlyEqual(data.RightPupilDilation, LastRightPupilDiamter))
		{
			cog->sensors->RecordSensor("HP.Right Pupil Diameter", data.RightPupilDilation);
			LastRightPupilDiamter = data.RightPupilDilation;
		}
	}
}
#endif