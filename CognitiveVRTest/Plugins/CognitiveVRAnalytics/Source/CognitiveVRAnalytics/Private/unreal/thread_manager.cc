/*
** Copyright (c) 2015 Knetik, Inc. All rights reserved.
*/
#include "thread_manager.h"

namespace cognitivevrapi
{
    ThreadManager::ThreadManager(Network* n)
    {
        this->network = n;
    }

    void DummyCallback(cognitivevrapi::CognitiveVRResponse response)
    {
        //Dummy callback does nothing, just used to signifiy an asynchronous call that wasn't already passed a callback.
    }

    void ThreadManager::PushTask(NetworkCallback callback, std::string sub_path, TSharedPtr<FJsonValueArray> content, std::string context)
    {
        if (callback == NULL) {
            callback = &DummyCallback;
        }

        //Threading is already handled by Unreal Engine, so we simply pass data along to the network class.
        this->network->Call(sub_path, content, context, callback);
    }
}