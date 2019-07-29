// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Private/CognitiveVRPrivatePCH.h"
#include "Runtime/Analytics/Analytics/Public/Interfaces/IAnalyticsProviderModule.h"
#include "Private/api/customevent.h"
#include "Private/CognitiveVRProvider.h"

//#include "IAnalyticsProviderModule.h"
//#include "Core.h"
//#include "ModuleManager.h"
//#include "AnalyticsEventAttribute.h"

//DECLARE_LOG_CATEGORY_EXTERN(CognitiveVR_Log, Log, All);
DEFINE_LOG_CATEGORY_STATIC(CognitiveVR_Log, Log, All);

#define COGNITIVEVR_SDK_NAME "unreal"
#define COGNITIVEVR_SDK_VERSION "0.7.3"

class IAnalyticsProvider;
class FAnalyticsProviderCognitiveVR;

/**
	* The public interface to this module
	*/
class FAnalyticsCognitiveVR : public IAnalyticsProviderModule
{
	/** Singleton for analytics */
	TSharedPtr<IAnalyticsProvider> CognitiveVRProvider;

	//--------------------------------------------------------------------------
	// Module functionality
	//--------------------------------------------------------------------------
public:

	/**
		* Singleton-like access to this module's interface.  This is just for convenience!
		* Beware of calling this during the shutdown phase, though.  Your module might have been unloaded already.
		*
		* @return Returns singleton instance, loading the module on demand if needed
		*/
	static inline FAnalyticsCognitiveVR& Get()
	{
		return FModuleManager::LoadModuleChecked< FAnalyticsCognitiveVR >( "CognitiveVR" );
	}

	//--------------------------------------------------------------------------
	// provider factory functions
	//--------------------------------------------------------------------------
public:
	/**
		* IAnalyticsProviderModule interface.
		* Creates the analytics provider given a configuration delegate.
		* The keys required exactly match the field names in the Config object. 
		*/
	virtual TSharedPtr<IAnalyticsProvider> CreateAnalyticsProvider(const FAnalyticsProviderConfigurationDelegate& GetConfigValue) const override;
	virtual TSharedPtr<FAnalyticsProviderCognitiveVR> GetCognitiveVRProvider() const;

	FString DeveloperKey;
		
private:
	//mutable TMap<FString, TSharedPtr<IAnalyticsProvider>> Analytics;
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};