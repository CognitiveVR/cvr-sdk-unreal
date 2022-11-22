// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "C3DCommonTypes.generated.h"

//multicast delegates cannot be static. use static pointer to playertracker instance in BP
//multicast also can't be used as argument in BP function (to implement custom bind function)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCognitiveSessionBegin, bool, Successful);

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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CognitiveVR Analytics")
		FString type; //question type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CognitiveVR Analytics")
		EAnswerValueTypeReturn AnswerValueType = EAnswerValueTypeReturn::Null;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CognitiveVR Analytics")
		int32 numberValue = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CognitiveVR Analytics")
		bool boolValue = false; //converted to 0 or 1
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CognitiveVR Analytics")
		FString stringValue; //for base64 voice
};

USTRUCT(BlueprintType)
struct FExitPollResponse
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CognitiveVR Analytics")
		FString userId;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CognitiveVR Analytics")
		FString participantId;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CognitiveVR Analytics")
		FString questionSetId;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CognitiveVR Analytics")
		FString sessionId;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CognitiveVR Analytics")
		FString hook;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CognitiveVR Analytics")
		TArray<FExitPollAnswer> answers;
	double duration;
};

USTRUCT(BlueprintType)
struct FExitPollScaleRange
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "CognitiveVR Analytics")
		int32 start = 0;
	UPROPERTY(BlueprintReadOnly, Category = "CognitiveVR Analytics")
		int32 end = 0;
};

USTRUCT(BlueprintType)
struct FExitPollMultipleChoice
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "CognitiveVR Analytics")
		FString answer;
	UPROPERTY(BlueprintReadOnly, Category = "CognitiveVR Analytics")
		bool icon = false;
};

USTRUCT(BlueprintType)
struct FExitPollQuestion
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "CognitiveVR Analytics")
		FString title;
	UPROPERTY(BlueprintReadOnly, Category = "CognitiveVR Analytics")
		FString type;

	//voice
	UPROPERTY(BlueprintReadOnly, Category = "CognitiveVR Analytics")
		int32 maxResponseLength = 0;

	//scale
	UPROPERTY(BlueprintReadOnly, Category = "CognitiveVR Analytics")
		FString minLabel;
	UPROPERTY(BlueprintReadOnly, Category = "CognitiveVR Analytics")
		FString maxLabel;
	UPROPERTY(BlueprintReadOnly, Category = "CognitiveVR Analytics")
		FExitPollScaleRange range;

	//multple choice
	UPROPERTY(BlueprintReadWrite, Category = "CognitiveVR Analytics")
		TArray<FExitPollMultipleChoice> answers;
};

USTRUCT(BlueprintType)
struct FExitPollQuestionSet
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "CognitiveVR Analytics")
		FString customerId;
	UPROPERTY(BlueprintReadOnly, Category = "CognitiveVR Analytics")
		//question set id
		FString id;
	UPROPERTY(BlueprintReadOnly, Category = "CognitiveVR Analytics")
		//question set name
		FString name;
	UPROPERTY(BlueprintReadOnly, Category = "CognitiveVR Analytics")
		int32 version = 0;
	UPROPERTY(BlueprintReadOnly, Category = "CognitiveVR Analytics")
		FString title;
	UPROPERTY(BlueprintReadOnly, Category = "CognitiveVR Analytics")
		FString status;
	UPROPERTY(BlueprintReadOnly, Category = "CognitiveVR Analytics")
		TArray<FExitPollQuestion> questions;
};

DECLARE_DYNAMIC_DELEGATE_OneParam(FCognitiveExitPollResponse, FExitPollQuestionSet, QuestionSet);

class FSceneData
{
public:
	FString Name = "";
	FString Id = "";
	int32 VersionNumber = 1;
	int32 VersionId = 0;

	FSceneData(FString name, FString id, int32 versionnumber, int32 versionid)
	{
		Name = name;
		Id = id;
		VersionNumber = versionnumber;
		VersionId = versionid;
	}
	FSceneData() {}
};

enum CognitiveVRError {
	kErrorSuccess = 0,
	kErrorGeneric = -1,
	kErrorNotInitialized = -2,
	kErrorNotFound = -3,
	kErrorInvalidArgs = -4,
	kErrorMissingId = -5,
	kErrorRequestTimedOut = -6,
	kErrorUnknown = -7
};


UENUM(BlueprintType)
enum class ECommonMeshName : uint8
{
	ViveController,
	OculusRiftTouchLeft,
	OculusRiftTouchRight,
	ViveTracker,
	WindowsMixedRealityLeft,
	WindowsMixedRealityRight,
	PicoNeo2EyeControllerLeft,
	PicoNeo2EyeControllerRight
};

//only used in blueprint to set up controllers using a macro
UENUM(BlueprintType)
enum class EC3DControllerType : uint8
{
	None,
	Vive,
	Oculus,
	WindowsMixedReality,
	PicoNeo2Eye
};

class FDynamicObjectManifestEntry
{
public:
	FString Id = "";
	FString Name = "";
	FString MeshName = "";
	TMap<FString, FString> StringProperties;
	FString ControllerType;
	bool IsRight;

	FDynamicObjectManifestEntry(FString id, FString name, FString mesh)
	{
		Id = id;
		Name = name;
		MeshName = mesh;
	}
	
	FDynamicObjectManifestEntry(){}

	FDynamicObjectManifestEntry* SetProperty(FString key, FString value);
};

class FDynamicObjectId
{
public:
	FString Id = "";
	bool Used = true;
	FString MeshName = "";

	FDynamicObjectId(FString id, FString meshName)
	{
		Id = id;
		MeshName = meshName;
	}

	FDynamicObjectId() {}
};

USTRUCT(BlueprintType)
struct COGNITIVEVR_API FControllerInputState
{
	GENERATED_BODY()

	FString AxisName;
	float AxisValue;
	bool IsVector;
	float X;
	float Y;
	FControllerInputState(){}
	FControllerInputState(FString name, float value)
	{
		AxisName = name;
		AxisValue = value;
		IsVector = false;
	}
	FControllerInputState(FString name, FVector vector)
	{
		AxisName = name;
		IsVector = true;
		X = vector.X;
		Y = vector.Y;
		AxisValue = vector.Z;
	}
};

USTRUCT(BlueprintType)
struct COGNITIVEVR_API FControllerInputStateCollection
{
	GENERATED_BODY()
	TMap<FString, FControllerInputState>States;
};

USTRUCT(BlueprintType)
struct FDynamicObjectSnapshot
{
	GENERATED_BODY()

public:
	FVector position = FVector(0, 0, 0);
	FVector scale = FVector(1, 1, 1);
	bool hasScaleChanged = false;
	FQuat rotation = FQuat(0, 0, 0, 1);
	double time = 0;
	FString id = "";
	TMap<FString, FString> StringProperties;
	TMap<FString, int32> IntegerProperties;
	TMap<FString, float> FloatProperties;
	TMap<FString, bool> BoolProperties;

	TMap<FString, FControllerInputState> Buttons;
};