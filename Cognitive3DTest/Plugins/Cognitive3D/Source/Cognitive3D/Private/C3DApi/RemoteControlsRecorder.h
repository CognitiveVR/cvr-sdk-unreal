// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Cognitive3D/Public/Cognitive3D.h"
#include "TimerManager.h"
#include "Cognitive3D/Private/C3DUtil/Util.h"
#include "Cognitive3D/Private/C3DComponents/RemoteControls.h"
#include "Delegates/Delegate.h"
#include "CoreMinimal.h"

class FRemoteControlsRecorder
{
public:

	TMap<FString, FString> RemoteControlVariablesString;
	TMap<FString, int32> RemoteControlVariablesInt;
	TMap<FString, float> RemoteControlVariablesFloat;
	TMap<FString, bool> RemoteControlVariablesBool;

	bool bHasRemoteControlVariables;

	static FRemoteControlsRecorder* RemoteControlsRecorderInstance;

	static void Initialize();

	static FRemoteControlsRecorder* GetInstance();

	FRemoteControlsRecorder();
	~FRemoteControlsRecorder();
};
