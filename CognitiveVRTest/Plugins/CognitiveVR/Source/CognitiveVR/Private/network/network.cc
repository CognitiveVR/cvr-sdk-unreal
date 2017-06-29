/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#include "network.h"
#include "AnalyticsSettings.h"
#if WITH_EDITOR
#include "CognitiveVRSettings.h"
#endif

using namespace cognitivevrapi;

Network::Network(FAnalyticsProviderCognitiveVR* sp)
{
    s = sp;
}

/*Network::~Network()
{
	CognitiveLog::Info("Freeing network memory.");
    delete httpint;
    httpint = NULL;
}*/

void Network::Init(HttpInterface* a, NetworkCallback callback)
{
    this->httpint = a;
	CognitiveLog::Info("CognitiveVR::Network - Init");

	//applicaiton init

	TArray< TSharedPtr<FJsonValue> > ObjArray;
	TSharedPtr<FJsonValueArray> jsonArray = MakeShareable(new FJsonValueArray(ObjArray));
	//TSharedPtr<FJsonObject> json = MakeShareable(new FJsonObject);

	FString fs = FString::SanitizeFloat(Util::GetTimestamp());
	FString empty = "";

	//FString user = s->GetUserID();
	//FString device = s->GetDeviceID();

	if (s == NULL)
	{
		CognitiveLog::Warning("CognitiveVR::Network Init cannot find provider");
		return;
	}

	Util::AppendToJsonArray(jsonArray, fs);
	Util::AppendToJsonArray(jsonArray, fs);
	s->AppendUD(jsonArray);
	Util::AppendToJsonArray(jsonArray, empty);

	if (s->initProperties.Get() == NULL)
	{
		CognitiveLog::Warning("CognitiveVR::Network Init initproperties is null");
		return;
	}
	TSharedPtr<FJsonObject>deviceProperties = Util::DeviceScraper(s->initProperties);

	Util::AppendToJsonArray(jsonArray, deviceProperties);

	//TODO application_init returns invalid responses?

	TArray<TSharedPtr<FJsonValue>> arr = jsonArray.Get()->AsArray();

	Network::DirectCall("application_init", arr, callback);

	//TSharedPtr<FJsonValueArray> jsonArrayDevice = MakeShareable(new FJsonValueArray(ObjArray));
	//TSharedPtr<FJsonObject> jsonDevice = MakeShareable(new FJsonObject);
	//Util::AppendToJsonArray(jsonArrayDevice, fs);
	//Util::AppendToJsonArray(jsonArrayDevice, fs);
	//s->AppendUD(jsonArrayDevice);
	//Util::AppendToJsonArray(jsonArrayDevice, deviceProperties);

	//Network::Call("datacollector_updateDeviceState", jsonArrayDevice, NULL);
	

	/*
	TSharedPtr<FJsonValueArray> jsonArrayDevice = MakeShareable(new FJsonValueArray(ObjArray));
	TSharedPtr<FJsonObject> jsonDevice = MakeShareable(new FJsonObject);
	Util::AppendToJsonArray(jsonArrayDevice, fs);
	Util::AppendToJsonArray(jsonArrayDevice, fs);
	s->AppendUD(jsonArrayDevice);
	Util::AppendToJsonArray(jsonArrayDevice, deviceProperties);

	Network::Call("datacollector_updateDeviceState", jsonArrayDevice, NULL);*/
}

void Network::Call(std::string sub_path, TSharedPtr<FJsonValueArray> content, NetworkCallback callback)
{
	const TArray<TSharedPtr<FJsonValue>> arr = content.Get()->AsArray();

	Network::Call(sub_path, arr, callback);
}

//'application_init' and json'd sendtimestamp, eventtimestamp, userid, deviceid, userprops, deviceprops
void Network::Call(std::string sub_path, TArray<TSharedPtr<FJsonValue>> content, NetworkCallback callback)
{
    if(!this->httpint)
	{
		CognitiveLog::Warning("Network::Call - No HTTP implementation available. Did you call cognitivevr::Init()?");
    }

	FString ValueReceived;

	bool moduleIsAvailable = FAnalytics::IsAvailable();
	if (!moduleIsAvailable)
	{
		CognitiveLog::Warning("Network::Call - analyticsModule is not available");
		return;
	}

	ValueReceived = s->CustomerId;// FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "Analytics", "CognitiveVRApiKey", false);

	std::string customerid(TCHAR_TO_UTF8(*ValueReceived));

	std::string path = "/" + Config::kSsfApp + "/ws/interface/datacollector_batch";// +sub_path;

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
	FJsonSerializer::Serialize(content, Writer);

	OutputString = "[" + FString::SanitizeFloat(Util::GetTimestamp()) + "," + OutputString + "]";

	str_response = this->httpint->Post(Config::kNetworkHost, path + query, headers, 2, TCHAR_TO_UTF8(*OutputString), Config::kNetworkTimeout, callback);
}

//'application_init' and json'd sendtimestamp, eventtimestamp, userid, deviceid, userprops, deviceprops
void Network::DirectCall(std::string sub_path, TArray<TSharedPtr<FJsonValue>> content, NetworkCallback callback)
{
	if (!this->httpint)
	{
		CognitiveLog::Warning("Network::Call - No HTTP implementation available. Did you call cognitivevr::Init()?");
	}

	FString ValueReceived;

	bool moduleIsAvailable = FAnalytics::IsAvailable();
	if (!moduleIsAvailable)
	{
		CognitiveLog::Warning("Network::Call - analyticsModule is not available");
		return;
	}

	ValueReceived = s->CustomerId;// = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "Analytics", "CognitiveVRApiKey", false);

	std::string customerid(TCHAR_TO_UTF8(*ValueReceived));

	std::string path = "/" + Config::kSsfApp + "/ws/interface/"+sub_path;

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
	//query.append("&ssf_sdk_contextname=");
	//query.append("defaultContext"); //context is rarely/never used?

	std::string headers[] = {
		"ssf-use-positional-post-params: true",
		"ssf-contents-not-url-encoded: true"
	};

	std::string str_response = "";

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(content, Writer);

	//OutputString = "[" + FString::SanitizeFloat(Util::GetTimestamp()) + "," + OutputString + "]";

	str_response = this->httpint->Post(Config::kNetworkHost, path + query, headers, 2, TCHAR_TO_UTF8(*OutputString), Config::kNetworkTimeout, callback);
}

CognitiveVRResponse Network::ParseResponse(std::string str_response)
{
	TSharedPtr<FJsonObject> root;
	TSharedRef<TJsonReader<>> reader = TJsonReaderFactory<>::Create(str_response.c_str());

	if (FJsonSerializer::Deserialize(reader,root))
	{
		//TODO where is this used? what data does it expect with "error" and "data" json fields?
		int32 error_code = root.Get()->GetIntegerField("error");
		bool success = (error_code == kErrorSuccess);

		CognitiveVRResponse response(success);

		if (!success) {
			std::string err = Network::InterpretError(error_code);
			CognitiveLog::Error(err);
			response.SetErrorMessage(err);
			CognitiveLog::Warning("Network::ParseResponse - Failed");
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
		CognitiveLog::Error("Network::ParseResponse - Failed to parse JSON response.");
		return response;
	}

	CognitiveVRResponse resp(false);
	resp.SetErrorMessage("Unknown error.");

	return resp;
}

std::string Network::InterpretError(int32 code)
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