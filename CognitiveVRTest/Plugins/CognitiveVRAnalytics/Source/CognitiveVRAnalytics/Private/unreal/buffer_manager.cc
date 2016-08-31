/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#include "buffer_manager.h"

namespace cognitivevrapi
{
    BufferManager::BufferManager(Network* n)
    {
        this->network = n;
    }

    void BufferManager::PushTask(NetworkCallback callback, std::string sub_path, TSharedPtr<FJsonValueArray> content)
    {
		if (this->network != NULL)
		{
			//TODO cache events and send them together

			//Threading is already handled by Unreal Engine, so we simply pass data along to the network class.
			this->network->Call(sub_path, content, callback);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("BufferManager::Network is null - probably not initialized yet!"));
		}
    }
}