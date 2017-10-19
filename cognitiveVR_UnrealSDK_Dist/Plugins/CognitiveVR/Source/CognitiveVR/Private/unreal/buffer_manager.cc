/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#include "buffer_manager.h"

using namespace cognitivevrapi;

BufferManager::BufferManager(FAnalyticsProviderCognitiveVR* cog)
{
	cvr = cog;
	FString ValueReceived;

	ValueReceived = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "TransactionBatchSize", false);

	if (ValueReceived.Len() > 0)
	{

		int32 transactionLimit = FCString::Atoi(*ValueReceived);
		if (transactionLimit > 0)
		{
			TransactionBatchSize = transactionLimit;
		}
	}
}

void BufferManager::ReleaseNetwork()
{
	//network = NULL;
}

void BufferManager::AddJsonToBatch(TSharedPtr<FJsonObject> json)
{
	if (cvr == nullptr) { return; }

	if (!json.IsValid())
	{
		CognitiveLog::Info("BufferManager::AddJsonToBatch json is empty");
		return;
	}

	/*if (this == NULL)
	{
	CognitiveLog::Error("BufferManager::AddJsonToBatch Buffer Manager is NULL!!");
	return;
	}*/

	batchedJson.Add(json);
	if (batchedJson.Num() >= TransactionBatchSize)
	{
		SendBatch();
	}
}

void BufferManager::SendBatch()
{
	if (cvr == nullptr)
	{
		CognitiveLog::Error("BufferManager::SendBatch could not GetCognitiveVRProvider!");
		return;
	}
	if (cvr->HasStartedSession() == false)
	{
		CognitiveLog::Error("BufferManager::SendBatch CognitiveVRProvider has not started session!");
		return;
	}

	if (batchedJson.Num() == 0)
	{
		CognitiveLog::Warning("BufferManager::SendBatch has no batched json to send");
		return;
	}

	/*if (network == NULL)
	{
	CognitiveLog::Warning("BufferManager::SendBatch network is null");
	return;
	}*/

	BufferManager::PushTask(NULL, "datacollected_batch", batchedJson);

	batchedJson.Empty();
}

void BufferManager::PushTask(NetworkCallback callback, std::string sub_path, TArray<TSharedPtr<FJsonObject>> content)
{
	if (cvr == nullptr) { return; }

	if (cvr->network.IsValid())
	{
		//turn all the content into one long string

		TArray< TSharedPtr<FJsonValue> > ObjArray;
		for (int32 i = 0; i < content.Num(); i++)
		{
			TSharedPtr<FJsonValue> tempVal = MakeShareable(new FJsonValueObject(content[i]));
			ObjArray.Emplace(tempVal);
		}

		TSharedPtr<FJsonValueArray> jsonArray = MakeShareable(new FJsonValueArray(ObjArray));

		//Threading is already handled by Unreal Engine, so we simply pass data along to the network class.
		cvr->network->Call(sub_path, jsonArray, callback);
	}
	else
	{
		CognitiveLog::Error("BufferManager::Network is null - probably not initialized yet!");
	}
}