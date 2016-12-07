/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#ifndef COGNITIVEVR_CONFIG_H_
#define COGNITIVEVR_CONFIG_H_

#include <string>
#include "CognitiveVR.h"

namespace cognitivevrapi
{
    class Config {
        public:
			#pragma warning(push)
			#pragma warning(disable:4251) //Disable DLL warning that does not apply in this context.

            //CognitiveVR API host. -- Only enterprise customers need this. Ex: https://data.cognitivevr.com
            static std::string kNetworkHost;

            //SSF app. Ex: isos-personalization
            static std::string kSsfApp;

            //SSF Version. Ex: 4
            static std::string kSsfVersion;

            //SSF Output. Ex: json
            static std::string kSsfOutput;

            //Tuning variable cache time to live, measured in milliseconds. Ex. 900000 - 15 Minutes
            static long kTuningCacheTtl;

            //Network call timeout, measured in seconds. Ex. 5
            static long kNetworkTimeout;
			#pragma warning(pop)
    };
}
#endif  // COGNITIVEVR_CONFIG_H_