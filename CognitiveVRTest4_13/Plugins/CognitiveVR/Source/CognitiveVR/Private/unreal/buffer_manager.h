/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#ifndef COGNITIVEVR_BUFFER_MANAGER_H_
#define COGNITIVEVR_BUFFER_MANAGER_H_

#include "CognitiveVR.h"
#include "CognitiveVRPrivatePCH.h"


class Network;
class CognitiveVRResponse;
typedef void(*NetworkCallback)(CognitiveVRResponse);

//TODO save these transactions in a buffer until a threshold is reached. send a batch of transactions at once, instead of as requested

class BufferManager
{
    private:
        Network* network;
		TArray<TSharedPtr<FJsonObject>> batchedJson;

    public:
		BufferManager(Network* n);

		void AddJsonToBatch(TSharedPtr<FJsonObject> json);
		void SendBatch();

        //~BufferManager(){}

		/** Begin and end new transaction.
			@param NetworkCallback
			@param std::string sub_path (context)
			@param std::string content (all send data)
		*/
        void PushTask(NetworkCallback callback, std::string sub_path, TArray<TSharedPtr<FJsonObject>> content);
};
#endif  // COGNITIVEVR_BUFFER_MANAGER_H_