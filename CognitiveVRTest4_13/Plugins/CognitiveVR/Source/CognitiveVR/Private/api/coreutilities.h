#ifndef COGNITIVEVR_COREUTILITIES_H_
#define COGNITIVEVR_COREUTILITIES_H_

#include "Private/CognitiveVRPrivatePCH.h"

//namespace cognitivevrapi
//{
	class FAnalyticsProviderCognitiveVR;

	class COGNITIVEVR_API CoreUtilities
	{
	private:
		FAnalyticsProviderCognitiveVR* s;

	public:
		CoreUtilities(FAnalyticsProviderCognitiveVR* sp);

		/** Create a new user.

		@param std::string user_id
		@throws cognitivevr_exception
		*/
		void NewUser(std::string user_id);

		/** Create a new device.

		@param std::string device_id
		@throws cognitivevr_exception
		*/
		void NewDevice(std::string device_id);

		/** Create a new user if it does not already exist.

		@param std::string user_id
		@throws cognitivevr_exception
		*/
		void NewUserChecked(std::string user_id);

		/** Create a new device if it does not already exist.

		@param std::string device_id
		@throws cognitivevr_exception
		*/
		void NewDeviceChecked(std::string device_id);

		/** Update user state.

		@param std::string user_id
		@param Json::Value properties
		@throws cognitivevr_exception
		*/
		void UpdateUserState(std::string user_id, TSharedPtr<FJsonObject> properties);

		/** Update device state.

		@param std::string device_id
		@param Json::Value properties
		@throws cognitivevr_exception
		*/
		void UpdateDeviceState(std::string device_id, TSharedPtr<FJsonObject> properties);

		/** Updates collections, used for virtual currencies or collections.

		@param std::string name
		@param double balance
		@param balance_delta
		@param bool is_currency
		@throws cognitivevr_exception
		*/
		void UpdateCollection(std::string nname, double nbalance, double nbalance_delta, bool nis_currency);

		/** Records purchases, used for real currencies.

		@param std::string transaction_id
		@param double price
		@param std::string currency_code
		@param std::string result
		@param std::string offer_id
		@param std::string point_of_sale
		@param std::string item_name
		@throws cognitivevr_exception
		*/
		void RecordPurchase(std::string transaction_id, double price, std::string currency_code, std::string result, std::string offer_id, std::string point_of_sale, std::string item_name);


	};
//}
#endif  // COGNITIVEVR_TRANSACTION_H_