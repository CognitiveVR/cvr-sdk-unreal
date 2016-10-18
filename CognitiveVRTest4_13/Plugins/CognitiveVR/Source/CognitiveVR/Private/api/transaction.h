/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/
#ifndef COGNITIVEVR_TRANSACTION_H_
#define COGNITIVEVR_TRANSACTION_H_

#include "Private/CognitiveVRPrivatePCH.h"
#include "Private/util/util.h"

class FAnalyticsProviderCognitiveVR;

class COGNITIVEVR_API Transaction
{
    private:
		FAnalyticsProviderCognitiveVR* s;

    public:
        Transaction(FAnalyticsProviderCognitiveVR* sp);

        /** Begin a new transaction.

			@param std::string category
            @param std::string transaction_id - Optional.
            @param Json::Value properties - Optional.
                
            @throws CognitiveVR_exception
        */
		void Begin(std::string category, TSharedPtr<FJsonObject> properties = NULL, std::string transaction_id = "");
        void BeginPosition(std::string category, FVector Position, TSharedPtr<FJsonObject> properties = NULL, std::string transaction_id = "");

        /** Update an existing transaction.

			@param std::string category
            @param std::string transaction_id - Optional.                
            @param double progress - Optional.
            @param Json::Value properties - Optional.

            @throws CognitiveVR_exception
        */
		void Update(std::string category, TSharedPtr<FJsonObject> properties = NULL, std::string transaction_id = "", double progress = 0);
        void UpdatePosition(std::string category, FVector Position, TSharedPtr<FJsonObject> properties = NULL, std::string transaction_id = "", double progress = 0);

        /** End an existing transaction.

			@param std::string category
            @param std::string transaction_id - Optional.
            @param std::string result - Optional.
            @param std::string user_id - Optional.
            @param std::string device_id - Optional.
            @param Json::Value properties - Optional.

            @throws cognitivevr_exception
        */
		void End(std::string category, TSharedPtr<FJsonObject> properties = NULL, std::string transaction_id = "", std::string result = "");
        void EndPosition(std::string category, FVector Position, TSharedPtr<FJsonObject> properties = NULL, std::string transaction_id = "", std::string result = "");

        /** Begin and end new transaction.

			@param std::string category
            @param std::string transaction_id - Optional.
            @param std::string result - Optional.
            @param Json::Value properties - Optional.

            @throws cognitivevr_exception
        */
		void BeginEnd(std::string category, TSharedPtr<FJsonObject> properties = NULL, std::string transaction_id = "", std::string result = "");
        void BeginEndPosition(std::string category, FVector Position, TSharedPtr<FJsonObject> properties = NULL, std::string transaction_id = "", std::string result = "");
};
#endif  // COGNITIVEVR_TRANSACTION_H_