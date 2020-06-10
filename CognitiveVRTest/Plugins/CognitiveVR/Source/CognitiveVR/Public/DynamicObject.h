// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CognitiveVR.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "TimerManager.h"
#include "CoreMinimal.h"
#include "CustomEvent.h"
#include "MotionControllerComponent.h"
#include "DynamicObject.generated.h"

class UCustomEvent;

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

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class COGNITIVEVR_API UDynamicObject : public USceneComponent //UActorComponent
{
	friend class FAnalyticsProviderCognitiveVR;

	GENERATED_BODY()

private:
	static TArray<FDynamicObjectSnapshot> snapshots; //this should be cleared when session starts in PIE
	static TArray<FDynamicObjectManifestEntry> manifest;
	static TArray<FDynamicObjectManifestEntry> newManifest;
	static TArray<TSharedPtr<FDynamicObjectId>> allObjectIds;
	static int32 jsonPart;// = 1;
	static int32 MaxSnapshots;// = -1;

	static int32 MinTimer;// = 5;
	static int32 AutoTimer;// = 10;
	static int32 ExtremeBatchSize;// = 128;
	static float NextSendTime;// = 0;
	static float LastSendTime;// = -60;
	static FTimerHandle CognitiveDynamicAutoSendHandle;
	static FString DynamicObjectFileType;

	static TSharedPtr<FAnalyticsProviderCognitiveVR> cogProvider;

	float currentTime = 0;
	TSharedPtr<FDynamicObjectId> ObjectID;
	FVector LastPosition;
	FVector LastForward;
	FVector LastScale;
	bool HasInitialized = false;

	//used to set unique object id from snapshot or when accessed from elsewhere
	void GenerateObjectId();

	void Initialize();
	static void TrySendData();
	static void ClearSnapshots();

public:	
	// Sets default values for this component's properties

	static void OnSessionBegin();
	static void OnSessionEnd();

	//should this object be represented by a custom mesh. requires uploading this mesh to the dashboard
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool UseCustomMeshName = true;

	bool IsController = false;

	bool IsRightController = false;

	FString ControllerType;

	//the name of the mesh to render on the dashboard
	UPROPERTY(EditAnywhere)
	FString MeshName;

	//if not using a custom mesh, which common mesh to render on the dashboard to represent this object
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay)
	ECommonMeshName CommonMeshName;

	UPROPERTY(EditAnywhere, AdvancedDisplay)
	bool SnapshotOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay)
	bool SnapshotOnInterval = true;

	UPROPERTY(EditAnywhere, AdvancedDisplay)
	bool ReleaseIdOnDestroy = true;

	//group and id

	//set a custom id for this dynamic object. recommended for non-spawned actors
	UPROPERTY(EditAnywhere, AdvancedDisplay)
	bool UseCustomId;

	//the custom id for registering this dynamic object. recommended for non-spawned actors
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay)
	FString CustomId = "";

	//snapshots

	//time in seconds between checking if position and rotation updates need to be recorded
	UPROPERTY(EditAnywhere, AdvancedDisplay)
	float SnapshotInterval = 0.1;

	//distance in cm the object needs to move before sending an update
	UPROPERTY(EditAnywhere, AdvancedDisplay)
	float PositionThreshold = 2;

	//rotation in degrees the object needs to rotate before sending an update
	UPROPERTY(EditAnywhere, AdvancedDisplay)
	float RotationThreshold = 10;

	UPROPERTY(EditAnywhere, AdvancedDisplay)
	float ScaleThreshold = 0.1;

	UDynamicObject();
	void TryGenerateMeshName();
	void GenerateCustomId();
	void TryGenerateCustomIdAndMesh();
	
	//engagements
	UPROPERTY()//need uproperty to keep from custom events from being garbage collected
	TMap<FString, UCustomEvent*> Engagements;

	// Alternate method for beginning a Custom Event and setting a Dynamic Object as the target using the DynamicObjectID
	// engagement name will be displayed as the event name
	// engagement id should be used when multiple events of the same type are active on a dynamic object
	void BeginEngagementId(FString parentDynamicObjectId, FString engagementName, FString UniqueEngagementId);
	
	// Alternate method for ending a Custom Event by DynamicObjectID
	// the name of the event to end. if there isn't an active event, will immediately create and end the event
	// engagement id should be used to cancel a specific event on a dynamic object if multiple with the same name are present
	void EndEngagementId(FString parentDynamicObjectId, FString engagementName, FString UniqueEngagementId);

	virtual void BeginPlay() override;

	TSharedPtr<FDynamicObjectId> GetUniqueId(FString meshName);
	TSharedPtr<FDynamicObjectId> GetObjectId();

	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Dynamic Object")
	FDynamicObjectSnapshot MakeSnapshot(bool hasChangedScale);

	static TSharedPtr<FJsonValueObject> WriteSnapshotToJson(FDynamicObjectSnapshot snapshot);

	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;

	static void SendData();
	static TArray<TSharedPtr<FJsonValueObject>> DynamicSnapshotsToString();
	static TSharedPtr<FJsonObject> DynamicObjectManifestToString();


	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Dynamic Object")
		FDynamicObjectSnapshot SnapshotStringProperty(UPARAM(ref) FDynamicObjectSnapshot& target, FString key, FString stringValue);

	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Dynamic Object")
		FDynamicObjectSnapshot SnapshotBoolProperty(UPARAM(ref) FDynamicObjectSnapshot& target, FString key, bool boolValue);

	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Dynamic Object")
		FDynamicObjectSnapshot SnapshotIntegerProperty(UPARAM(ref) FDynamicObjectSnapshot& target, FString key, int32 intValue);

	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Dynamic Object")
		FDynamicObjectSnapshot SnapshotFloatProperty(UPARAM(ref) FDynamicObjectSnapshot& target, FString key, float floatValue);

	//IMPROVEMENT investigate CallableWithoutWorldContext ufunction keyword
	// Alternate method for beginning a Custom Event and setting this Dynamic Object as the target
	// engagement name will be displayed as the event name
	// engagement id should be used when multiple events of the same type are active on a dynamic object
	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Dynamic Object")
		static void BeginEngagement(UDynamicObject* target, FString engagementName, FString UniqueEngagementId);

	// Alternate method for ending a Custom Event on a specific dynamic object
	// the name of the event to end. if there isn't an active event, will immediately create and end the event
	// engagement id should be used to cancel a specific event on a dynamic object if multiple with the same name are present
	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Dynamic Object")
		static void EndEngagement(UDynamicObject* target, FString engagementType, FString UniqueEngagementId);

	//this does not directly send a snapshot - it stores it until Flush is called or the number of stored dynamic snapshots reaches its limit
	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Dynamic Object", DisplayName = "Record Dynamic Object")
		void SendDynamicObjectSnapshot(UPARAM(ref) FDynamicObjectSnapshot& target);

	//adds a dynamic object component and applies all relevant settings
	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Dynamic Object")
		static UDynamicObject* SetupController(AActor* target, bool IsRight, EC3DControllerType controllerType);

	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Dynamic Object")
		static UDynamicObject* SetupControllerComponent(UDynamicObject* target, bool IsRight, EC3DControllerType controllerType);

	//write all controller input states to snapshot to be written to json next frame
	void FlushButtons(FControllerInputStateCollection& target);

	void EndPlay(const EEndPlayReason::Type EndPlayReason);

	float static GetLastSendTime() { return LastSendTime; }
	int32 static GetPartNumber() { return jsonPart; }
	int32 static GetDataPoints() { return snapshots.Num(); }
	int32 static GetDynamicObjectCount() { return manifest.Num(); }
};
