/*
	
*/
#include "CognitiveVR.h"
#include "Private/CognitiveVRPrivatePCH.h"

#include "CognitiveVRBlueprints.h"

using namespace cognitivevrapi;

extern bool bHasSessionStarted;


void UCognitiveVRBlueprints::BeginTransaction(FString Category, FString TransactionID, const TArray<FAnalyticsEventAttr>& Attributes)
{
	FVector HMDPosition;

	TArray<APlayerController*, FDefaultAllocator> controllers;
	GEngine->GetAllLocalPlayerControllers(controllers);
	HMDPosition = controllers[0]->GetPawn()->GetActorTransform().GetLocation();

	UCognitiveVRBlueprints::BeginTransactionPosition(Category, TransactionID, Attributes, HMDPosition);
}

void UCognitiveVRBlueprints::BeginTransactionPosition(FString Category, FString TransactionID, const TArray<FAnalyticsEventAttr>& Attributes, FVector Position)
{
	TSharedPtr<IAnalyticsProvider> Provider = FAnalytics::Get().GetDefaultConfiguredProvider();
	FAnalyticsProviderCognitiveVR* cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Get();
	if (!Provider.IsValid() || !bHasSessionStarted || cog == NULL)
	{
		CognitiveLog::Error("UCognitiveVRBlueprints::BeginTransaction could not get provider!");
		return;
	}

	TSharedPtr<FJsonObject> properties = MakeShareable(new FJsonObject);
	for (int i = 0; i < Attributes.Num(); i++)
	{
		properties.Get()->SetStringField(Attributes[i].Name, Attributes[i].Value);
	}
	
	cog->transaction->Begin(TCHAR_TO_UTF8(*Category), properties, TCHAR_TO_UTF8(*TransactionID));
}

//void UCognitiveVRBlueprints::BeginTransactionWithAttributes(FString Category, FString TransactionID, TArray<FAnalyticsEventAttribute>Attributes){}

void UCognitiveVRBlueprints::UpdateTransaction(FString Category, FString TransactionID, const TArray<FAnalyticsEventAttr>& Attributes, float Progress)
{
	FVector HMDPosition;

	TArray<APlayerController*, FDefaultAllocator> controllers;
	GEngine->GetAllLocalPlayerControllers(controllers);
	HMDPosition = controllers[0]->GetPawn()->GetActorTransform().GetLocation();
	UCognitiveVRBlueprints::UpdateTransactionPosition(Category, TransactionID, Attributes, HMDPosition, Progress);
}

void UCognitiveVRBlueprints::UpdateTransactionPosition(FString Category, FString TransactionID, const TArray<FAnalyticsEventAttr>& Attributes, FVector Position, float Progress)
{
	TSharedPtr<IAnalyticsProvider> Provider = FAnalytics::Get().GetDefaultConfiguredProvider();
	FAnalyticsProviderCognitiveVR* cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Get();
	if (!Provider.IsValid() || !bHasSessionStarted || cog == NULL)
	{
		CognitiveLog::Error("UCognitiveVRBlueprints::UpdateTransaction could not get provider!");
		return;
	}

	TSharedPtr<FJsonObject> properties = MakeShareable(new FJsonObject);
	for (int i = 0; i < Attributes.Num(); i++)
	{
		properties.Get()->SetStringField(Attributes[i].Name, Attributes[i].Value);
	}
	FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider()->transaction->Update(TCHAR_TO_UTF8(*Category), properties, TCHAR_TO_UTF8(*TransactionID),Progress);
}

void UCognitiveVRBlueprints::EndTransaction(FString Category, FString TransactionID, const TArray<FAnalyticsEventAttr>& Attributes)
{
	FVector HMDPosition;

	TArray<APlayerController*, FDefaultAllocator> controllers;
	GEngine->GetAllLocalPlayerControllers(controllers);
	HMDPosition = controllers[0]->GetPawn()->GetActorTransform().GetLocation();
	UCognitiveVRBlueprints::EndTransactionPosition(Category, TransactionID, Attributes, HMDPosition);
}

void UCognitiveVRBlueprints::EndTransactionPosition(FString Category, FString TransactionID, const TArray<FAnalyticsEventAttr>& Attributes, FVector Position)
{
	TSharedPtr<IAnalyticsProvider> Provider = FAnalytics::Get().GetDefaultConfiguredProvider();
	FAnalyticsProviderCognitiveVR* cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Get();
	if (!Provider.IsValid() || !bHasSessionStarted || cog == NULL)
	{
		CognitiveLog::Error("UCognitiveVRBlueprints::EndTransaction could not get provider!");
		return;
	}

	TSharedPtr<FJsonObject> properties = MakeShareable(new FJsonObject);
	for (int i = 0; i < Attributes.Num(); i++)
	{
		properties.Get()->SetStringField(Attributes[i].Name, Attributes[i].Value);
	}
	FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider()->transaction->End(TCHAR_TO_UTF8(*Category), properties, TCHAR_TO_UTF8(*TransactionID));
}

//void UCognitiveVRBlueprints::EndTransactionWithAttributes(FString Category, FString TransactionID, TArray<FAnalyticsEventAttribute>Attributes){}

void UCognitiveVRBlueprints::BeginEndTransaction(FString Category, const TArray<FAnalyticsEventAttr>& Attributes)
{
	FVector HMDPosition;

	TArray<APlayerController*, FDefaultAllocator> controllers;
	GEngine->GetAllLocalPlayerControllers(controllers);
	HMDPosition = controllers[0]->GetPawn()->GetActorTransform().GetLocation();
	UCognitiveVRBlueprints::BeginEndTransactionPosition(Category, Attributes, HMDPosition);
}

void UCognitiveVRBlueprints::BeginEndTransactionPosition(FString Category, const TArray<FAnalyticsEventAttr>& Attributes, FVector Position)
{
	TSharedPtr<IAnalyticsProvider> Provider = FAnalytics::Get().GetDefaultConfiguredProvider();
	FAnalyticsProviderCognitiveVR* cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Get();
	if (!Provider.IsValid() || !bHasSessionStarted || cog == NULL)
	{
		CognitiveLog::Error("UCognitiveVRBlueprints::BeginEndTransaction could not get provider!");
		return;
	}

	TSharedPtr<FJsonObject> properties = MakeShareable(new FJsonObject);
	for (int i = 0; i < Attributes.Num(); i++)
	{
		properties.Get()->SetStringField(Attributes[i].Name, Attributes[i].Value);
	}
	
	FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider()->transaction->BeginEnd(TCHAR_TO_UTF8(*Category),properties);
}

//void UCognitiveVRBlueprints::BeginEndTransactionWithAttributes(FString Category, TArray<FAnalyticsEventAttribute>Attributes){}

void UCognitiveVRBlueprints::UpdateCollection(FString Name, float Balance, float Change, bool IsCurrency)
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

	FString outString;
	if (json.TryGetStringField(Key, outString))
	{
		Branches = ETuningValueReturn::Success;
		return outString;
	}

	Branches = ETuningValueReturn::Failed;
	outString = FString();
	return outString;
}