/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#include "util/cognitive_log.h"
#include "AnalyticsSettings.h"

using namespace cognitivevrapi;

bool MuteInfoMessages;
bool MuteErrorMessages;

void CognitiveLog::Init()
{
	FString ValueReceived = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "MuteInfoMessages", false);
	if (ValueReceived.Len() == 4 && ValueReceived == "true")
	{
		MuteInfoMessages = true;
	}

	ValueReceived = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "MuteErrorMessages", false);
	if (ValueReceived.Len() == 4 && ValueReceived == "true")
	{
		MuteErrorMessages = true;
	}
}

void CognitiveLog::Info(std::string s, bool newline)
{
	if (MuteInfoMessages) { return; }
	UE_LOG(CognitiveVR_Log, Log, TEXT("%s"), UTF8_TO_TCHAR(s.c_str()));
}

void CognitiveLog::Warning(std::string s, bool newline)
{
	if (MuteInfoMessages) { return; }
	UE_LOG(CognitiveVR_Log, Warning, TEXT("%s"), UTF8_TO_TCHAR(s.c_str()));
}

void CognitiveLog::Error(std::string s, bool newline)
{
	if (MuteErrorMessages) { return; }
	UE_LOG(CognitiveVR_Log, Error, TEXT("%s"), UTF8_TO_TCHAR(s.c_str()));
}
