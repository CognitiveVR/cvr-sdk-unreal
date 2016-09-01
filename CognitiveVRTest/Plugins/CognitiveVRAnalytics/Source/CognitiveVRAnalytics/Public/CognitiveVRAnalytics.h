#pragma once

//#include "Runtime/Analytics/Analytics/Public/Interfaces/IAnalyticsProviderModule.h"

#include <cstdio>
#include <cstddef>
#include <stdexcept>
#include <ctime>
#include <string>

#include "Private/unreal/override_http_interface.h"
#include "Private/util/util.h"
#include "Private/util/config.h"
#include "Private/util/log.h"
#include "Private/network/cognitivevr_response.h"
#include "Private/util/cognitivevr_exception.h"
#include "Private/network/http_interface.h"
#include "Private/unreal/buffer_manager.h"
#include "Private/api/tuning.h"
#include "Private/api/transaction.h"
#include "Private/api/coreutilities.h"
#include "Private/network/network.h"
#include "Json.h"
//#include "util/util.h"

DECLARE_LOG_CATEGORY_EXTERN(CognitiveVR_Log, Log, All);

/**
 * The public interface to this module.  In most cases, this interface is only public to sibling modules
 * within this plugin.
 */

#define COGNITIVEVR_SDK_NAME "unreal"
#define COGNITIVEVR_SDK_VERSION "0.1.0"

//class IAnalyticsProvider;

class FCognitiveVRAnalytics : public IModuleInterface
{
protected:
	cognitivevrapi::CognitiveVR* cognitivevr;
public:
	virtual void Init(std::string user_id, std::string device_id);
	virtual void Init(std::string user_id, std::string device_id, TSharedPtr<FJsonObject> init_device_properties);
	virtual cognitivevrapi::CognitiveVR* CognitiveVR();
	//virtual TSharedPtr<IAnalyticsProvider> CreateAnalyticsProvider(const FAnalytics::FProviderConfigurationDelegate& GetConfigValue) const override;

	/**
	* Singleton-like access to this module's interface.  This is just for convenience!
	* Beware of calling this during the shutdown phase, though.  Your module might have been unloaded already.
	*
	* @return Returns singleton instance, loading the module on demand if needed
	*/
	static inline FCognitiveVRAnalytics& Get()
	{
		return FModuleManager::LoadModuleChecked< FCognitiveVRAnalytics >("CognitiveVRAnalytics");
	}

	/**
	* Checks to see if this module is loaded and ready.  It is only valid to call Get() if IsAvailable() returns true.
	*
	* @return True if the module is loaded and ready to use
	*/
	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("CognitiveVRAnalytics");
	}

	//virtual void Init(std::string customer_id, std::string user_id, std::string device_id, std::string context);
	//virtual cognitivevrapi::CognitiveVR* CognitiveVR();
private:
	mutable TMap<FString, TSharedPtr<IAnalyticsProvider>> Analytics;
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	//private:
	//mutable TMap<FString, TSharedPtr<IAnalyticsProvider>> Analytics;
};

namespace cognitivevrapi
{
	enum Error {
		kErrorSuccess = 0,
		kErrorGeneric = -1,
		kErrorNotInitialized = -2,
		kErrorNotFound = -3,
		kErrorInvalidArgs = -4,
		kErrorMissingId = -5,
		kErrorRequestTimedOut = -6,
		kErrorUnknown = -7
	};

	class Network;
	class Transaction;
	class Tuning;
	class BufferManager;
	class Core;

class CognitiveVR
{
	friend class Transaction;
	friend class Tuning;
	friend class CoreUtilities;

#pragma warning(push)
#pragma warning(disable:4251) //Disable DLL warning that does not apply in this context.
private:



public:

	std::string customer_id;// = "altimagegames59340-unitywanderdemo-test"; //TODO put this somewhere nice - a text file or editor options or something
	FString user_id;
	FString device_id;
	TSharedPtr<FJsonObject> initProperties; //optional properties sent when initializing. platform, ram, etc
	Transaction* transaction;
	Tuning* tuning;
	Network* network;
	BufferManager* thread_manager;
	CoreUtilities* core_utils;

#pragma warning(pop)

	~CognitiveVR();

	//Helper function to append user_id and device_id to API calls.
	void AppendUD(TSharedPtr<FJsonValueArray> &json);

	void InitNetwork(HttpInterface* httpint);
	//void InitCallback(CognitiveVRResponse resp);

	Network* GetNetwork()
	{
		return network;
	}

	CognitiveVRResponse HandleResponse(std::string type, CognitiveVRResponse resp);

	/** Create a new user.

	@param std::string user_id
	@param std::string context

	@return CognitiveVRResponse
	@throws cognitivevr_exception
	*/
	//void NewUser(std::string user_id);

	/** Create a new device.

	@param std::string device_id
	@param std::string context

	@return CognitiveVRResponse
	@throws cognitivevr_exception
	*/
	//void NewDevice(std::string device_id);

	/** Create a new user if it does not already exist.

	@param std::string user_id
	@param std::string context

	@return CognitiveVRResponse
	@throws cognitivevr_exception
	*/
	//void NewUserChecked(std::string user_id);

	/** Create a new device if it does not already exist.

	@param std::string device_id
	@param std::string context

	@return CognitiveVRResponse
	@throws cognitivevr_exception
	*/
	//void NewDeviceChecked(std::string device_id);

	/** Update user state.

	@param std::string user_id
	@param std::string context
	@param Json::Value properties

	@return CognitiveVRResponse
	@throws cognitivevr_exception
	*/
	//void UpdateUserState(std::string user_id, TSharedPtr<FJsonObject> properties);

	/** Update device state.

	@param std::string device_id
	@param std::string context
	@param Json::Value properties

	@return CognitiveVRResponse
	@throws cognitivevr_exception
	*/
	//void UpdateDeviceState(std::string device_id, TSharedPtr<FJsonObject> properties);

	/** Updates collections, used for virtual currencies or collections.

	@param std::string name
	@param double balance
	@param balance_delta
	@param bool is_currency
	@param std::string context
	@param std::string user_id - Optional.
	@param std::string device_id - Optional.

	@return CognitiveVRResponse
	@throws cognitivevr_exception
	*/
	//void UpdateCollection(std::string nname, double nbalance, double nbalance_delta, bool nis_currency);//, std::string user_id = "", std::string device_id = "");

	/** Records purchases, used for real currencies.

	@param std::string transaction_id
	@param double price
	@param std::string currency_code
	@param std::string result
	@param std::string offer_id
	@param std::string point_of_sale
	@param std::string item_name
	@param std::string context
	@param std::string user_id - Optional.
	@param std::string device_id - Optional.

	@return CognitiveVRResponse
	@throws cognitivevr_exception
	*/
	//void RecordPurchase(std::string transaction_id, double price, std::string currency_code, std::string result, std::string offer_id, std::string point_of_sale, std::string item_name);//, std::string user_id = "", std::string device_id = "");

};

/** Initialize the CognitiveVR SDK.
This function must be called first to use any functions provided in the SDK.

NOTE: user_id and device_id are optional, but you must at least pass one or the other.
@param std::string customer_id - Customer ID provided by CognitiveVR.
@param std::string user_id
@param std::string device_id
@param std::string context - Context of this API call.
@param HttpInterface httpint - Optional HttpInterface used for HTTP requests. If one is not passed, it will use the default CurlHttpInterface.

@returns CognitiveVR - An instance of the CognitiveVR API.
@throws cognitivevr_exception
*/
extern CognitiveVR* Init(std::string user_id, std::string device_id, TSharedPtr<FJsonObject> properties);
//extern CognitiveVR* Init(std::string customer_id, std::string user_id, std::string device_id, std::string context, HttpInterface* httpint);

//Helper function to throw CognitiveVRException.
void ThrowDummyResponseException(std::string s);
}

