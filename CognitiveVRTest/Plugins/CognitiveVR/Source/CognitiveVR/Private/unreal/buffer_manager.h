/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#ifndef COGNITIVEVR_BUFFER_MANAGER_H_
#define COGNITIVEVR_BUFFER_MANAGER_H_

#include "EngineMinimal.h"
#include "Http.h"
#include "CognitiveVR.h"
#include "CognitiveVRProvider.h"

//namespace cognitivevrapi
//{
    class Network;

	//TODO save these transactions in a buffer until a threshold is reached. send a batch of transactions at once, instead of as requested

    class BufferManager
    {
        private:
            Network* network;

        public:
			BufferManager(Network* n);
            ~BufferManager(){}

			/** Begin and end new transaction.
				@param NetworkCallback
				@param std::string sub_path (context)
				@param std::string content (all send data)
			*/
            void PushTask(NetworkCallback callback, std::string sub_path, TSharedPtr<FJsonValueArray> content);
    };
//}
#endif  // COGNITIVEVR_BUFFER_MANAGER_H_