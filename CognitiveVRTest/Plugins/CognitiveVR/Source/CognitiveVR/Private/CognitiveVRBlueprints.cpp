/*
	
*/
#include "CognitiveVR.h"
#include "Private/CognitiveVRPrivatePCH.h"

#include "CognitiveVRBlueprints.h"

using namespace cognitivevrapi;

extern bool bHasSessionStarted;

void UCognitiveVRBlueprints::BeginTransaction(FString Category, FString TransactionID)
{
	TSharedPtr<IAnalyticsProvider> Provider = FAnalytics::Get().GetDefaultConfiguredProvider();
	FAnalyticsProviderCognitiveVR* cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Get();
	if (!Provider.IsValid() || !bHasSessionStarted || cog == NULL)
	{
		CognitiveLog::Error("UCognitiveVRBlueprints::BeginTransaction could not get provider!");
		return;
	}

	TSharedPtr<FJsonObject> properties = MakeShareable(new FJsonObject);
	
	cog->transaction->Begin(TCHAR_TO_UTF8(*Category), properties, TCHAR_TO_UTF8(*TransactionID));
}

//void UCognitiveVRBlueprints::BeginTransactionWithAttributes(FString Category, FString TransactionID, TArray<FAnalyticsEventAttribute>Attributes){}

void UCognitiveVRBlueprints::UpdateTransaction(FString Category, FString TransactionID, float Progress)
{
	TSharedPtr<IAnalyticsProvider> Provider = FAnalytics::Get().GetDefaultConfiguredProvider();
	FAnalyticsProviderCognitiveVR* cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Get();
	if (!Provider.IsValid() || !bHasSessionStarted || cog == NULL)
	{
		CognitiveLog::Error("UCognitiveVRBlueprints::UpdateTransaction could not get provider!");
		return;
	}

	TSharedPtr<FJsonObject> properties = MakeShareable(new FJsonObject);
	FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider()->transaction->Update(TCHAR_TO_UTF8(*Category), properties, TCHAR_TO_UTF8(*TransactionID),Progress);
}

void UCognitiveVRBlueprints::EndTransaction(FString Category, FString TransactionID)
{
	TSharedPtr<IAnalyticsProvider> Provider = FAnalytics::Get().GetDefaultConfiguredProvider();
	FAnalyticsProviderCognitiveVR* cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Get();
	if (!Provider.IsValid() || !bHasSessionStarted || cog == NULL)
	{
		CognitiveLog::Error("UCognitiveVRBlueprints::EndTransaction could not get provider!");
		return;
	}

	TSharedPtr<FJsonObject> properties = MakeShareable(new FJsonObject);
	FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider()->transaction->End(TCHAR_TO_UTF8(*Category), properties, TCHAR_TO_UTF8(*TransactionID));
}

//void UCognitiveVRBlueprints::EndTransactionWithAttributes(FString Category, FString TransactionID, TArray<FAnalyticsEventAttribute>Attributes){}

void UCognitiveVRBlueprints::BeginEndTransaction(FString Category)
{
	TSharedPtr<IAnalyticsProvider> Provider = FAnalytics::Get().GetDefaultConfiguredProvider();
	FAnalyticsProviderCognitiveVR* cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Get();
	if (!Provider.IsValid() || !bHasSessionStarted || cog == NULL)
	{
		CognitiveLog::Error("UCognitiveVRBlueprints::BeginEndTransaction could not get provider!");
		return;
	}

	TSharedPtr<FJsonObject> properties = MakeShareable(new FJsonObject);
	FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider()->transaction->BeginEnd(TCHAR_TO_UTF8(*Category));
}

//void UCognitiveVRBlueprints::BeginEndTransactionWithAttributes(FString Category, TArray<FAnalyticsEventAttribute>Attributes){}

void UCognitiveVRBlueprints::UpdateCollection(FString Name, int Balance, int Change, bool IsCurrency)
{
	TSharedPtr<IAnalyticsProvider> Provider = FAnalytics::Get().GetDefaultConfiguredProvider();
	FAnalyticsProviderCognitiveVR* cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Get();
	if (!Provider.IsValid() || !bHasSessionStarted || cog == NULL)
	{
		CognitiveLog::Error("UCognitiveVRBlueprints::UpdateCollection could not get provider!");
		return;
	}

	FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider()->core_utils->UpdateCollection(TCHAR_TO_UTF8(*Name), Balance, Change, IsCurrency);
}

FString UCognitiveVRBlueprints::GetTuningValue(FString Key, ETuningValueReturn& Branches)
{
	TSharedPtr<IAnalyticsProvider> Provider = FAnalytics::Get().GetDefaultConfiguredProvider();
	FAnalyticsProviderCognitiveVR* cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Get();
	if (!Provider.IsValid() || !bHasSessionStarted || cog == NULL)
	{
		CognitiveLog::Error("UCognitiveVRBlueprints::GetTuningValue could not get provider!");
		return FString();
	}
	
	CognitiveVRResponse response = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider()->tuning->GetValue(TCHAR_TO_UTF8(*Key),"default");

	if (!response.IsSuccessful())
	{
		CognitiveLog::Warning("FAnalyticsCognitiveVR::GetTuningValue response failed!");
		Branches = ETuningValueReturn::Failed;
		return FString();
	}

	FJsonObject json = response.GetContent();

	return FString();
}