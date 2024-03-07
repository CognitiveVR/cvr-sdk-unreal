/*
** Copyright (c) 2016 Cognitive3D, Inc. All rights reserved.
*/
#include "C3DUtil/Cognitive_Log.h"

bool FCognitiveLog::ShowDebugLogs; //basic info/warning/errors
bool FCognitiveLog::ShowDevLogs; //development specific logs
bool FCognitiveLog::HasInitialized = false;

void FCognitiveLog::Init()
{
	ShowDebugLogs = true;
	ShowDevLogs = false;
	FString ValueReceived = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/Cognitive3D.Cognitive3DSettings", "EnableLogging", false);
	if (ValueReceived.Len()>0 && ValueReceived == "false")
	{
		ShowDebugLogs = false;
	}
	else
	{
		if (!HasInitialized)
		{
			HasInitialized = true;
			Warning("==========================");
			Warning("See 'Project Settings > Cognitive VR' for preferences and to toggle debug messages");
			Warning("https://docs.cognitive3d.com/unreal/troubleshooting/ for help");
			Warning("==========================");
		}
	}

	ValueReceived = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/Cognitive3D.Cognitive3DSettings", "EnableDevLogging", false);
	if (ValueReceived.Len()>0 && ValueReceived == "true")
	{
		ShowDevLogs = true;
	}
}

bool FCognitiveLog::DevLogEnabled()
{
	return ShowDevLogs;
}

void FCognitiveLog::DevLog(FString s)
{
	if (!ShowDevLogs) { return; }
	UE_LOG(Cognitive3D_Log, Log, TEXT("%s"), *s);
}

void FCognitiveLog::Info(FString s)
{
	if (!ShowDebugLogs) { return; }
	UE_LOG(Cognitive3D_Log, Log, TEXT("%s"), *s);
}

void FCognitiveLog::Warning(FString s)
{
	if (!ShowDebugLogs) { return; }
	UE_LOG(Cognitive3D_Log, Warning, TEXT("%s"), *s);
}

void FCognitiveLog::Error(FString s)
{
	if (!ShowDebugLogs) { return; }
	UE_LOG(Cognitive3D_Log, Error, TEXT("%s"), *s);
}
