/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/
#include "C3DUtil/CognitiveLog.h"
#include "Analytics.h"

bool FCognitiveLog::ShowDebugLogs; //basic info/warning/errors
bool FCognitiveLog::ShowDevLogs; //development specific logs
bool FCognitiveLog::HasInitialized = false;

void FCognitiveLog::Init()
{
	ShowDebugLogs = true;
	ShowDevLogs = false;

	auto cognitive = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (!cognitive.IsValid()) {
		return;
	}

	FString C3DSettingsPath = cognitive->GetSettingsFilePathRuntime();
	GConfig->LoadFile(C3DSettingsPath);

	FString ValueReceived = FAnalytics::Get().GetConfigValueFromIni(C3DSettingsPath, "/Script/Cognitive3D.Cognitive3DSettings", "EnableLogging", false);
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
