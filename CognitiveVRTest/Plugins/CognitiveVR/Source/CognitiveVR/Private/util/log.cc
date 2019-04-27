/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#include "util/cognitive_log.h"
//#include "AnalyticsSettings.h"

using namespace cognitivevrapi;

bool ShowDebugLogs; //basic info/warning/errors
bool ShowDevLogs; //development specific logs

//bool MuteDebugMessages; //developer and debugging only
//bool MuteInfoMessages; //general responses and messaging
//bool MuteWarningMessages; //something going wrong with cognitive analytics
//bool MuteErrorMessages; //something going very wrong

void CognitiveLog::Init()
{
	ShowDebugLogs = true;
	ShowDevLogs = false;
	FString ValueReceived = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "EnableLogging", false);
	if (ValueReceived.Len()>0 && ValueReceived == "false")
	{
		ShowDebugLogs = false;
	}
	else
	{
		Warning("==========================");
		Warning("See 'Project Settings > Cognitive VR' for preferences and to toggle debug messages");
		Warning("https://docs.cognitivevr.io/unreal/troubleshooting/ for help");
		Warning("==========================");
	}

	ValueReceived = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "EnableDevLogging", false);
	if (ValueReceived.Len()>0 && ValueReceived == "true")
	{
		ShowDevLogs = true;
	}
}

void CognitiveLog::DevLog(FString s)
{
	if (!ShowDevLogs) { return; }
	UE_LOG(CognitiveVR_Log, Log, TEXT("%s"), *s);
}

void CognitiveLog::Info(FString s)
{
	if (!ShowDebugLogs) { return; }
	UE_LOG(CognitiveVR_Log, Log, TEXT("%s"), *s);
}

void CognitiveLog::Warning(FString s)
{
	if (!ShowDebugLogs) { return; }
	UE_LOG(CognitiveVR_Log, Warning, TEXT("%s"), *s);
}

void CognitiveLog::Error(FString s)
{
	if (!ShowDebugLogs) { return; }
	UE_LOG(CognitiveVR_Log, Error, TEXT("%s"), *s);
}
