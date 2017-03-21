/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#include "buffer_manager.h"

using namespace cognitivevrapi;

BufferManager::BufferManager(Network* n)
{
    this->network = n;

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

void BufferManager::AddJsonToBatch(TSharedPtr<FJsonObject> json)
{
	CognitiveLog::Info("======add json to batch");
	batchedJson.Add(json);
	if (batchedJson.Num() >= TransactionBatchSize)
	{
		BufferManager::SendBatch();
	}
}

void BufferManager::SendBatch()
{
	if (network == NULL)
	{
		CognitiveLog::Warning("BufferManager::SendBatch network is null");
		return;
	}

	BufferManager::PushTask(NULL, "datacollected_batch", batchedJson);

	batchedJson.Empty();
}

void BufferManager::PushTask(NetworkCallback callback, std::string sub_path, TArray<TSharedPtr<FJsonObject>> content)
{
	if (this->network != NULL)
	{
		//TODO cache events and send them together
		//TODO cache events sent before Init called

		//turn all the content into one long string

		TArray< TSharedPtr<FJsonValue> > ObjArray;
		for (int i = 0; i < content.Num(); i++)
		{
			//ObjArray.Emplace(content[i]);

			//TArray<TSharedPtr<FJsonValue>> ValueArray = json.Get()->AsArray();
			TSharedPtr<FJsonValue> tempVal = MakeShareable(new FJsonValueObject(content[i]));
			ObjArray.Emplace(tempVal);

			//json = MakeShareable(new FJsonValueArray(ValueArray));
		}

		TSharedPtr<FJsonValueArray> jsonArray = MakeShareable(new FJsonValueArray(ObjArray));

		//Threading is already handled by Unreal Engine, so we simply pass data along to the network class.
		this->network->Call(sub_path, jsonArray, callback);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("BufferManager::Network is null - probably not initialized yet!"));
	}
}