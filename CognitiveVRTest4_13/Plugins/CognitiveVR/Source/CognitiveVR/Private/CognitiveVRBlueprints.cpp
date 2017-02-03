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
	//HMDPosition = controllers[0]->GetPawn()->GetActorTransform().GetLocation();
	HMDPosition = controllers[0]->GetPawn()->FindComponentByClass<UCameraComponent>()->ComponentToWorld.GetTranslation();

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
		if (Attributes[i].Name.Len() > 0)
		{
			properties.Get()->SetStringField(Attributes[i].Name, Attributes[i].Value);
		}
	}

	cog->transaction->BeginPosition(TCHAR_TO_UTF8(*Category), Position, properties, TCHAR_TO_UTF8(*TransactionID));
}

void UCognitiveVRBlueprints::UpdateTransaction(FString Category, FString TransactionID, const TArray<FAnalyticsEventAttr>& Attributes, float Progress)
{
	FVector HMDPosition;

	TArray<APlayerController*, FDefaultAllocator> controllers;
	GEngine->GetAllLocalPlayerControllers(controllers);
	//HMDPosition = controllers[0]->GetPawn()->GetActorTransform().GetLocation();
	HMDPosition = controllers[0]->GetPawn()->FindComponentByClass<UCameraComponent>()->ComponentToWorld.GetTranslation();
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
		if (Attributes[i].Name.Len() > 0)
		{
			properties.Get()->SetStringField(Attributes[i].Name, Attributes[i].Value);
		}
	}

	cog->transaction->UpdatePosition(TCHAR_TO_UTF8(*Category), Position, properties, TCHAR_TO_UTF8(*TransactionID), Progress);
}

void UCognitiveVRBlueprints::EndTransaction(FString Category, FString TransactionID, const TArray<FAnalyticsEventAttr>& Attributes)
{
	FVector HMDPosition;

	TArray<APlayerController*, FDefaultAllocator> controllers;
	GEngine->GetAllLocalPlayerControllers(controllers);
	//HMDPosition = controllers[0]->GetPawn()->GetActorTransform().GetLocation();
	HMDPosition = controllers[0]->GetPawn()->FindComponentByClass<UCameraComponent>()->ComponentToWorld.GetTranslation();
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
		if (Attributes[i].Name.Len() > 0)
		{
			properties.Get()->SetStringField(Attributes[i].Name, Attributes[i].Value);
		}
	}

	cog->transaction->EndPosition(TCHAR_TO_UTF8(*Category), Position, properties, TCHAR_TO_UTF8(*TransactionID));
}

void UCognitiveVRBlueprints::BeginEndTransaction(FString Category, const TArray<FAnalyticsEventAttr>& Attributes)
{
	FVector HMDPosition;

	TArray<APlayerController*, FDefaultAllocator> controllers;
	GEngine->GetAllLocalPlayerControllers(controllers);
	//HMDPosition = controllers[0]->GetPawn()->GetActorTransform().GetLocation();
	HMDPosition = controllers[0]->GetPawn()->FindComponentByClass<UCameraComponent>()->ComponentToWorld.GetTranslation();
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
		if (Attributes[i].Name.Len() > 0)
		{
			properties.Get()->SetStringField(Attributes[i].Name, Attributes[i].Value);
		}
	}
	
	cog->transaction->BeginEndPosition(TCHAR_TO_UTF8(*Category),Position,properties);
}

void UCognitiveVRBlueprints::UpdateCollection(FString Name, float Balance, float Change, bool IsCurrency)
{
	TSharedPtr<IAnalyticsProvider> Provider = FAnalytics::Get().GetDefaultConfiguredProvider();
	FAnalyticsProviderCognitiveVR* cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Get();
	if (!Provider.IsValid() || !bHasSessionStarted || cog == NULL)
	{
		CognitiveLog::Error("UCognitiveVRBlueprints::UpdateCollection could not get provider!");
		return;
	}

	cog->core_utils->UpdateCollection(TCHAR_TO_UTF8(*Name), Balance, Change, IsCurrency);
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
	
	CognitiveVRResponse response = cog->tuning->GetValue(TCHAR_TO_UTF8(*Key),"default");

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

void UCognitiveVRBlueprints::SendPlayerData()
{
	UPlayerTracker::RequestSendData();
}