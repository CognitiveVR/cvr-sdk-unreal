/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#ifndef COGNITIVEVR_HTTP_INTERFACE_H_
#define COGNITIVEVR_HTTP_INTERFACE_H_

#include <stdexcept>

#include "CognitiveVRPrivatePCH.h"

#include "Private/util/log.h"
#include "Private/network/cognitivevr_response.h"

class FAnalyticsProviderCognitiveVR;

//namespace cognitivevrapi
//{
    class HttpInterface
    {
        protected:
            HttpInterface(){}

        public:
            virtual ~HttpInterface(){}
            //Note: The NetworkCallback callback argument is currently only used for the Unreal Engine, as it does not use the thread manager.
            virtual std::string Post(std::string url, std::string path, std::string headers[], int header_count, std::string content, long timeout, NetworkCallback callback = NULL) = 0;
    };
//}
#endif  // COGNITIVEVR_HTTP_INTERFACE_H_