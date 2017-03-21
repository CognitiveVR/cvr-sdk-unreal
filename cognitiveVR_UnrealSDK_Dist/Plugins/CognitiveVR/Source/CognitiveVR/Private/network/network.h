/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#ifndef COGNITIVEVR_NETWORK_H_
#define COGNITIVEVR_NETWORK_H_

#include <stdexcept>
#include "CognitiveVR.h"
#include "CognitiveVRPrivatePCH.h"
#include "cognitivevr_response.h"


class FAnalyticsProviderCognitiveVR;
class HttpInterface;
//typedef void(*NetworkCallback)(CognitiveVRResponse);

class Network
{
    private:
		FAnalyticsProviderCognitiveVR* s;
        HttpInterface* httpint;

    public:
        Network(FAnalyticsProviderCognitiveVR* sp);

        /** Initialize the networking class used to make calls to the CognitiveVR API and
            send the initial application_init request.

            @param httpint - HttpInterface used for HTTP requests.
        */

		void Init(HttpInterface* http, NetworkCallback callback);

        /** Make a call to the CognitiveVR API.

            @param std::string sub_path - The path for the API call.
            @param Json::Value content - JSON content sent to CognitiveVR.
			@param NetworkCallback callback - JSON content sent to CognitiveVR.
        */
		void Call(std::string path, TArray<TSharedPtr<FJsonValue>> content, NetworkCallback callback = NULL);
		void Call(std::string sub_path, TSharedPtr<FJsonValueArray> content, NetworkCallback callback = NULL);

        static CognitiveVRResponse ParseResponse(std::string str_response);
        static std::string InterpretError(int32 code);
};

#endif  // COGNITIVEVR_NETWORK_H_