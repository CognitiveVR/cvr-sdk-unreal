/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#include "util/cognitive_log.h"
#include "AnalyticsSettings.h"

using namespace cognitivevrapi;

void CognitiveLog::Info(std::string s, bool newline)
{
	FString ValueReceived;

	ValueReceived = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "Analytics", "CognitiveVRDebugAll", false);

	if (ValueReceived.Len() == 0) { return; }
	UE_LOG(CognitiveVR_Log, Log, TEXT("%s"), UTF8_TO_TCHAR(s.c_str()));
}

void CognitiveLog::Warning(std::string s, bool newline)
{
	FString ValueReceived;

	ValueReceived = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "Analytics", "CognitiveVRDebugAll", false);

	if (ValueReceived.Len() == 0) { return; }
	UE_LOG(CognitiveVR_Log, Warning, TEXT("%s"), UTF8_TO_TCHAR(s.c_str()));
}

void CognitiveLog::Error(std::string s, bool newline)
{
	FString ValueReceived;

	ValueReceived = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "Analytics", "CognitiveVRDebugAll", false);

	FString ValueReceivedE;

	ValueReceivedE = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "Analytics", "CognitiveVRDebugError", false);

	if (ValueReceived.Len() + ValueReceivedE.Len() == 0) { return; }
	UE_LOG(CognitiveVR_Log, Error, TEXT("%s"), UTF8_TO_TCHAR(s.c_str()));
}
