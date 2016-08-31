/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
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

				@param std::string category
                @param std::string transaction_id - Optional.
                @param std::string user_id - Optional.
                @param std::string device_id - Optional.
                @param Json::Value properties - Optional.
				@param NetworkCallback - Optional.
                
                @throws CognitiveVR_exception
            */
            void Begin(std::string category, TSharedPtr<FJsonObject> properties = NULL, std::string transaction_id = "", NetworkCallback callback = NULL);

            /** Update an existing transaction.

				@param std::string category
                @param std::string transaction_id - Optional.                
                @param double progress - Optional.
                @param std::string user_id - Optional.
                @param std::string device_id - Optional.
                @param Json::Value properties - Optional.
				@param NetworkCallback - Optional.

                @throws CognitiveVR_exception
            */
            void Update(std::string category, TSharedPtr<FJsonObject> properties = NULL, std::string transaction_id = "", NetworkCallback callback = NULL, double progress = 0);

            /** End an existing transaction.

				@param std::string category
                @param std::string transaction_id - Optional.
                @param std::string result - Optional.
                @param std::string user_id - Optional.
                @param std::string device_id - Optional.
                @param Json::Value properties - Optional.
				@param NetworkCallback - Optional.

                @throws cognitivevr_exception
            */
            void End(std::string category, TSharedPtr<FJsonObject> properties = NULL, std::string transaction_id = "", NetworkCallback callback = NULL, std::string result = "");

            /** Begin and end new transaction.

				@param std::string category
                @param std::string transaction_id - Optional.
                @param std::string result - Optional.
                @param std::string user_id - Optional.
                @param std::string device_id - Optional.
                @param Json::Value properties - Optional.
				@param NetworkCallback - Optional.

                @throws cognitivevr_exception
            */
            void BeginEnd(std::string category, TSharedPtr<FJsonObject> properties = NULL, std::string transaction_id = "", NetworkCallback callback = NULL,  std::string result = "");
    };
}
#endif  // COGNITIVEVR_TRANSACTION_H_