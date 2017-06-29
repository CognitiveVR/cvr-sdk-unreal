// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/SceneComponent.h"
//#include "Json.h"
#include "Engine/SceneCapture2D.h"
#include "Engine/Texture.h"
#include "Engine/Texture2D.h"
#include "SceneView.h"
#include "Engine/TextureRenderTarget2D.h"
#include "DynamicObject.h"
#include "PlayerTracker.generated.h"

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
};



//DECLARE_DYNAMIC_DELEGATE_OneParam(FExitPollQuestionHookRequestDelegate, FExitPollQuestionSet, QuestionSet);
//static FExitPollQuestionSet ExitPollRequestDelegate;

//DECLARE_DYNAMIC_DELEGATE_OneParam(FOnlineUserImageRetrievedDelegate, UTexture2D*, Texture);

DECLARE_DYNAMIC_DELEGATE_OneParam(FCognitiveExitPollResponse, FExitPollQuestionSet, QuestionSet);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class COGNITIVEVR_API UPlayerTracker : public UActorComponent
{
	GENERATED_BODY()

private:
	float currentTime = 0;
	TArray<TSharedPtr<FJsonObject>> snapshots;
	TArray<TSharedPtr<FJsonObject>> events;

	float maxDistance = 8192;
	int32 jsonEventPart;
	int32 jsonGazePart;

	UMaterial* SceneDepthMat;

	FString materialPath = "/CognitiveVR/DepthPostProcessing";
	TSharedPtr<FAnalyticsProviderCognitiveVR> s;

public:
	FCognitiveExitPollResponse OnExitPollResponse;

	AActor* SceneCaptureActor;

	USceneCaptureComponent2D* sceneCapture;
	bool waitingForDeferred = false;
	int32 framesTillRender = 0;
	FVector captureLocation;
	FRotator captureRotation;

	USceneComponent* transformComponent;

	UPROPERTY(EditAnywhere)
		float PlayerSnapshotInterval = 0.1;
	int32 GazeBatchSize = 100;

	UPlayerTracker();

	virtual void BeginPlay() override;

	void InitializePlayerTracker();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void SendGazeEventDataToSceneExplorer();
	void SendGazeEventDataToSceneExplorer(FString sceneName);

	FVector GetGazePoint(FVector location, FRotator rotator);
	float GetPixelDepth(float minvalue, float maxvalue);

	void AddJsonEvent(FJsonObject* newEvent);

	FString GazeSnapshotsToString();
	FString EventSnapshotsToString();

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};