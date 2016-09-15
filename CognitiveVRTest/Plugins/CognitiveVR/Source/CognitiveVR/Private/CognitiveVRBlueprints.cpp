/*
	
*/
#include "CognitiveVR.h"
#include "Private/CognitiveVRPrivatePCH.h"

#include "CognitiveVRBlueprints.h"

void UCognitiveVRBlueprints::BeginTransaction(FString Category, FString TransactionID)
{
	
}

//void UCognitiveVRBlueprints::BeginTransactionWithAttributes(FString Category, FString TransactionID, TArray<FAnalyticsEventAttribute>Attributes){}

void UCognitiveVRBlueprints::UpdateTransaction(FString Category, FString TransactionID, float Progress)
{
	
}

void UCognitiveVRBlueprints::EndTransaction(FString Category, FString TransactionID)
{
	
}

//void UCognitiveVRBlueprints::EndTransactionWithAttributes(FString Category, FString TransactionID, TArray<FAnalyticsEventAttribute>Attributes){}

void UCognitiveVRBlueprints::BeginEndTransaction(FString Category)
{
	
}

//void UCognitiveVRBlueprints::BeginEndTransactionWithAttributes(FString Category, TArray<FAnalyticsEventAttribute>Attributes){}

void UCognitiveVRBlueprints::UpdateCollection(FString Name, int Balance, int Change, bool IsCurrency)
{
	
}

FString UCognitiveVRBlueprints::GetTuningValue(FString Key, ETuningValueReturn& Branches)
{
	return FString();
}