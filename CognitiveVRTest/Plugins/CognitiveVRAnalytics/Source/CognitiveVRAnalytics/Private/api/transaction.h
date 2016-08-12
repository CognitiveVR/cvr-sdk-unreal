/*
** Copyright (c) 2015 Knetik, Inc. All rights reserved.
*/
#ifndef COGNITIVEVR_TRANSACTION_H_
#define COGNITIVEVR_TRANSACTION_H_

#include "CognitiveVRAnalytics.h"

namespace cognitivevrapi
{
    class COGNITIVEVRANALYTICS_API Transaction
    {
        private:
			CognitiveVR* s;

        public:
            Transaction(CognitiveVR* sp);

			/*void Begin(FString category, TSharedPtr<FJsonObject> properties, FString transaction_id, int timeout = Config::kNetworkTimeout);
			void End(FString category, TSharedPtr<FJsonObject> properties, FString transaction_id);
			void BeginAndEnd(FString category, TSharedPtr<FJsonObject> properties, FString transaction_id);*/

            /** Begin a new transaction.

                @param std::string transaction_id
                @param std::string category
                @param int timeout
                @param std::string context
                @param std::string user_id - Optional.
                @param std::string device_id - Optional.
                @param Json::Value properties - Optional.

                @return CognitiveVRResponse
                @throws CognitiveVR_exception
            */
            void BeginAsync(NetworkCallback callback, std::string transaction_id, std::string category, int timeout, std::string context, TSharedPtr<FJsonObject> properties, FString user_id = "", FString device_id = "");

            /** Update an existing transaction.

                @param std::string transaction_id
                @param std::string category
                @param double progress
                @param std::string context
                @param std::string user_id - Optional.
                @param std::string device_id - Optional.
                @param Json::Value properties - Optional.

                @return CognitiveVRResponse
                @throws CognitiveVR_exception
            */
            void UpdateAsync(NetworkCallback callback, std::string transaction_id, std::string category, double progress, std::string context, TSharedPtr<FJsonObject> properties, FString user_id = "", FString device_id = "");

            /** End an existing transaction.

                @param std::string transaction_id
                @param std::string category
                @param std::string result
                @param std::string context
                @param std::string user_id - Optional.
                @param std::string device_id - Optional.
                @param Json::Value properties - Optional.

                @return CognitiveVRResponse
                @throws cognitivevr_exception
            */
            void EndAsync(NetworkCallback callback, std::string transaction_id, std::string category, std::string result, std::string context, TSharedPtr<FJsonObject> properties, FString user_id = "", FString device_id = "");

            /** Begin and end new transaction.

                @param std::string transaction_id
                @param std::string category
                @param std::string result
                @param std::string context
                @param std::string user_id - Optional.
                @param std::string device_id - Optional.
                @param Json::Value properties - Optional.

                @return CognitiveVRtResponse
                @throws cognitivevr_exception
            */
            void BeginEndAsync(NetworkCallback callback, std::string transaction_id, std::string category, std::string result, std::string context, TSharedPtr<FJsonObject> properties, FString user_id = "", FString device_id = "");
    };
}
#endif  // COGNITIVEVR_TRANSACTION_H_