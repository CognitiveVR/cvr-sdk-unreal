/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#ifndef COGNITIVEVR_HTTP_INTERFACE_H_
#define COGNITIVEVR_HTTP_INTERFACE_H_

#include <string>
#include <stdexcept>
#include "json.h"

#include "Private/util/log.h"
#include "Private/network/cognitivevr_response.h"

namespace cognitivevrapi
{
    class CognitiveVR;

    class HttpInterface
    {
        protected:
            HttpInterface(){}

        public:
            virtual ~HttpInterface(){}
            //Note: The NetworkCallback callback argument is currently only used for the Unreal Engine, as it does not use the thread manager.
            virtual std::string Post(std::string url, std::string path, std::string headers[], int header_count, std::string content, long timeout, NetworkCallback callback = NULL) = 0;
    };
}
#endif  // COGNITIVEVR_HTTP_INTERFACE_H_