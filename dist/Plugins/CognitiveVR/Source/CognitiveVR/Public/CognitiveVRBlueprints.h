/*
	
*/
#pragma once


#include "CognitiveVR.h"
#include "Private/CognitiveVRPrivatePCH.h"
#include "Engine.h"
#include "Classes/AnalyticsBlueprintLibrary.h"
//#include "EngineAnalytics.h"
//#include "Runtime/Analytics/Analytics/Public/AnalyticsEventAttribute.h"
//#include "IAnalyticsProvider.h"
#include "CognitiveVRBlueprints.generated.h"

UENUM(BlueprintType)
enum class ETuningValueReturn : uint8
{
	Success,
	Failed
};
 
UCLASS()
class COGNITIVEVR_API UCognitiveVRBlueprints : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	
	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Transaction")
	static void BeginTransaction(FString Category, FString TransactionID);
	
	/*UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Transaction")
	static void BeginTransactionWithAttributes(FString Category, FString TransactionID, const TArray<FAnalyticsEventAttribute>& Attributes);*/


	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Transaction")
	static void UpdateTransaction(FString Category, FString TransactionID, float Progress);
	

	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Transaction")
	static void EndTransaction(FString Category, FString TransactionID);
	
	/*UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Transaction")
	static void EndTransactionWithAttributes(FString Category, FString TransactionID, const TArray<FAnalyticsEventAttribute>& Attributes);*/


	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Transaction")
	static void BeginEndTransaction(FString Category);

	/*UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Transaction")
	static void BeginEndTransactionWithAttributes(FString Category, const TArray<FAnalyticsEventAttribute>& Attributes);*/
	
	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Collection")
	static void UpdateCollection(FString Name, int Balance, int Change, bool IsCurrency);
	
	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Tuning", Meta = (ExpandEnumAsExecs = "Branches"))
	static FString GetTuningValue(FString Key, ETuningValueReturn& Branches);
};
