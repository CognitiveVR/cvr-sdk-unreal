// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "CognitiveVRSettings.h"
//#include "CognitiveVR.h"
//#include "CognitiveVRPrivatePCH.h"

#define LOCTEXT_NAMESPACE "CognitiveVRLoc"

UCognitiveVRSettings::UCognitiveVRSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SettingsDisplayName = LOCTEXT("SettingsDisplayName", "Cognitive VR");
	SettingsTooltip = LOCTEXT("SettingsTooltip", "Cognitive VR analytics configuration settings");
}

void UCognitiveVRSettings::ReadConfigSettings()
{

}

void UCognitiveVRSettings::WriteConfigSettings()
{

}


#undef LOCTEXT_NAMESPACE