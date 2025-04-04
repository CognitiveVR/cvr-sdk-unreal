/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/

#pragma once
#include "CoreMinimal.h"

//a collection of these are kept to record current and recent gaze data and the state of the eye

class COGNITIVE3D_API FEyeCapture
{
public:
	FVector WorldPosition;
	FVector LocalPosition;
	
	FTransform CaptureMatrix;
	bool UseCaptureMatrix;
	FString HitDynamicId;

	bool SkipPositionForFixationAverage;
	FVector HMDPosition;
	int64 Time;
	
	bool Discard = true;
	bool EyesClosed;
	bool OutOfRange;
	bool OffTransform;
	FVector2D ScreenPos;
};