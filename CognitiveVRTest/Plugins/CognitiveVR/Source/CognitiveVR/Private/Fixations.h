// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "EyeCapture.h"
#include "Runtime/Engine/Classes/Components/SceneComponent.h"
#include "Fixations.generated.h"

//TODO use this struct for drawing saccade lines/fixations in active session view on dynamics
USTRUCT(BlueprintType)
struct FC3DFixation
{
	GENERATED_BODY()

//public:
	UPROPERTY(BlueprintReadWrite)
	float radius;
	UPROPERTY(BlueprintReadWrite)
	USceneComponent* Parent;
	UPROPERTY(BlueprintReadWrite)
	bool IsLocalFixation;
	UPROPERTY(BlueprintReadWrite)
	FVector Position = FVector(0, 0, 0);
};

class COGNITIVEVR_API FFixation
{
public:
	FVector WorldPosition;
	FVector LocalPosition;
	//UDynamicObject* LocalTransform;
	USceneComponent* LocalTransform;

	float DebugScale;
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
	FString DynamicObjectId;
	
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
			DurationMs = StartMs - LastUpdated;
		}
	}
};