/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/

#pragma once
#include "Cognitive3D/Private/EyeCapture.h"
#include "Runtime/Engine/Classes/Components/SceneComponent.h"
#include "Fixations.generated.h"


//used to pass basic eye data into active session view to draw on screen
USTRUCT(BlueprintType)
struct FC3DGazePoint
{
	GENERATED_BODY()

		//public:
	UPROPERTY(BlueprintReadWrite, Category = "Cognitive3D Analytics")
		USceneComponent* Parent = NULL;
	UPROPERTY(BlueprintReadWrite, Category = "Cognitive3D Analytics")
		bool IsLocal = false;
	UPROPERTY(BlueprintReadWrite, Category = "Cognitive3D Analytics")
		FVector WorldPosition = FVector(0, 0, 0);
	UPROPERTY(BlueprintReadWrite, Category = "Cognitive3D Analytics")
		FVector LocalPosition = FVector(0, 0, 0);
};

USTRUCT()
struct FGazeData
{
	GENERATED_BODY()
public:
		double Time = 0;
		FVector HMDPosition = FVector(0, 0, 0);
		FRotator HMDRotation = FRotator(0, 0, 0);
		
		bool UseGaze = false;
		FVector GazePosition = FVector(0, 0, 0);

		bool UseFloor = false;
		FVector FloorPosition = FVector(0, 0, 0);

		bool UseDynamicId = false;
		FString DynamicObjectId = "";
};

USTRUCT()
struct COGNITIVE3D_API FFixation
{
	GENERATED_BODY()
public:
	FVector WorldPosition;
	FVector LocalPosition;
	//USceneComponent* LocalTransform = NULL;

	//float DebugScale;
	int64 LastUpdated;
	int64 DurationMs;
	int64 StartMs;
	
	int64 LastNonDiscardedTime;
	int64 LastEyesOpen;
	int64 LastInRange;
	int64 LastOnTransform;

	float StartDistance;
	float MaxRadius;
	bool IsLocal;
	FString DynamicObjectId = "";
	
	//set in begin local fixation and updated while fixation is active
	FTransform Transformation;
	
	void AddEyeCapture(FEyeCapture eyeCapture)
	{
		bool flag = true;
		if (eyeCapture.Discard)
		{
			flag = false;
		}
		else
		{
			LastNonDiscardedTime = eyeCapture.Time;
		}
		if (eyeCapture.EyesClosed)
		{
			flag = false;
		}
		else
		{
			LastEyesOpen = eyeCapture.Time;
		}
		if (eyeCapture.OutOfRange)
		{
			flag = false;
		}
		else
		{
			LastInRange = eyeCapture.Time;
		}
		if (eyeCapture.OffTransform)
		{
			flag = false;
		}
		else
		{
			LastOnTransform = eyeCapture.Time;
		}
		if (flag)
		{
			LastUpdated = eyeCapture.Time;
			DurationMs = eyeCapture.Time - StartMs;
		}

		if (IsLocal && eyeCapture.HitDynamicId == DynamicObjectId)
		{
			Transformation = eyeCapture.CaptureMatrix;
		}
	}
};