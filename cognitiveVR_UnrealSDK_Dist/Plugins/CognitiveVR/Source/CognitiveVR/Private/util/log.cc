/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#include "util/cognitive_log.h"
#include "AnalyticsSettings.h"
#include "Public/CognitiveVRSettings.h"

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

	UCognitiveVRSettings* settings = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider()->GetCognitiveSettings();
	
	MuteWarningMessages = settings->MuteInfoMessages;
	MuteInfoMessages = settings->MuteInfoMessages;
	//MuteDebugMessages = settings->MuteInfoMessages;
	MuteErrorMessages = settings->MuteErrorMessages;

	if (!settings->MuteInfoMessages)
	{
		Warning("==========================");
		Warning("See 'Project Settings > Cognitive VR' for preferences and to toggle debug messages");
		Warning("https://docs.cognitivevr.io/unreal/troubleshooting/ for help");
		Warning("==========================");
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
