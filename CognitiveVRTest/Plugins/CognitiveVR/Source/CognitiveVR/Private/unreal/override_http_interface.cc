/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#include "override_http_interface.h"
#include "Engine.h" //only needed for eu_log
#include "CognitiveVRPrivatePCH.h"
#include "CognitiveVR.h"
#include "json.h"

//namespace cognitivevrapi
//{
    OverrideHttpInterface::OverrideHttpInterface()
    {

    }

    void OverrideHttpInterface::OnResponseReceivedAsync(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, NetworkCallback callback)
    {
        FString UE4Str = Response->GetContentAsString();
		std::string content(TCHAR_TO_UTF8(*UE4Str));
		Log::Info("OverrideHttpInterface::OnResponseReceivedAsync - Response: " + content);
        
		CognitiveVRResponse response = Network::ParseResponse(content);
        callback(response);
    }

	//TODO remove this return value. never anything useful!
    std::string OverrideHttpInterface::Post(std::string url, std::string path, std::string headers[], int header_count, std::string stdcontent, long timeout, NetworkCallback callback)
    {
        //Construct URL.
        std::string stdfull_url = url + path;
        FString full_url(stdfull_url.c_str());
        Log::Info("POST URL " + stdfull_url);
        Log::Info("POST CONTENT " + stdcontent);

        FString content(stdcontent.c_str());

        TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();
        FHttpModule::Get().SetHttpTimeout((float)(timeout + 5));

        HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/x-www-form-urlencoded"));

        //Add headers.
        for (int i=0; i<header_count; i++) {
            std::string header = headers[i];

            int del_pos = header.find(":");
            std::string key = header.substr(0, del_pos);
            key = Util::Trim(key);
            std::string value = header.substr(del_pos + 1, header.size());
            value = Util::Trim(value);

            FString uekey(key.c_str());
            FString uevalue(value.c_str());
            HttpRequest->SetHeader(uekey, uevalue);
        }

        HttpRequest->SetURL(full_url);
        HttpRequest->SetVerb(TEXT("POST"));
        HttpRequest->SetContentAsString(content);

		if (callback != NULL) {
			HttpRequest->OnProcessRequestComplete().BindRaw(this, &OverrideHttpInterface::OnResponseReceivedAsync, callback);
		}

		bool process_result = HttpRequest->ProcessRequest();
		
        if (!process_result) {
			Log::Error("OverrideHttpInterface::Post - Process Request Failed!");
        }

		this->http_response = "";
		return this->http_response;
		/*
        if (process_result) {
            double ntimeout = FPlatformTime::Seconds() + timeout;
            double last_tick = FPlatformTime::Seconds();
            while (!this->response_received) {
                double cur_timestamp = FPlatformTime::Seconds();
                double delta =  cur_timestamp - last_tick;

                last_tick = FPlatformTime::Seconds();
                FHttpModule::Get().GetHttpManager().Tick(delta);

                if (cur_timestamp >= ntimeout) {
					UE_LOG(LogTemp, Warning, TEXT("override_http_int.cc::HTTP request timed out"));
                    throw std::runtime_error("HTTP request timed out.");
                }
            }
        }

        if (!this->response_valid || this->http_response.empty()) {
			if (this->http_response.empty())
			{
				//UE_LOG(LogTemp, Warning, TEXT("override_http_int.cc::empty http response"));
			}
			if (!this->response_valid)
			{
				//UE_LOG(LogTemp, Warning, TEXT("override_http_int.cc::invalid http response"));
			}
			
            throw std::runtime_error("Invalid or Empty HTTP response.");
        }

        this->response_received = false;

        return th
		is->http_response;
		*/
    }
//}