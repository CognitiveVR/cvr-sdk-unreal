/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/

#include "Cognitive3D/Private/C3DComponents/FramerateSensor.h"
#include "Cognitive3D/Private/C3DApi/SensorRecorder.h"

#ifdef INCLUDE_OCULUS_PLUGIN
#if ENGINE_MAJOR_VERSION == 4
#include "OculusFunctionLibrary.h"
#elif ENGINE_MAJOR_VERSION == 5
#include "OculusXRFunctionLibrary.h"
#endif
#endif


UFramerateSensor::UFramerateSensor()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UFramerateSensor::BeginPlay()
{
	if (HasBegunPlay()) { return; }
	Super::BeginPlay();

	auto cognitive = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (cognitive.IsValid())
	{
		cognitive->OnSessionBegin.AddDynamic(this, &UFramerateSensor::OnSessionBegin);
		if (cognitive->HasStartedSession())
		{
			OnSessionBegin();
		}
	}
}

void UFramerateSensor::OnSessionBegin()
{
	deltaTimes.Empty();
}

void UFramerateSensor::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	auto cognitive = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (!cognitive.IsValid())
	{
		return;
	}
	if (!cognitive->HasStartedSession())
	{
		return;
	}

	intervalFrameCount++;
	currentTime += DeltaTime;
	deltaTimes.Add(DeltaTime);

	if (currentTime > FramerateTrackingInterval)
	{
		EndInterval();
	}
}

void UFramerateSensor::EndInterval()
{
	float framesPerSecond = intervalFrameCount / currentTime;

	int32 lowerCount5Percent = FMath::CeilToInt(deltaTimes.Num() * 0.05f);
	int32 lowerCount1Percent = FMath::CeilToInt(deltaTimes.Num() * 0.01f);
	deltaTimes.Sort();
	Algo::Reverse(deltaTimes);

	float lowerTotal5Percent = 0;
	for (int32 i = 0; i < lowerCount5Percent;i++)
	{
		lowerTotal5Percent += deltaTimes[i];
	}
	float lowerTotal1Percent = 0;
	for (int32 i = 0; i < lowerCount1Percent;i++)
	{
		lowerTotal1Percent += deltaTimes[i];
	}

	float min5Percent = lowerTotal5Percent / (float)lowerCount5Percent;
	float min1Percent = lowerTotal1Percent / (float)lowerCount1Percent;
	float finalLow5Percent = 1.0f / min5Percent;
	float finalLow1Percent = 1.0f / min1Percent;
	
	auto cognitive = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();

	float fpsMultiplier = 1.0f;

#ifdef INCLUDE_OCULUS_PLUGIN

	bool bIsSpaceWarpSupported = false;

	// Check for r.Mobile.Oculus.SpaceWarp.Enable
	bool bOculusSpaceWarp = false;
	GConfig->GetBool(
		TEXT("/Script/Engine.RendererSettings"),
		TEXT("r.Mobile.Oculus.SpaceWarp.Enable"),
		bOculusSpaceWarp,
		GEngineIni
	);

	// Check for vr.SupportMobileSpaceWarp
	bool bVrSpaceWarp = false;
	GConfig->GetBool(
		TEXT("/Script/Engine.RendererSettings"),
		TEXT("vr.SupportMobileSpaceWarp"),
		bVrSpaceWarp,
		GEngineIni
	);

	// Set bIsSpaceWarpSupported if either setting is true
	bIsSpaceWarpSupported = bOculusSpaceWarp || bVrSpaceWarp;

	if (bIsSpaceWarpSupported)
	{
		cognitive->sensors->RecordSensor("c3d.app.meta.spaceWarp", 1.0);
		cognitive->SetSessionProperty("c3d.app.meta.wasSpaceWarpUsed", true);

#if ENGINE_MAJOR_VERSION == 4
		float currentDisplayFrequency = UOculusFunctionLibrary::GetCurrentDisplayFrequency();
#elif ENGINE_MAJOR_VERSION == 5
		float currentDisplayFrequency = UOculusXRFunctionLibrary::GetCurrentDisplayFrequency();
#endif

		
		if (framesPerSecond <= (currentDisplayFrequency / 2.0f) + TOLERANCE_FOR_CAPPED_FPS)
		{
			fpsMultiplier = 2.0f;
		}
	}
	else
	{
		cognitive->sensors->RecordSensor("c3d.app.meta.spaceWarp", 0.0);
		fpsMultiplier = 1.0f;
	}

#endif

	if (cognitive.IsValid() && cognitive->HasStartedSession())
	{
		cognitive->sensors->RecordSensor("c3d.fps.avg", framesPerSecond * fpsMultiplier);
		cognitive->sensors->RecordSensor("c3d.fps.5pl", finalLow5Percent * fpsMultiplier);
		cognitive->sensors->RecordSensor("c3d.fps.1pl", finalLow1Percent * fpsMultiplier);
	}

	intervalFrameCount = 0;
	currentTime = 0;
	deltaTimes.Empty();
}
