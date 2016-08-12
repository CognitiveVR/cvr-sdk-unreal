/*
** Copyright (c) 2015 Knetik, Inc. All rights reserved.
*/
#ifndef COGNITIVEVR_NETWORK_H_
#define COGNITIVEVR_NETWORK_H_

#include <string>
#include <stdexcept>

#include "CognitiveVRAnalytics.h"

namespace cognitivevrapi
{
    class Network
    {
        private:
			CognitiveVR* s;
            HttpInterface* httpint;
			//void InitCallback(cognitivevrapi::CognitiveVRtResponse callback);

        public:
            Network(CognitiveVR* sp);
            ~Network();

            /** Initialize the networking class used to make calls to the CognitiveVR API and
                send the initial application_init request.

                @param httpint - HttpInterface used for HTTP requests.

                @return Json::Value
            */
			FJsonObject Init(HttpInterface* http);

            /** Make a call to the CognitiveVR API.

                @param std::string sub_path - The path for the API call.
                @param Json::Value content - JSON content sent to CognitiveVR.
                @param std::string context - The context of the call.

                @return CognitiveVRResponse - Object that contains error data or the JSON content.
            */
			CognitiveVRResponse Call(std::string path, TSharedPtr<FJsonValueArray> content, std::string context = "defaultContext", NetworkCallback callback = NULL);

            static CognitiveVRResponse ParseResponse(std::string str_response);
            static std::string InterpretError(int code);
    };
}
#endif  // COGNITIVEVR_NETWORK_H_