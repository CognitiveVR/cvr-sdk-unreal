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

UCLASS()
class COGNITIVEVR_API UCognitiveVRBlueprints : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	
	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Custom Events")
	static void SendCustomEvent(FString Name, const TArray<FAnalyticsEventAttr>& Attributes);

	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Custom Events")
	static void SendCustomEventPosition(FString Name, const TArray<FAnalyticsEventAttr>& Attributes, FVector Position);

	/*UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Device")
	static void UpdateDeviceInt(const FString name, const int32 value);
	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Device")
	static void UpdateDeviceFloat(const FString name, const float value);
	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Device")
	static void UpdateDeviceString(const FString name, const FString value);

	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|User")
	static void UpdateUserInt(const FString name, const int32 value);
	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|User")
	static void UpdateUserFloat(const FString name, const float value);
	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|User")
	static void UpdateUserString(const FString name, const FString value);*/

	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Properties")
		static void UpdateSessionInt(const FString name, const int32 value);
	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Properties")
		static void UpdateSessionFloat(const FString name, const float value);
	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Properties")
		static void UpdateSessionString(const FString name, const FString value);

	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics")
		static void SetSessionName(const FString name);

	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics")
	static void RecordSensor(const FString Name, const float Value);

	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Exit Poll")
	static void GetQuestionSet(const FString Hook, const FCognitiveExitPollResponse response);

	UFUNCTION(BlueprintPure, Category = "CognitiveVR Analytics|Exit Poll")
	static FExitPollQuestionSet GetCurrentExitPollQuestionSet();

	//UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Exit Poll")
	//static void SendExitPollResponse(const FExitPollResponses Responses);

	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Exit Poll")
	static void SendExitPollAnswers(const TArray<FExitPollAnswer>& Answers);
};
