/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#include "network.h"

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
        Log::Info("Network init.");

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

		//s->initProperties.Get()

		TSharedPtr<FJsonObject>deviceProperties = Util::DeviceScraper(s->initProperties);
		Util::AppendToJsonArray(jsonArray, deviceProperties);
		//Util::AppendToJsonArray(jsonArray, empty);

		Network::Call("application_init", jsonArray, callback);
    }

	//'application_init' and json'd sendtimestamp, eventtimestamp, userid, deviceid, userprops, deviceprops
	void Network::Call(std::string sub_path, TSharedPtr<FJsonValueArray> content, NetworkCallback callback)
    {
        if(!this->httpint) {
			UE_LOG(LogTemp, Warning, TEXT("Network::Call NO HTTP IMPLEMENTATION"));
            cognitivevrapi::ThrowDummyResponseException("No HTTP implementation available. Did you call cognitivevr::Init()?");
        }

        std::string path = "/" + Config::kSsfApp + "/ws/interface/" + sub_path;

        //Build query string.
        std::string query = "?";
        query.append("&ssf_output=");
        query.append(Config::kSsfOutput);
        query.append("&ssf_cust_id=");
        query.append(s->customer_id);
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

		//cognitivevrapi::FastWriter fast_writer;
        std::string str_response = "";

        try {

			//UE_LOG(LogTemp, Warning, TEXT("Network::Call try POST"));

			//unity
			//uri https://data.cognitivevr.io/isos-personalization/ws/interface/application_init?ssf_ws_version=4&ssf_cust_id=altimagegames59340-unitywanderdemo-test&ssf_output=json&ssf_sdk=unity&ssf_sdk_version=0.2.1
			//senddata [1468862637.06639,1468862637.06639,null,null,null,{"cvr.app.name":"InitDataSend","cvr.app.version":"1.0","cvr.unity.version":"5.3.5f1","cvr.device.model":"System Product Name (System manufacturer)","cvr.device.type":"Desktop","cvr.device.platform":"WindowsEditor","cvr.device.os":"Windows 10  (10.0.0) 64bit","cvr.device.graphics.name":"NVIDIA GeForce GTX 970","cvr.device.graphics.type":"Direct3D11","cvr.device.graphics.vendor":"NVIDIA","cvr.device.graphics.version":"Direct3D 11.0 [level 11.0]","cvr.device.graphics.memory":4007,"cvr.device.processor":"Intel(R) Core(TM) i7-6700 CPU @ 3.40GHz","cvr.device.memory":16300,"cvr.vr.enabled":false,"cvr.vr.display.model":"","cvr.vr.display.family":""}]

			//this
			//https://data.cognitivevr.io/isos-personalization/ws/interface/application_init?&ssf_output=json&ssf_cust_id=altimagegames59340-unitywanderdemo-test&ssf_ws_version=4&ssf_sdk=cpp&ssf_sdk_version=5.2.6&ssf_sdk_contextname=defaultContext
			//std::string data = "[1468862637.06639, 1468862637.06639, null, null, null, { \"cvr.app.name\":\"InitDataSend\",\"cvr.app.version\" : \"1.0\",\"cvr.cpp.version\" : \"2014\" }]";

			std::cout << "Netwerk.c Call network host: " << Config::kNetworkHost << std::endl; //https://data.cognitivevr.io
			std::cout << "Netwerk.c Call network path: " << path << std::endl; // /isos-personalization/ws/interface/application_init
			std::cout << "Netwerk.c Call network query: " << query << std::endl; // ?&ssf_output=json&ssf_cust_id=altimagegames59340-unitywanderdemo-test&ssf_ws_version=4&ssf_sdk=cpp&ssf_sdk_version=5.2.6&ssf_sdk_contextname=defaultContext
			std::cout << "Netwerk.c Call network header 1: " << headers[0] << std::endl; //ssf-use-positional-post-params: true
			std::cout << "Netwerk.c Call network header 2: " << headers[1] << std::endl; //ssf-contents-not-url-encoded: true

			//[1234,1234,"","deviceid","",""]

			//std::string hardcode = "[\"1470782585\",\"1470782585\",null,\"deviceid\",\"testcategory\",\"TXN\",31,\"testtransaction\",{\"propertyname\":600,\"propertyname2\":\"value\"}]";

			FString OutputString;		
			TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
			//TSharedRef< TJsonWriter<TCHAR, TCondensedJsonPrintPolicy > > JsonWriter = TJsonWriterFactory<tchar, tcondesedjsonprintpolicy = "">::Create(&OutputString);
			FJsonSerializer::Serialize(content.Get()->AsArray(), Writer);

			//FString hardCodeFS = hardcode.c_str();

			UE_LOG(LogTemp, Warning, TEXT("%s"), *OutputString);
			//UE_LOG(LogTemp, Warning, TEXT("___%s____"), *hardCodeFS);

			std::cout << "Netwerk.c Call network timeout: " << Config::kNetworkTimeout << std::endl;

			//str_response = this->httpint->Post(Config::kNetworkHost, path + query, headers, 2, hardcode, Config::kNetworkTimeout, callback);
			str_response = this->httpint->Post(Config::kNetworkHost, path + query, headers, 2, TCHAR_TO_UTF8(*OutputString), Config::kNetworkTimeout, callback);
        } catch (std::runtime_error e) {
            std::string err = e.what();

			std::cout << "Netwerk.c Call response error: " << ".....something went wrong in post" << std::endl;
			UE_LOG(LogTemp, Warning, TEXT("Network::Call catch %s"), UTF8_TO_TCHAR(err.c_str()));
            cognitivevrapi::ThrowDummyResponseException("Network Error: " + err); //EXCEPTION HERE
        }
		//UE_LOG(LogTemp, Warning, TEXT("Network::Response pre parse"));
		//CognitiveVRResponse resp = Network::ParseResponse(str_response);
		//CognitiveVRResponse resp(false);
		//UE_LOG(LogTemp, Warning, TEXT("Network::return parsed Response"));
        //return resp;
    }

	CognitiveVRResponse Network::ParseResponse(std::string str_response)
    {
		//CognitiveVRResponse response(false);
		//return response;

		TSharedPtr<FJsonObject> root;
		TSharedRef<TJsonReader<>> reader = TJsonReaderFactory<>::Create(str_response.c_str());
		
		//UE_LOG(LogTemp, Warning, TEXT("Network parse response::%s"), str_response.c_str());

		if (FJsonSerializer::Deserialize(reader,root))
		{
			if (Config::kDebugLog) {
				//Log::Info("RESPONSE: " + root.toStyledString());
			}

			int error_code = root.Get()->GetIntegerField("error");
			bool success = (error_code == kErrorSuccess);

			CognitiveVRResponse response(success);

			if (!success) {
				std::string err = Network::InterpretError(error_code);
				Log::Error(err);
				response.SetErrorMessage(err);
				UE_LOG(LogTemp, Warning, TEXT("Network::failure to parse response"));
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
					UE_LOG(LogTemp, Warning, TEXT("Network::found data field"));
				}
				else
				{
					response.SetContent(*root.Get());
					UE_LOG(LogTemp, Warning, TEXT("Network::did not find data field"));
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
		else {
			CognitiveVRResponse response(false);
			response.SetErrorMessage("Failed to parse JSON response.");
			return response;
			Log::Error("Failed to parse JSON response.");
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