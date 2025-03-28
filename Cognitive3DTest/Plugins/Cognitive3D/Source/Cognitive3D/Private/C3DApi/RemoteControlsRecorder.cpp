// Fill out your copyright notice in the Description page of Project Settings.


#include "C3DApi/RemoteControlsRecorder.h"
#include "Cognitive3D/Private/C3DComponents/RemoteControls.h"

FRemoteControlsRecorder* FRemoteControlsRecorder::RemoteControlsRecorderInstance;

void FRemoteControlsRecorder::Initialize()
{
	RemoteControlsRecorderInstance = new FRemoteControlsRecorder;

	RemoteControlsRecorderInstance->bHasRemoteControlVariables = false;
	RemoteControlsRecorderInstance->RemoteControlVariablesString.Empty();
	RemoteControlsRecorderInstance->RemoteControlVariablesInt.Empty();
	RemoteControlsRecorderInstance->RemoteControlVariablesFloat.Empty();
	RemoteControlsRecorderInstance->RemoteControlVariablesBool.Empty();
}

FRemoteControlsRecorder* FRemoteControlsRecorder::GetInstance()
{
	return RemoteControlsRecorderInstance;
}

FRemoteControlsRecorder::FRemoteControlsRecorder()
{
}

FRemoteControlsRecorder::~FRemoteControlsRecorder()
{
}
