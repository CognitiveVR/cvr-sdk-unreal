/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#ifndef COGNITIVEVR_TRANSACTION_H_
#define COGNITIVEVR_TRANSACTION_H_

#include "Private/CognitiveVRPrivatePCH.h"
#include "Private/util/util.h"

class FAnalyticsProviderCognitiveVR;

class COGNITIVEVR_API CustomEvent
{
    private:
		FAnalyticsProviderCognitiveVR* s;
		TArray<TSharedPtr<FJsonObject>> batchedJson;
		int32 jsonEventPart = 1;
		int32 CustomEventBatchSize = 16;

		TArray<TSharedPtr<FJsonObject>> events;

    public:
		CustomEvent(FAnalyticsProviderCognitiveVR* sp);

        /** Begin a new transaction.

			@param std::string category
			@param FVector position - Optional.
            @param Json::Value properties - Optional.
			@param std::string transaction_id - Optional.
                
            @throws CognitiveVR_exception
        */
		//void Begin(std::string category, TSharedPtr<FJsonObject> properties = NULL);
        //void BeginPosition(std::string category, FVector Position, TSharedPtr<FJsonObject> properties = NULL, std::string transaction_id = "");

        /** Update an existing transaction.

			@param std::string category
			@param FVector position - Optional.
			@param Json::Value properties - Optional.
            @param std::string transaction_id - Optional.
            @param double progress - Optional.

            @throws CognitiveVR_exception
        */
		//void Update(std::string category, TSharedPtr<FJsonObject> properties = NULL, std::string transaction_id = "", double progress = 0);
        //void UpdatePosition(std::string category, FVector Position, TSharedPtr<FJsonObject> properties = NULL, std::string transaction_id = "", double progress = 0);

        /** End an existing transaction.

			@param std::string category
			@param FVector position - Optional.
			@param Json::Value properties - Optional.
            @param std::string transaction_id - Optional.
            @param std::string result - Optional.

            @throws cognitivevr_exception
        */
		void Send(FString category);
		void Send(FString category, TSharedPtr<FJsonObject> properties);
		void Send(FString category, FVector Position);
		void Send(FString category, FVector Position, TSharedPtr<FJsonObject> properties);


		void SendData();
		void EndSession();

        //void EndPosition(std::string category, FVector Position, TSharedPtr<FJsonObject> properties = NULL, std::string transaction_id = "", std::string result = "");

        /** Begin and end new transaction.

			@param std::string category
			@param FVector position - Optional.
			@param Json::Value properties - Optional.
            @param std::string transaction_id - Optional.
            @param std::string result - Optional.

            @throws cognitivevr_exception
        */
		//void BeginEnd(std::string category, TSharedPtr<FJsonObject> properties = NULL, std::string transaction_id = "", std::string result = "");
        //void BeginEndPosition(std::string category, FVector Position, TSharedPtr<FJsonObject> properties = NULL, std::string transaction_id = "", std::string result = "");
};
#endif  // COGNITIVEVR_TRANSACTION_H_