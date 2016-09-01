#include "CognitiveVRAnalyticsPrivatePCH.h"
#include "CognitiveVRAnalytics.h"
#include "json.h"
#include "ISettingsModule.h"
#include "ISettingsSection.h"
#include "ModuleInterface.h"
#include "ModuleManager.h"

DEFINE_LOG_CATEGORY(CognitiveVR_Log);

#define LOCTEXT_NAMESPACE "FCognitiveVRAnalytics"

void FCognitiveVRAnalytics::Init(std::string user_id, std::string device_id)
{
	if (this->cognitivevr == NULL)
	{

	}
	else if (this->cognitivevr->user_id.IsEmpty() && this->cognitivevr->device_id.IsEmpty())
	{
		cognitivevrapi::Log::Info("CognitiveVRAnalytics::Init - Found empty cognitivevr object. Initializing");
		TSharedPtr<FJsonObject> properties = MakeShareable(new FJsonObject);
		this->cognitivevr = cognitivevrapi::Init(user_id, device_id, properties);
	}
	return;
}

void FCognitiveVRAnalytics::Init(std::string user_id, std::string device_id, TSharedPtr<FJsonObject>init_device_properties)
{
	if (this->cognitivevr == NULL)
	{
		
	}
	else if (this->cognitivevr->user_id.IsEmpty() && this->cognitivevr->device_id.IsEmpty())
	{
		cognitivevrapi::Log::Info("CognitiveVRAnalytics::Init - Found empty cognitivevr object. Initializing");
		this->cognitivevr = cognitivevrapi::Init(user_id, device_id, init_device_properties);
	}
	return;
}

cognitivevrapi::CognitiveVR* FCognitiveVRAnalytics::CognitiveVR()
{
	if (this->cognitivevr == NULL)
	{
		TSharedPtr<FJsonObject> init_device_properties = MakeShareable(new FJsonObject);
		this->cognitivevr = cognitivevrapi::Init("", "", init_device_properties);
		cognitivevrapi::Log::Warning("CognitiveVR Not Initialized!");
	}

	return this->cognitivevr;
}


void FCognitiveVRAnalytics::StartupModule()
{
	// This code will execute after your module is loaded into memory (but after global variables are initialized, of course.)
	FModuleManager::LoadModuleChecked<FHttpModule>("HTTP");

	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");

	if (SettingsModule != nullptr)
	{
		ISettingsSectionPtr SettingsSection = SettingsModule->RegisterSettings("Project", "Plugins", "CognitiveVR",
			LOCTEXT("DisplayName", "CognitiveVR Analytics"),
			LOCTEXT("Description", "Configure the CognitiveVR Analytics plug-in."),
			GetMutableDefault<UCognitiveVRSettings>()
		);
	}
}

void FCognitiveVRAnalytics::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	// unregister settings
	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");

	if (SettingsModule != nullptr)
	{
		SettingsModule->UnregisterSettings("Project", "Plugins", "CognitiveVR");
	}
}


IMPLEMENT_MODULE(FCognitiveVRAnalytics, "CognitiveVRAnalytics")

/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/

namespace cognitivevrapi
{
	void InitCallback(CognitiveVRResponse resp)
	{
		Log::Info("CognitiveVR InitCallback Response");

		if (!resp.IsSuccessful())
		{
			cognitivevrapi::ThrowDummyResponseException("Failed to initialize CognitiveVR: " + resp.GetErrorMessage());
		}

		FJsonObject json = resp.GetContent();

		CognitiveVR *cog = FCognitiveVRAnalytics::Get().CognitiveVR();
		cog->transaction = new Transaction(cog);
		cog->tuning = new Tuning(cog, json);
		cog->thread_manager = new BufferManager(cog->network);
		cog->core_utils = new CoreUtilities(cog);
	}

    void CognitiveVR::InitNetwork(HttpInterface* httpint)
    {
		this->network = new cognitivevrapi::Network(this);
        this->network->Init(httpint, &InitCallback);
    }

    void CognitiveVR::AppendUD(TSharedPtr<FJsonValueArray>& jsonArray)
    {
        if (user_id.IsEmpty())
		{
			FString empty = FString("");
			Util::AppendToJsonArray(jsonArray, empty);
        }
		else
		{
			Util::AppendToJsonArray(jsonArray, user_id);
        }

		if (device_id.IsEmpty())
		{
			FString empty = FString("");
			Util::AppendToJsonArray(jsonArray, empty);
		}
		else
		{
			Util::AppendToJsonArray(jsonArray, device_id);
		}
    }

	CognitiveVRResponse CognitiveVR::HandleResponse(std::string type, CognitiveVRResponse resp)
    {
        if (!resp.IsSuccessful())
		{
            resp.SetErrorMessage("CognitiveVR Error: " + resp.GetErrorMessage());
			Log::Error("CognitiveVRAnalytics::ResponseException!");
        }

        return resp;
    }

	CognitiveVR* Init(std::string user_id, std::string device_id, TSharedPtr<FJsonObject> init_device_properties)
	{
        OverrideHttpInterface* httpint = new OverrideHttpInterface();

		CognitiveVR* s = new CognitiveVR();

		if (user_id.empty() && device_id.empty()) {
			Log::Error("CognitiveVR::Init - A user or device ID is required. Returning un-initialized CognitiveVR");
			return s;
		}

		if (Config::kTuningCacheTtl <= 0) {
			cognitivevrapi::ThrowDummyResponseException("Invalid tuning cache TTL.");
		}

		s->user_id = user_id.c_str();
		s->device_id = device_id.c_str();
		s->initProperties = init_device_properties;

		s->InitNetwork(httpint);

		return s;
    }

	CognitiveVR::~CognitiveVR()
    {
        Log::Info("Freeing CognitiveVR memory.");
        delete thread_manager;
        thread_manager = NULL;

        delete network;
        network = NULL;

        delete transaction;
        transaction = NULL;

        delete tuning;
        tuning = NULL;
        Log::Info("CognitiveVR memory freed.");
    }

    void ThrowDummyResponseException(std::string s)
    {
		CognitiveVRResponse response(false);
        response.SetErrorMessage(s);
        response.SetContent(FJsonObject());
		Log::Error("CognitiveVRAnalytics::ResponseException! " + s);
    }
}