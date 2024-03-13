/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/

#pragma once

#include "Cognitive3D/Public/C3DCommonTypes.h"
#include "UObject/UObjectIterator.h"
#include "CoreMinimal.h"
#include "Cognitive3D/Public/Cognitive3DProvider.h"
#include "Interfaces/IAnalyticsProviderModule.h"
#include "Modules/ModuleManager.h"
#include "Runtime/Engine/Classes/Engine/LevelStreaming.h"

DEFINE_LOG_CATEGORY_STATIC(Cognitive3D_Log, Log, All);

#define Cognitive3D_SDK_NAME "unreal"
#define Cognitive3D_SDK_VERSION "1.0.0"

class IAnalyticsProvider;
class FAnalyticsProviderCognitive3D;

/**
	* The public interface to this module
	*/
class FAnalyticsCognitive3D : public IAnalyticsProviderModule
{
	/** Singleton for analytics */
	TSharedPtr<IAnalyticsProvider> AnalyticsProvider;
	TWeakPtr<FAnalyticsProviderCognitive3D> Cognitive3DProvider;

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
	static inline FAnalyticsCognitive3D& Get()
	{
		return FModuleManager::LoadModuleChecked< FAnalyticsCognitive3D >( "Cognitive3D" );
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
	virtual TWeakPtr<FAnalyticsProviderCognitive3D> GetCognitive3DProvider() const;

private:
	//mutable TMap<FString, TSharedPtr<IAnalyticsProvider>> Analytics;
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};