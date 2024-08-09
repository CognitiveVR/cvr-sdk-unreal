/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/

#include "Cognitive3DSettings.h"
//#include "Cognitive3D.h"
//#include "Cognitive3DPrivatePCH.h"

#define LOCTEXT_NAMESPACE "Cognitive3DLoc"

UCognitive3DSettings::UCognitive3DSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SettingsDisplayName = LOCTEXT("SettingsDisplayName", "Cognitive 3D");
	SettingsTooltip = LOCTEXT("SettingsTooltip", "Cognitive 3D analytics configuration settings");
}

void UCognitive3DSettings::ReadConfigSettings()
{

}

void UCognitive3DSettings::WriteConfigSettings()
{

}


#undef LOCTEXT_NAMESPACE