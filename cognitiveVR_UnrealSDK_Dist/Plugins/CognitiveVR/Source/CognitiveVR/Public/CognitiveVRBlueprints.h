/*
	
*/
#pragma once


#include "CognitiveVR.h"
#include "CognitiveVRProvider.h"
#include "Private/CognitiveVRPrivatePCH.h"
#include "Engine.h"
#include "Classes/AnalyticsBlueprintLibrary.h"
//#include "EngineAnalytics.h"
#include "Runtime/Analytics/Analytics/Public/AnalyticsEventAttribute.h"
//#include "IAnalyticsProvider.h"
//#include "DynamicObject.h"
#include "PlayerTracker.h"
#include "ExitPoll.h"
#include "LatentActions.h"
#include "CognitiveVRBlueprints.generated.h"

//class FExitPoll;

class CognitiveVRResponse;

// FDelayAction
// A simple delay action; counts down and triggers it's output link when the time remaining falls to zero
/*class FExitPollLatentAction : public FPendingLatentAction
{
public:
	float TimeRemaining;
	FName ExecutionFunction;
	int32 OutputLink;
	FWeakObjectPtr CallbackTarget;

	FExitPollLatentAction(float Duration, const FLatentActionInfo& LatentInfo)
		: TimeRemaining(Duration)
		, ExecutionFunction(LatentInfo.ExecutionFunction)
		, OutputLink(LatentInfo.Linkage)
		, CallbackTarget(LatentInfo.CallbackTarget)
	{
	}

	virtual void UpdateOperation(FLatentResponse& Response) override
	{
		TimeRemaining -= Response.ElapsedTime();
		Response.FinishAndTriggerIf(TimeRemaining <= 0.0f, ExecutionFunction, OutputLink, CallbackTarget);
	}

#if WITH_EDITOR
	// Returns a human readable description of the latent operation's current state
	virtual FString GetDescription() const override
	{
		return FString::Printf(*NSLOCTEXT("DelayAction", "DelayActionTime", "Delay (%.3f seconds left)").ToString(), TimeRemaining);
	}
#endif
};*/

UENUM(BlueprintType)
enum class ETuningValueReturn : uint8
{
	Success,
	Failed
};

UENUM(BlueprintType)
enum class EResponseValueReturn : uint8
{
	Success,
	Fail
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
	//UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Player Tracker")
	//static void SendPlayerData();

	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Update")
	static void UpdateDevice(const TArray<FAnalyticsEventAttr>& Attributes);

	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Update")
	static void UpdateUser(const TArray<FAnalyticsEventAttr>& Attributes);

	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics")
	static void RecordSensor(const FString Name, const float Value);

	/** Send request using latent action */
	//UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Exit Poll", meta = (Latent, LatentInfo = "LatentInfo", HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
	//static void GetRequestLatent(const FString Hook, struct FLatentActionInfo LatentInfo);

	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Exit Poll")
	static void GetRequestDelegate(const FString Hook, const FCognitiveExitPollResponse response);

	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Exit Poll")
	static FExitPollQuestionSet GetCurrentExitPollQuestionSet(EResponseValueReturn& Out);

	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Exit Poll")
	static void SendExitPollResponse(const FExitPollResponses Responses);
};
