// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "JsonObjectConverter.h"
#include "Http.h"
#include "Cognitive3D/Private/C3DUtil/Util.h"
#include "Cognitive3D/Public/Cognitive3D.h"
//#include "BoundaryRecorder.generated.h"

/**
 * 
 */

class COGNITIVE3D_API BoundaryRecorder
{

public:
	BoundaryRecorder();
	~BoundaryRecorder();

	TSharedPtr<FAnalyticsProviderCognitive3D> cog;
	int32 jsonPart = 1;

	UFUNCTION()
	void StartSession();
	UFUNCTION()
	void PreSessionEnd();
	UFUNCTION()
	void PostSessionEnd();



	// --- Structs ---

	struct FDataEntry
	{
		double Time;
		FVector P;
		FQuat R; // Important: rotation stored as quaternion
	};

	struct FShapeEntry
	{
		double Time;
		TArray<FVector> Points;
	};

	// --- Member Variables ---
	TArray<FDataEntry> Data;
	TArray<FShapeEntry> Shapes;
	FString UserId;
	double Timestamp;
	FString SessionId;

	// --- Functions ---
	bool SerializeToJsonString(FString& OutJsonString);
	bool DeserializeFromJsonString(const FString& InJsonString);

	void AddGuardianBoundaryPoints(TArray<FVector>& GuardianPoints);

	void CapturePlayerTransform();

	void BoundaryCheckInterval();

	void SendData(bool bCacheData);

private:

	FString JsonDataString;

	FTimerHandle DelayedCaptureTimerHandle;   // For the interval timer
	void PerformInitialCapture(); // will be called after delay
	bool bGuardianCaptured = false;        // Do once
	TArray<FVector> LastCapturedGuardianPoints; // To compare for changes later

	FVector LastPlayerLocation;
	FTimerHandle IntervalTimerHandle; // For the interval timer
};

