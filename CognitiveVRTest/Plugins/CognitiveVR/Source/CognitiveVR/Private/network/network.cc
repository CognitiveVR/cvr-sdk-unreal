/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#include "network.h"
#include "AnalyticsSettings.h"
#include "Http.h"

using namespace cognitivevrapi;
FHttpModule* Http;
FString Gateway;

Network::Network(FAnalyticsProviderCognitiveVR* sp)
{
	CognitiveLog::Info("CognitiveVR::Network - Init");
	Gateway = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "Gateway", false);
    s = sp;
	if (Http == NULL)
		Http = &FHttpModule::Get();
}

void Network::NetworkCall(FString suburl, FString contents)
{	
	if (s == NULL)
	{
		CognitiveLog::Warning("FAnalyticsProviderCognitiveVR::SendJson CognitiveVRProvider has not started a session!");
		return;
	}

	if (s->GetCurrentSceneId().Len() == 0)
	{
		CognitiveLog::Warning("FAnalyticsProviderCognitiveVR::SendJson CognitiveVRProvider has not started a session!");
		return;
	}
	if (s->GetCurrentSceneVersionNumber().Len() == 0)
	{
		CognitiveLog::Warning("FAnalyticsProviderCognitiveVR::SendJson CognitiveVRProvider has not started a session!");
		return;
	}

	if (Http == NULL)
	{
		CognitiveLog::Warning("Cognitive Provider::SendJson Http module not initialized! likely hasn't started session");
		return;
	}

	//json to scene endpoint
	FString url = "https://"+ Gateway +"/v"+FString::FromInt(0)+"/"+suburl+"/"+s->GetCurrentSceneId() + "?version=" + s->GetCurrentSceneVersionNumber();

	FString AuthValue = "APIKEY:DATA " + s->APIKey;

	TSharedRef<IHttpRequest> HttpRequest = Http->CreateRequest();
	HttpRequest->SetContentAsString(contents);
	HttpRequest->SetURL(url);
	HttpRequest->SetVerb("POST");
	HttpRequest->SetHeader("Content-Type", TEXT("application/json"));
	HttpRequest->SetHeader("Authorization", AuthValue);
	HttpRequest->ProcessRequest();
	return;
}

void Network::NetworkExitPollGet(FString hook)
{

}
void Network::NetworkExitPollPost(FString questionsetname, FString questionsetversion, FString contents)
{

}