/*
** Copyright (c) 2015 Knetik, Inc. All rights reserved.
*/
#ifndef COGNITIVEVR_CURL_THREAD_MANAGER_H_
#define COGNITIVEVR_CURL_THREAD_MANAGER_H_

#include "EngineMinimal.h"
#include "Http.h"
#include "CognitiveVRAnalytics.h"

namespace cognitivevrapi
{
    class Network;

    class ThreadManager
    {
        private:
            Network* network;

        public:
            ThreadManager(Network* n);
            ~ThreadManager(){}

            void PushTask(NetworkCallback callback, std::string sub_path, TSharedPtr<FJsonValueArray> content, std::string context);
    };
}
#endif  // COGNITIVEVR_CURL_THREAD_MANAGER_H_