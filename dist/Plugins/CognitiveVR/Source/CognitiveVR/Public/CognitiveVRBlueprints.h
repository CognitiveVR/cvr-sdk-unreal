/*
	
*/
#pragma once


#include "CognitiveVR.h"
#include "Private/CognitiveVRPrivatePCH.h"
#include "Engine.h"
#include "Classes/AnalyticsBlueprintLibrary.h"
//#include "EngineAnalytics.h"
#include "Runtime/Analytics/Analytics/Public/AnalyticsEventAttribute.h"
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
	static void BeginTransaction(FString Name, FString TransactionID, const TArray<FAnalyticsEventAttr>& Attributes);

	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Transaction")
	static void BeginTransactionPosition(FString Name, FString TransactionID, const TArray<FAnalyticsEventAttr>& Attributes, FVector Position);

	//Update an extended Transaction with a unique ID
	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Transaction")
	static void UpdateTransaction(FString Name, FString TransactionID, const TArray<FAnalyticsEventAttr>& Attributes, float Progress);

	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Transaction")
	static void UpdateTransactionPosition(FString Name, FString TransactionID, const TArray<FAnalyticsEventAttr>& Attributes, FVector Position, float Progress);
	
	//End extended Transaction with a unique ID
	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Transaction")
	static void EndTransaction(FString Name, FString TransactionID, const TArray<FAnalyticsEventAttr>& Attributes);

	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Transaction")
	static void EndTransactionPosition(FString Name, FString TransactionID, const TArray<FAnalyticsEventAttr>& Attributes, FVector Position);


	//Immediately Begin and End Transaction. This is the same as Analytics > RecordEvent
	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Transaction")
	static void BeginEndTransaction(FString Name, const TArray<FAnalyticsEventAttr>& Attributes);
	
	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Transaction")
	static void BeginEndTransactionPosition(FString Name, const TArray<FAnalyticsEventAttr>& Attributes, FVector Position);

	//Update a Collection. Set the new Balance of the Collection, the change of this Collection and if this Collection should be displayed as currency
	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Collection")
	static void UpdateCollection(FString Name, float Balance, float Change, bool IsCurrency);
	
	//Request a Tuning Value by Key. If a Tuning Value is found, it is returned as a String
	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Tuning", Meta = (ExpandEnumAsExecs = "Branches"))
	static FString GetTuningValue(FString Key, ETuningValueReturn& Branches);

	//sends player gaze data and player events collected during this level to sceneexplorer.com
	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Player Tracker")
	static void SendPlayerData();
};
