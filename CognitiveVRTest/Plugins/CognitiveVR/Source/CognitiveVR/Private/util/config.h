/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#ifndef COGNITIVEVR_CONFIG_H_
#define COGNITIVEVR_CONFIG_H_

#include <string>

namespace cognitivevrapi
{
    class Config {
        public:
			#pragma warning(push)
			#pragma warning(disable:4251) //Disable DLL warning that does not apply in this context.

            //Enables use of async calls.
            //static bool kNetworkEnableThreading;

            //Enables a graceful shutdown for thread task queue, if enabled the async thread will finish all tasks before stopping.
            //static bool kNetworkEnableGracefulShutdown;

            //CognitiveVR API host. -- Only enterprise customers need this. Ex: https://data.cognitivevr.com
            static std::string kNetworkHost;

			//Enable curl verify peer.
			//static bool kNetworkEnableVerifyPeer;

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