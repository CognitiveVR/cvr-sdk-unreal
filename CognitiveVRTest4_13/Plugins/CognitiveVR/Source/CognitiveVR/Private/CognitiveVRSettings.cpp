// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "CognitiveVR.h"
#include "CognitiveVRPrivatePCH.h"
#include "CognitiveVRSettings.h"

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
	//full
	if (EnableFullDebugLogging)
	{
		FAnalytics::Get().WriteConfigValueToIni("DefaultEngine", "Analytics", TEXT("CognitiveVRDebugAll"), "true");
	}
	else
	{
		FAnalytics::Get().WriteConfigValueToIni("DefaultEngine", "Analytics", TEXT("CognitiveVRDebugAll"), "");
	}
												
	//errors only					
	if (EnableErrorDebugLogging)			
	{										
		FAnalytics::Get().WriteConfigValueToIni("DefaultEngine", "Analytics", TEXT("CognitiveVRDebugError"), "true");
	}										
	else									
	{			
		FAnalytics::Get().WriteConfigValueToIni("DefaultEngine", "Analytics", TEXT("CognitiveVRDebugError"), "");
	}

	GConfig->Flush(false, "DefaultEngine");
}

#undef LOCTEXT_NAMESPACE