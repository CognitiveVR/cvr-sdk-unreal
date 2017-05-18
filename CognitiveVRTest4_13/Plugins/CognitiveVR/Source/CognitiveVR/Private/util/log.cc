/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#include "util/cognitive_log.h"
#include "AnalyticsSettings.h"

using namespace cognitivevrapi;

void CognitiveLog::Info(std::string s, bool newline)
{
	FString ValueReceived;

	ValueReceived = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "EnableFullDebugLogging", false);
	if (ValueReceived.Len() != 4) { return; }
	UE_LOG(CognitiveVR_Log, Log, TEXT("%s"), UTF8_TO_TCHAR(s.c_str()));
}

void CognitiveLog::Warning(std::string s, bool newline)
{
	FString ValueReceived;

	ValueReceived = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "EnableFullDebugLogging", false);
	if (ValueReceived.Len() != 4) { return; }
	UE_LOG(CognitiveVR_Log, Warning, TEXT("%s"), UTF8_TO_TCHAR(s.c_str()));
}

void CognitiveLog::Error(std::string s, bool newline)
{
	FString ValueReceived;

	ValueReceived = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "EnableFullDebugLogging", false);
	if (ValueReceived.Len() != 4) { return; }

	FString ValueReceivedE;

	ValueReceived = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "EnableErrorDebugLogging", false);
	if (ValueReceived.Len() != 4) { return; }

	UE_LOG(CognitiveVR_Log, Error, TEXT("%s"), UTF8_TO_TCHAR(s.c_str()));
}
