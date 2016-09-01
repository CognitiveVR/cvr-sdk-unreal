/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#ifndef COGNITIVEVR_UNREAL_HTTP_INTERFACE_H_
#define COGNITIVEVR_UNREAL_HTTP_INTERFACE_H_

#include <string>
#include <stdexcept>

#include "EngineMinimal.h"
#include "Http.h"
#include "Private/util/util.h"
#include "Private/network/http_interface.h"
#include "Private/network/network.h"

namespace cognitivevrapi
{
    class CognitiveVR;

    class OverrideHttpInterface : public HttpInterface
    {
        private:
            bool response_received;
            bool response_valid;
            std::string http_response;
            virtual void OnResponseReceivedAsync(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, NetworkCallback callback);
            //virtual void OnResponseReceivedSync(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

        public:
            OverrideHttpInterface();
            ~OverrideHttpInterface(){}
            virtual std::string Post(std::string url, std::string path, std::string headers[], int header_count, std::string content, long timeout, NetworkCallback callback = NULL);
    };
}
#endif  // COGNITIVEVR_UNREAL_HTTP_INTERFACE_H_