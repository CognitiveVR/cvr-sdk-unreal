/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#ifndef COGNITIVEVR_NETWORK_H_
#define COGNITIVEVR_NETWORK_H_

#include <stdexcept>
#include "CognitiveVR.h"
#include "CognitiveVRPrivatePCH.h"

class FAnalyticsProviderCognitiveVR;
class HttpInterface;

class Network
{
    private:
		FAnalyticsProviderCognitiveVR* s;

    public:
        Network(FAnalyticsProviderCognitiveVR* sp);

		void NetworkCall(FString suburl, FString contents);
		
		//TODO exitpoll with callback should be here
		void NetworkExitPollGet(FString hook);
		void NetworkExitPollPost(FString questionsetname, FString questionsetversion, FString contents);
};

#endif  // COGNITIVEVR_NETWORK_H_