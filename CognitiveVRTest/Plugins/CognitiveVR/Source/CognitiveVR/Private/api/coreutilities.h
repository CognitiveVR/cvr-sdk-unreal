#ifndef COGNITIVEVR_COREUTILITIES_H_
#define COGNITIVEVR_COREUTILITIES_H_

#include "Private/CognitiveVRPrivatePCH.h"

//setting user/device properties
//TODO implement all the user/device property stuff

class FAnalyticsProviderCognitiveVR;

class COGNITIVEVR_API CoreUtilities
{
private:
	FAnalyticsProviderCognitiveVR* s;

public:
	CoreUtilities(FAnalyticsProviderCognitiveVR* sp);

	/** Create a new user.

	@param FString user_id
	@throws cognitivevr_exception
	*/
	void NewUser(FString user_id);

	/** Create a new device.

	@param FString device_id
	@throws cognitivevr_exception
	*/
	void NewDevice(FString device_id);

	/** Create a new user if it does not already exist.

	@param FString user_id
	@throws cognitivevr_exception
	*/
	void NewUserChecked(FString user_id);

	/** Create a new device if it does not already exist.

	@param FString device_id
	@throws cognitivevr_exception
	*/
	void NewDeviceChecked(FString device_id);

	/** Update user state.

	@param FString user_id
	@param Json::Value properties
	@throws cognitivevr_exception
	*/
	void UpdateUserState(FString user_id, TSharedPtr<FJsonObject> properties);

	/** Update device state.

	@param FString device_id
	@param Json::Value properties
	@throws cognitivevr_exception
	*/
	void UpdateDeviceState(FString device_id, TSharedPtr<FJsonObject> properties);

	/** Updates collections, used for virtual currencies or collections.

	@param FString name
	@param double balance
	@param balance_delta
	@param bool is_currency
	@throws cognitivevr_exception
	*/
	//void UpdateCollection(std::string nname, double nbalance, double nbalance_delta, bool nis_currency);

	/** Records purchases, used for real currencies.

	@param FString transaction_id
	@param double price
	@param FString currency_code
	@param FString result
	@param FString offer_id
	@param FString point_of_sale
	@param FString item_name
	@throws cognitivevr_exception
	*/
	//void RecordPurchase(std::string transaction_id, double price, std::string currency_code, std::string result, std::string offer_id, std::string point_of_sale, std::string item_name);


};
#endif  // COGNITIVEVR_TRANSACTION_H_