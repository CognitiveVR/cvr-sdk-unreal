// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "CognitiveVR.h"
#include "CognitiveVRPrivatePCH.h"
#include "CognitiveVRSettings.h"

//IMPLEMENT_MODULE(FAnalyticsCognitiveVR, CognitiveVR);

#define LOCTEXT_NAMESPACE "CognitiveVRLoc"

UCognitiveVRSettings::UCognitiveVRSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SettingsDisplayName = LOCTEXT("SettingsDisplayName", "Cognitive VR");
	SettingsTooltip = LOCTEXT("SettingsTooltip", "Cognitive VR analytics configuration settings");
}

void UCognitiveVRSettings::ReadConfigSettings()
{
	//FString ReadApiKey = FAnalytics::Get().GetConfigValueFromIni(GetIniName(), GetTestIniSection(), TEXT("CognitiveVRApiKey"), true);

	FString ReadApiKey = FAnalytics::Get().GetConfigValueFromIni("DefaultEngine", "Analytics", TEXT("CognitiveVRApiKey"), true);
	CustomerID = ReadApiKey;

	
	ReadApiKey = FAnalytics::Get().GetConfigValueFromIni("DefaultEngine", "Analytics", TEXT("CognitiveVRDebugAll"), true);
	if (ReadApiKey.Len() > 0)
	{
		//ReadApiKey = ReleaseApiKey;
		EnableFullDebugLogging = true;
	}
	else
	{
		EnableFullDebugLogging = false;
	}

	ReadApiKey = FAnalytics::Get().GetConfigValueFromIni("DefaultEngine", "Analytics", TEXT("CognitiveVRDebugError"), true);
	if (ReadApiKey.Len() > 0)
	{
		//ReadApiKey = ReleaseApiKey;
		EnableErrorDebugLogging = true;
	}
	else
	{
		EnableErrorDebugLogging = false;
	}
	/*
	ReadApiKey = FAnalytics::Get().GetConfigValueFromIni(GetIniName(), GetTestIniSection(), TEXT("FlurryApiKey"), true);
	if (ReadApiKey.Len() == 0)
	{
		ReadApiKey = ReleaseApiKey;
	}
	TestApiKey = ReadApiKey;

	ReadApiKey = FAnalytics::Get().GetConfigValueFromIni(GetIniName(), GetDevelopmentIniSection(), TEXT("FlurryApiKey"), true);
	if (ReadApiKey.Len() == 0)
	{
		ReadApiKey = ReleaseApiKey;
	}
	DevelopmentApiKey = ReadApiKey;*/
}

void UCognitiveVRSettings::WriteConfigSettings()
{
	//FAnalytics::Get().WriteConfigValueToIni(GetIniName(), GetTestIniSection(), TEXT("CognitiveVRApiKey"), CustomerID);
	FAnalytics::Get().WriteConfigValueToIni("DefaultEngine", "Analytics", TEXT("CognitiveVRApiKey"), CustomerID);

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

	//FAnalytics::Get().WriteConfigValueToIni(GetIniName(), GetReleaseIniSection(), TEXT("CognitiveVRDebugAll"), EnableFullDebugLogging);
	//FAnalytics::Get().WriteConfigValueToIni(GetIniName(), GetReleaseIniSection(), TEXT("CognitiveVRDebugError"), EnableErrorDebugLogging);
	/*FAnalytics::Get().WriteConfigValueToIni(GetIniName(), GetTestIniSection(), TEXT("FlurryApiKey"), TestApiKey);
	FAnalytics::Get().WriteConfigValueToIni(GetIniName(), GetDebugIniSection(), TEXT("FlurryApiKey"), DebugApiKey);
	FAnalytics::Get().WriteConfigValueToIni(GetIniName(), GetDevelopmentIniSection(), TEXT("FlurryApiKey"), DevelopmentApiKey);*/
	GConfig->Flush(false, "DefaultEngine");
}

#undef LOCTEXT_NAMESPACE