// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CognitiveVR/Public/C3DCommonTypes.h"
#include "CognitiveVR/Public/CognitiveVR.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "TimerManager.h"
#include "CoreMinimal.h"
#include "MotionControllerComponent.h"
#include "DynamicObject.generated.h"

class UCustomEvent;
class UCognitiveVRBlueprints;
class UDynamicObjectManager;
class FDynamicObjectId;
class UDynamicIdPoolAsset;

UENUM(BlueprintType)
enum class EIdSourceType : uint8
{
	CustomId,
	GeneratedId,
	PoolId
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class COGNITIVEVR_API UDynamicObject : public USceneComponent
{
	friend class FAnalyticsProviderCognitiveVR;

	GENERATED_BODY()

private:

	UDynamicObjectManager* dynamicObjectManager;
	float currentTime = 0;
	TSharedPtr<FDynamicObjectId> ObjectID;
	FVector LastPosition;
	FVector LastForward;
	FVector LastScale;
	bool HasInitialized = false;

	//used to set unique object id from snapshot or when accessed from elsewhere
	//void GenerateObjectId();

	//must be called after session begins - dynamicObjectManager doesn't exist until then - and holds all the dynamic object data
	UFUNCTION()
	void Initialize();
	UFUNCTION()
	void OnPostSessionEnd();
	UFUNCTION()
	void OnPreSessionEnd();

public:

	//should this object be represented by a custom mesh. requires uploading this mesh to the dashboard
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CognitiveVR Analytics")
		bool UseCustomMeshName = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CognitiveVR Analytics")
	bool IsController = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CognitiveVR Analytics")
	bool IsRightController = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "CognitiveVR Analytics")
		EC3DControllerType ControllerType;

	FString ControllerInputImageName;

	//the name of the mesh to render on the dashboard
	UPROPERTY(EditAnywhere, Category = "CognitiveVR Analytics")
		FString MeshName;

	//group and id

	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "CognitiveVR Analytics")
		EIdSourceType IdSourceType;

	//the custom id for registering this dynamic object. recommended for non-spawned actors
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "CognitiveVR Analytics")
		FString CustomId = "";

	bool HasValidPoolId = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "CognitiveVR Analytics")
		UDynamicIdPoolAsset* IDPool;

	//snapshots

	//time in seconds between checking if position and rotation updates need to be recorded
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "CognitiveVR Analytics")
		float SnapshotInterval = 0.1;

	//distance in cm the object needs to move before sending an update
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "CognitiveVR Analytics")
		float PositionThreshold = 2;

	//rotation in degrees the object needs to rotate before sending an update
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "CognitiveVR Analytics")
		float RotationThreshold = 10;

	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "CognitiveVR Analytics")
		float ScaleThreshold = 0.1;

	UDynamicObject();
	void TryGenerateMeshName();
	void TryGenerateCustomId();

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
	virtual void OnComponentCreated() override;

	TSharedPtr<FDynamicObjectId> GetObjectId();

	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Dynamic Object")
		FDynamicObjectSnapshot MakeSnapshot(bool hasChangedScale);

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

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

	//write all controller input states to snapshot to be written to json next frame
	void FlushButtons(FControllerInputStateCollection& target);

	void EndPlay(const EEndPlayReason::Type EndPlayReason);
	void CleanupDynamicObject();
};
