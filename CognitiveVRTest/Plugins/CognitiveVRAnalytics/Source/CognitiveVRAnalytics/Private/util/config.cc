/*
** Copyright (c) 2015 Knetik, Inc. All rights reserved.
*/
#include "util/config.h"

namespace cognitivevrapi
{
    //Configuration defaults.
    bool Config::kDebugLog = false;
    bool Config::kNetworkEnableThreading = false;
    bool Config::kNetworkEnableGracefulShutdown = true;
    std::string Config::kNetworkHost = "https://data.cognitivevr.io";
	bool Config::kNetworkEnableVerifyPeer = true;
    std::string Config::kSsfApp = "isos-personalization";
    std::string Config::kSsfVersion = "4";
    std::string Config::kSsfOutput = "json";
    long Config::kTuningCacheTtl = 900000;
    long Config::kNetworkTimeout = 5;
}