/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#ifndef COGNITIVEVR_NETWORK_H_
#define COGNITIVEVR_NETWORK_H_

#include <stdexcept>
#include "CognitiveVRPrivatePCH.h"
#include "cognitivevr_response.h"

//namespace cognitivevrapi
//{
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
            //~Network();

			//FJsonObject InitCallback(CognitiveVRResponse response);

            /** Initialize the networking class used to make calls to the CognitiveVR API and
                send the initial application_init request.

                @param httpint - HttpInterface used for HTTP requests.

                @return Json::Value
            */
			void Init(HttpInterface* http, NetworkCallback callback);

            /** Make a call to the CognitiveVR API.

                @param std::string sub_path - The path for the API call.
                @param Json::Value content - JSON content sent to CognitiveVR.
                @param std::string context - The context of the call.

                @return CognitiveVRResponse - Object that contains error data or the JSON content.
            */
			void Call(std::string path, TSharedPtr<FJsonValueArray> content, NetworkCallback callback = NULL);

            static CognitiveVRResponse ParseResponse(std::string str_response);
            static std::string InterpretError(int code);
    };
//}
#endif  // COGNITIVEVR_NETWORK_H_