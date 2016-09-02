/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#include "util/log.h"
#include "CognitiveVRAnalyticsPrivatePCH.h"

namespace cognitivevrapi
{
    /*void Log::LogString(std::string s, bool newline)
    {
        std::string nstring = "[" + Log::CurrentDateTime() + "] " + s;
        std::cout << nstring;
        if (newline) {
            std::cout << "\n";
        }
    }*/

	void Log::Info(std::string s, bool newline)
	{
		FString ValueReceived;

		GConfig->GetString(
			TEXT("Analytics"),
			TEXT("CognitiveVRDebugAll"),
			ValueReceived,
			GGameIni
		);

		if (ValueReceived.Len() == 0) { return; }
		UE_LOG(CognitiveVR_Log, Log, TEXT("%s"), UTF8_TO_TCHAR(s.c_str()));

		/*
		UCognitiveVRSettings* Settings = GetMutableDefault<UCognitiveVRSettings>();
		if (!Settings || !Settings->EnableFullDebugLogging) { return; }
		if (s.empty()) { return; }
		UE_LOG(CognitiveVR_Log, Log, TEXT("%s"),UTF8_TO_TCHAR(s.c_str()));*/
	}

    void Log::Warning(std::string s, bool newline)
    {
		FString ValueReceived;

		GConfig->GetString(
			TEXT("Analytics"),
			TEXT("CognitiveVRDebugAll"),
			ValueReceived,
			GGameIni
		);

		if (ValueReceived.Len() == 0) { return; }
		UE_LOG(CognitiveVR_Log, Warning, TEXT("%s"), UTF8_TO_TCHAR(s.c_str()));

		/*UCognitiveVRSettings* Settings = GetMutableDefault<UCognitiveVRSettings>();
		if (!Settings || !Settings->EnableFullDebugLogging) { return; }
		if (s.empty()) { return; }
		UE_LOG(CognitiveVR_Log, Warning, TEXT("%s"), UTF8_TO_TCHAR(s.c_str()));*/
    }

    void Log::Error(std::string s, bool newline)
    {
		FString ValueReceived;

		GConfig->GetString(
			TEXT("Analytics"),
			TEXT("CognitiveVRDebugAll"),
			ValueReceived,
			GGameIni
		);

		FString ValueReceivedE;

		GConfig->GetString(
			TEXT("Analytics"),
			TEXT("CognitiveVRDebugError"),
			ValueReceivedE,
			GGameIni
		);

		if (ValueReceived.Len() + ValueReceivedE.Len() == 0) { return; }
		UE_LOG(CognitiveVR_Log, Error, TEXT("%s"), UTF8_TO_TCHAR(s.c_str()));

		/*UCognitiveVRSettings* Settings = GetMutableDefault<UCognitiveVRSettings>();
		if (!Settings || (!Settings->EnableFullDebugLogging && !Settings->EnableErrorDebugLogging)) { return; }
		if (s.empty()) { return; }
		UE_LOG(CognitiveVR_Log, Error, TEXT("%s"), UTF8_TO_TCHAR(s.c_str()));*/
    }
}