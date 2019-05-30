/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#include "network.h"
//#include "AnalyticsSettings.h"
#include "Http.h"

//using namespace cognitivevrapi;
FHttpModule* Http;
FString Gateway;

cognitivevrapi::Network::Network(FAnalyticsProviderCognitiveVR* sp)
{
	Gateway = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "Gateway", false);
    s = sp;
	if (Http == NULL)
		Http = &FHttpModule::Get();
}

void cognitivevrapi::Network::NetworkCall(FString suburl, FString contents)
{	
	if (s == NULL)
	{
		cognitivevrapi::CognitiveLog::Warning("FAnalyticsProviderCognitiveVR::SendJson CognitiveVRProvider has not started a session!");
		return;
	}

	if (s->GetCurrentSceneId().Len() == 0)
	{
		cognitivevrapi::CognitiveLog::Warning("FAnalyticsProviderCognitiveVR::SendJson CognitiveVRProvider has not started a session!");
		return;
	}
	if (s->GetCurrentSceneVersionNumber().Len() == 0)
	{
		cognitivevrapi::CognitiveLog::Warning("FAnalyticsProviderCognitiveVR::SendJson CognitiveVRProvider has not started a session!");
		return;
	}

	if (Http == NULL)
	{
		cognitivevrapi::CognitiveLog::Warning("Cognitive Provider::SendJson Http module not initialized! likely hasn't started session");
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

	if (cognitivevrapi::CognitiveLog::DevLogEnabled())
		cognitivevrapi::CognitiveLog::DevLog(url + "\n" + contents);

	return;
}

void cognitivevrapi::Network::NetworkExitPollGet(FString hook)
{

}
void cognitivevrapi::Network::NetworkExitPollPost(FString questionsetname, FString questionsetversion, FString contents)
{

}