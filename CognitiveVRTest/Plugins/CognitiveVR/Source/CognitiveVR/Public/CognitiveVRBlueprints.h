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
	
	//Begin an extended Transaction with a unique ID
	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Transaction")
	static void BeginTransaction(FString Name, FString TransactionID);

	//Update an extended Transaction with a unique ID
	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Transaction")
	static void UpdateTransaction(FString Name, FString TransactionID, float Progress);
	
	//End extended Transaction with a unique ID
	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Transaction")
	static void EndTransaction(FString Name, FString TransactionID);
	
	


	//Immediately Begin and End Transaction. This is the same as Analytics > RecordEvent
	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Transaction")
	static void BeginEndTransaction(FString Name);
	
	//Update a Collection. Set the new Balance of the Collection, the change of this Collection and if this Collection should be displayed as currency
	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Collection")
	static void UpdateCollection(FString Name, float Balance, float Change, bool IsCurrency);
	
	//Request a Tuning Value by Key. If a Tuning Value is found, it is returned as a String
	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Tuning", Meta = (ExpandEnumAsExecs = "Branches"))
	static FString GetTuningValue(FString Key, ETuningValueReturn& Branches);

	/*UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Transaction")
	static void BeginTransactionWithAttributes(FString Category, FString TransactionID, const TArray<FAnalyticsEventAttribute>& Attributes);*/
	/*UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Transaction")
	static void EndTransactionWithAttributes(FString Category, FString TransactionID, const TArray<FAnalyticsEventAttribute>& Attributes);*/
	/*UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Transaction")
	static void BeginEndTransactionWithAttributes(FString Category, const TArray<FAnalyticsEventAttribute>& Attributes);*/
};
