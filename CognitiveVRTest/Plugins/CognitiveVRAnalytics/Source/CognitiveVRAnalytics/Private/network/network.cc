/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#include "network.h"
#include "CognitiveVRAnalyticsPrivatePCH.h"
#if WITH_EDITOR
#include "CognitiveVRSettings.h"
#endif

namespace cognitivevrapi
{
    Network::Network(CognitiveVR* sp)
    {
        s = sp;
    }

    Network::~Network()
    {
        cognitivevrapi::Log::Info("Freeing network memory.");
        delete httpint;
        httpint = NULL;
    }

	void Network::Init(HttpInterface* a, NetworkCallback callback)
    {
        this->httpint = a;
        Log::Info("CognitiveVR::Network - Init");

		//applicaiton init

		TArray< TSharedPtr<FJsonValue> > ObjArray;
		TSharedPtr<FJsonValueArray> jsonArray = MakeShareable(new FJsonValueArray(ObjArray));
		TSharedPtr<FJsonObject> json = MakeShareable(new FJsonObject);

		std::string ts = Util::GetTimestampStr();
		FString fs(ts.c_str());
		FString empty;
		
		Util::AppendToJsonArray(jsonArray, fs);
		Util::AppendToJsonArray(jsonArray, fs);
		Util::AppendToJsonArray(jsonArray, s->user_id);
		Util::AppendToJsonArray(jsonArray, s->device_id);
		Util::AppendToJsonArray(jsonArray, empty);

		TSharedPtr<FJsonObject>deviceProperties = Util::DeviceScraper(s->initProperties);
		Util::AppendToJsonArray(jsonArray, deviceProperties);

		Network::Call("application_init", jsonArray, callback);

		//device update

		TSharedPtr<FJsonValueArray> jsonArrayDevice = MakeShareable(new FJsonValueArray(ObjArray));
		TSharedPtr<FJsonObject> jsonDevice = MakeShareable(new FJsonObject);
		Util::AppendToJsonArray(jsonArrayDevice, fs);
		Util::AppendToJsonArray(jsonArrayDevice, fs);
		Util::AppendToJsonArray(jsonArrayDevice, s->user_id);
		Util::AppendToJsonArray(jsonArrayDevice, s->device_id);
		Util::AppendToJsonArray(jsonArrayDevice, deviceProperties);


		Network::Call("datacollector_updateDeviceState", jsonArrayDevice, NULL);
		//s->thread_manager->PushTask(NULL, "datacollector_updateDeviceState", jsonArray);
    }

	//'application_init' and json'd sendtimestamp, eventtimestamp, userid, deviceid, userprops, deviceprops
	void Network::Call(std::string sub_path, TSharedPtr<FJsonValueArray> content, NetworkCallback callback)
    {
        if(!this->httpint)
		{
			Log::Warning("Network::Call - No HTTP implementation available. Did you call cognitivevr::Init()?");
        }

		//UCognitiveVRSettings* Settings = GetMutableDefault<UCognitiveVRSettings>();

		FString ValueReceived;

		GConfig->GetString(
			TEXT("Analytics"),
			TEXT("CognitiveVRApiKey"),
			ValueReceived,
			GGameIni
		);

		std::string customerid(TCHAR_TO_UTF8(*ValueReceived));

		//Log::Info("network::call api key from ini = "+ customerid);

		//return;

		//FString ReadApiKey = FAnalytics::Get().GetConfigValueFromIni(GetIniName(), GetReleaseIniSection(), TEXT("CognitiveVRApiKey"), true);

		//std::string customerid(TCHAR_TO_UTF8(*ValueReceived));

        std::string path = "/" + Config::kSsfApp + "/ws/interface/" + sub_path;

        //Build query string.
        std::string query = "?";
        query.append("&ssf_output=");
        query.append(Config::kSsfOutput);
        query.append("&ssf_cust_id=");
		query.append(customerid);
        query.append("&ssf_ws_version=");
        query.append(Config::kSsfVersion);
        query.append("&ssf_sdk=");
        query.append(COGNITIVEVR_SDK_NAME);
        query.append("&ssf_sdk_version=");
        query.append(COGNITIVEVR_SDK_VERSION);
        query.append("&ssf_sdk_contextname=");
        query.append("defaultContext"); //context is rarely/never used?

        std::string headers[] = {
            "ssf-use-positional-post-params: true",
            "ssf-contents-not-url-encoded: true"
        };

        std::string str_response = "";


		FString OutputString;
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
		FJsonSerializer::Serialize(content.Get()->AsArray(), Writer);

		str_response = this->httpint->Post(Config::kNetworkHost, path + query, headers, 2, TCHAR_TO_UTF8(*OutputString), Config::kNetworkTimeout, callback);

        /*try
		{
			//networkHost https://data.cognitivevr.io
			//path  /isos-personalization/ws/interface/application_init
			//query ?&ssf_output=json&ssf_cust_id=companyname1234-productname-test&ssf_ws_version=4&ssf_sdk=cpp&ssf_sdk_version=5.2.6&ssf_sdk_contextname=defaultContext
			//header[0] ssf-use-positional-post-params: true
			//header[1] ssf-contents-not-url-encoded: true

			FString OutputString;		
			TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
			FJsonSerializer::Serialize(content.Get()->AsArray(), Writer);

			str_response = this->httpint->Post(Config::kNetworkHost, path + query, headers, 2, TCHAR_TO_UTF8(*OutputString), Config::kNetworkTimeout, callback);
        } catch (std::runtime_error e)
		{
            std::string err = e.what();
            cognitivevrapi::ThrowDummyResponseException("Network Error: " + err);
        }*/
    }

	CognitiveVRResponse Network::ParseResponse(std::string str_response)
    {
		TSharedPtr<FJsonObject> root;
		TSharedRef<TJsonReader<>> reader = TJsonReaderFactory<>::Create(str_response.c_str());

		if (FJsonSerializer::Deserialize(reader,root))
		{
			int error_code = root.Get()->GetIntegerField("error");
			bool success = (error_code == kErrorSuccess);

			CognitiveVRResponse response(success);

			if (!success) {
				std::string err = Network::InterpretError(error_code);
				Log::Error(err);
				response.SetErrorMessage(err);
				Log::Warning("Network::ParseResponse - Failed");
			}
			else {
				if (root.Get()->GetObjectField("data").IsValid())
				{
					/*
					{
						"error":0,
						"description" : "",
						"data" :
						{
							"userid":"test username",
							"usertuning" :
							{
								"Hungry":"true",
								"FavouriteFood" : "Burritos",
							},
								"usernew" : false,
								"deviceid" : "test device id",
								"devicetuning" :
							{
								"Hungry":"true",
								"FavouriteFood" : "Burritos",
							},
							"devicenew" : false
						}
					}
					*/

					response.SetContent(*root.Get()->GetObjectField("data").Get());
				}
				else
				{
					response.SetContent(*root.Get());
				}

				/*
				if (root["data"].isNull()) {
					response.SetContent(root);
				}
				else {
					response.SetContent(root["data"]);
				}*/
			}

			return response;
		}
		else
		{
			CognitiveVRResponse response(false);
			response.SetErrorMessage("Failed to parse JSON response.");
			Log::Error("Network::ParseResponse - Failed to parse JSON response.");
			return response;
		}

		CognitiveVRResponse resp(false);
		resp.SetErrorMessage("Unknown error.");

		return resp;
    }

    std::string Network::InterpretError(int code)
    {
        switch (code) {
            case kErrorSuccess:
                return "Success.";
            case kErrorGeneric:
                return "Generic error.";
            case kErrorNotInitialized:
                return "CognitiveVR not initialized.";
            case kErrorNotFound:
                return "Path not found.";
            case kErrorInvalidArgs:
                return "Invalid arguments or invalid JSON format.";
            case kErrorMissingId:
                return "Missing ID.";
            case kErrorRequestTimedOut:
                return "Request timed out.";
        }

        return "Unknown error.";
    }
}