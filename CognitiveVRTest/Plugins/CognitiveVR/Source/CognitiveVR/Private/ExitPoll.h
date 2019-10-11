// Fill out your copyright notice in the Description page of Project Settings.

//organizes all the question sets, responses and exit poll panel actors

#pragma once

//#include "Engine.h"
//#include "Kismet/BlueprintFunctionLibrary.h"
//#include "ExitPoll.generated.h"
//#include "CognitiveVRBlueprints.h"
#include "PlayerTracker.h"
//#include "Http.h"
#include "ExitPoll.generated.h"


USTRUCT(BlueprintType)
struct FExitPollScaleRange
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
		int32 start;
	UPROPERTY(BlueprintReadOnly)
		int32 end;
};

USTRUCT(BlueprintType)
struct FExitPollMultipleChoice
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
		FString answer;
	UPROPERTY(BlueprintReadOnly)
		bool icon;
};

USTRUCT(BlueprintType)
struct FExitPollQuestion
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
		FString title;
	UPROPERTY(BlueprintReadOnly)
		FString type;

	//voice
	UPROPERTY(BlueprintReadOnly)
		int32 maxResponseLength;

	//scale
	UPROPERTY(BlueprintReadOnly)
		FString minLabel;
	UPROPERTY(BlueprintReadOnly)
		FString maxLabel;
	UPROPERTY(BlueprintReadOnly)
		FExitPollScaleRange range;

	//multple choice
	UPROPERTY(BlueprintReadWrite)
		TArray<FExitPollMultipleChoice> answers;
};

USTRUCT(BlueprintType)
struct FExitPollQuestionSet
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
		FString customerId;
	UPROPERTY(BlueprintReadOnly)
		//question set id
		FString id;
	UPROPERTY(BlueprintReadOnly)
		//question set name
		FString name;
	UPROPERTY(BlueprintReadOnly)
		int32 version;
	UPROPERTY(BlueprintReadOnly)
		FString title;
	UPROPERTY(BlueprintReadOnly)
		FString status;
	UPROPERTY(BlueprintReadOnly)
		TArray<FExitPollQuestion> questions;
};

UENUM(BlueprintType)
enum class EAnswerValueTypeReturn : uint8
{
	Number,
	Bool,
	String, //used for voice
	Null
};

USTRUCT(BlueprintType)
struct FExitPollAnswer
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString type; //question type
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		EAnswerValueTypeReturn AnswerValueType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 numberValue;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool boolValue; //converted to 0 or 1
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString stringValue; //for base64 voice
};

USTRUCT(BlueprintType)
struct FExitPollResponse
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString user;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString questionSetId;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString sessionId;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString hook;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FExitPollAnswer> answers;
	double duration;
};

DECLARE_DYNAMIC_DELEGATE_OneParam(FCognitiveExitPollResponse, FExitPollQuestionSet, QuestionSet);

class COGNITIVEVR_API ExitPoll
{
private:

	static double lastStartTime;
	static FCognitiveExitPollResponse lastResponse;
	static FExitPollQuestionSet currentQuestionSet;
	static FString lastHook;

public:
	FCognitiveExitPollResponse OnExitPollResponse;	
	static FCognitiveExitPollResponse& ResponseDelegate;

	static void MakeQuestionSetRequest(const FString Hook, FCognitiveExitPollResponse& response);
	static void SendQuestionResponse(FExitPollResponse responses);
	static void SendQuestionAnswers(const TArray<FExitPollAnswer>& Answers);
	static void OnResponseReceived(FString ResponseContent, bool successful);
	static FExitPollQuestionSet GetCurrentQuestionSet();
	
};