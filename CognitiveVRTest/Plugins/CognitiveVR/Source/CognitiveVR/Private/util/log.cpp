/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#include "util/cognitive_log.h"

bool CognitiveLog::ShowDebugLogs; //basic info/warning/errors
bool CognitiveLog::ShowDevLogs; //development specific logs

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

bool CognitiveLog::DevLogEnabled()
{
	return ShowDevLogs;
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
