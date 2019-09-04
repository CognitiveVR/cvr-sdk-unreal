// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/SceneComponent.h"
//#include "Json.h"
#include "Engine/SceneCapture2D.h"
#include "Engine/Texture.h"
#include "Engine/Texture2D.h"
#include "SceneView.h"
#include "Engine/TextureRenderTarget2D.h"
//#include "KismetMathLibrary.h"
#include "DynamicObject.generated.h"

UENUM(BlueprintType)
enum class ECommonMeshName : uint8
{
	ViveController,
	OculusTouchLeft,
	OculusTouchRight,
	ViveTracker,
	WindowsMixedRealityLeft,
	WindowsMixedRealityRight
};

//only used in blueprint to set up controllers using a macro
UENUM(BlueprintType)
enum class EC3DControllerType : uint8
{
	Vive,
	Oculus,
	WindowsMixedReality
};

namespace cognitivevrapi
{
class FEngagementEvent
{
public:
	bool Active = true;
	FString EngagementType = "";
	FString Parent = "";
	float EngagementTime = 0;
	int32 EngagementNumber = 0;

	FEngagementEvent(FString name, FString parent, int engagementNumber)
	{
		EngagementType = name;
		Parent = parent;
		EngagementNumber = engagementNumber;
	}
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
}

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

	TArray<cognitivevrapi::FEngagementEvent> Engagements;

	/*FDynamicObjectSnapshot* SnapshotProperty(FString key, FString value);
	FDynamicObjectSnapshot* SnapshotProperty(FString key, bool value);
	FDynamicObjectSnapshot* SnapshotProperty(FString key, int32 value);
	FDynamicObjectSnapshot* SnapshotProperty(FString key, double value);*/
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class COGNITIVEVR_API UDynamicObject : public USceneComponent //UActorComponent
{
	friend class FAnalyticsProviderCognitiveVR;

	GENERATED_BODY()

private:
	//TArray<TSharedPtr<FJsonObject>> snapshots;
	//extern TArray<TSharedPtr<FJsonObject>> snapshots;
	//extern int32 jsonPart = 0;
	//extern int32 MaxSnapshots = 64;

	float currentTime = 0;
	TSharedPtr<FAnalyticsProviderCognitiveVR> s;
	TSharedPtr<cognitivevrapi::FDynamicObjectId> ObjectID;
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

	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool IsController = false;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool IsRightController = false;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
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
	UPROPERTY(EditAnywhere, AdvancedDisplay)
	FString CustomId = "";

	//should this object record how the player is gazing?
	UPROPERTY(EditAnywhere)
	bool TrackGaze = true;

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
	TArray<cognitivevrapi::FEngagementEvent> DirtyEngagements;
	TArray<cognitivevrapi::FEngagementEvent> Engagements;
	void BeginEngagementId(FString engagementName, FString parentDynamicObjectId);
	void EndEngagementId(FString engagementName, FString parentDynamicObjectId);


	//virtual void OnComponentCreated() override;
	virtual void BeginPlay() override;

	TSharedPtr<cognitivevrapi::FDynamicObjectId> GetUniqueId(FString meshName);
	TSharedPtr<cognitivevrapi::FDynamicObjectId> GetObjectId();

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

	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Dynamic Object")
		static void BeginEngagement(UDynamicObject* target, FString engagementType);

	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Dynamic Object")
		static void EndEngagement(UDynamicObject* target, FString engagementType);

	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Dynamic Object")
		///this does not directly send a snapshot - it stores it until Flush is called or the number of stored dynamic snapshots reaches its limit
		void SendDynamicObjectSnapshot(UPARAM(ref) FDynamicObjectSnapshot& target);

	//adds a dynamic object component and applies all relevant settings
	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Dynamic Object")
		static UDynamicObject* SetupController(AActor* target, bool IsRight, EC3DControllerType controllerType);

	//write all controller input states to snapshot to be written to json next frame
	void FlushButtons(FControllerInputStateCollection& target);

	void EndPlay(const EEndPlayReason::Type EndPlayReason);

};
