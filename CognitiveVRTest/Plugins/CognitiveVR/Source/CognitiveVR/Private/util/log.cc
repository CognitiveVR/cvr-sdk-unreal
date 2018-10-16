/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#include "util/cognitive_log.h"
//#include "AnalyticsSettings.h"

using namespace cognitivevrapi;

bool MuteDebugMessages; //developer and debugging only
bool MuteInfoMessages; //general responses and messaging
bool MuteWarningMessages; //something going wrong with cognitive analytics
bool MuteErrorMessages; //something going very wrong

void CognitiveLog::Init()
{
	MuteDebugMessages = true;
	MuteInfoMessages = false;
	MuteWarningMessages = false;
	MuteErrorMessages = false;
	FString ValueReceived = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "MuteInfoMessages", false);
	if (ValueReceived.Len()>0 && ValueReceived == "true")
	{
		MuteWarningMessages = true;
	}
	else
	{
		Warning("==========================");
		Warning("See 'Project Settings > Cognitive VR' for preferences and to toggle debug messages");
		Warning("https://docs.cognitivevr.io/unreal/troubleshooting/ for help");
		Warning("==========================");
	}

	ValueReceived = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "MuteErrorMessages", false);
	if (ValueReceived.Len()>0 && ValueReceived == "true")
	{
		MuteErrorMessages = true;
	}

	ValueReceived = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "MuteDebugMessages", false);
	if (ValueReceived.Len() == 0 || ValueReceived == "false")
	{
		MuteDebugMessages = false;
	}

	ValueReceived = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "MuteInfoMessages", false);
	if (ValueReceived.Len()>0 && ValueReceived == "true")
	{
		MuteInfoMessages = true;
	}
}

void CognitiveLog::DebugInfo(FString s)
{
	if (MuteDebugMessages) { return; }
	UE_LOG(CognitiveVR_Log, Log, TEXT("%s"), *s);
}

void CognitiveLog::Info(FString s)
{
	if (MuteInfoMessages) { return; }
	UE_LOG(CognitiveVR_Log, Log, TEXT("%s"), *s);
}

void CognitiveLog::Warning(FString s)
{
	if (MuteWarningMessages) { return; }
	UE_LOG(CognitiveVR_Log, Warning, TEXT("%s"), *s);
}

void CognitiveLog::Error(FString s)
{
	if (MuteErrorMessages) { return; }
	UE_LOG(CognitiveVR_Log, Error, TEXT("%s"), *s);
}
