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
#include "CognitiveVR/Public/CustomEvent.h"
#include "CognitiveVR/Public/DynamicIdPoolAsset.h"
#include "MotionControllerComponent.h"
#include "CognitiveVR/Public/DynamicObject.h"
#include "DynamicObjectManager.generated.h"

class UCustomEvent;
class UCognitiveVRBlueprints;

UCLASS(BlueprintType)
class COGNITIVEVR_API UDynamicObjectManager : public UObject
{
	friend class FAnalyticsProviderCognitiveVR;

	GENERATED_BODY()

private:
	TArray<FDynamicObjectSnapshot> snapshots; //this should be cleared when session starts in PIE
	TArray<FDynamicObjectManifestEntry> manifest;
	TArray<FDynamicObjectManifestEntry> newManifest;
	TArray<TSharedPtr<FDynamicObjectId>> allObjectIds;
	int32 jsonPart = 1;
	int32 MaxSnapshots = -1;
	bool callbackInitialized;

	int32 MinTimer = 5;
	int32 AutoTimer = 10;
	int32 ExtremeBatchSize = 128;
	float NextSendTime = 0;
	float LastSendTime = -60;
	FTimerHandle AutoSendHandle;
	FString DynamicObjectFileType;

	TSharedPtr<FAnalyticsProviderCognitiveVR> cogProvider;

	UDynamicObjectManager();
	void TrySendData();
	void ClearSnapshots();
	

public:
	void RegisterObjectId(FString MeshName, FString Id, FString ActorName, bool IsController, bool IsRightController, FString ControllerName);

	// Sets default values for this component's properties

	void Initialize();

	UFUNCTION()
	void OnSessionBegin();
	UFUNCTION()
	void OnPreSessionEnd();
	UFUNCTION()
	void OnPostSessionEnd();

	void AddSnapshot(FDynamicObjectSnapshot snapshot);
	bool HasRegisteredObjectId(const FString id);
	TSharedPtr<FDynamicObjectId> GetUniqueId(FString meshName);

	TSharedPtr<FJsonValueObject> WriteSnapshotToJson(FDynamicObjectSnapshot snapshot);
	UFUNCTION()
	void SendData(bool copyDataToCache);
	TArray<TSharedPtr<FJsonValueObject>> DynamicSnapshotsToString();
	TSharedPtr<FJsonObject> DynamicObjectManifestToString();

	void EndPlay(const EEndPlayReason::Type EndPlayReason);

	float GetLastSendTime() { return LastSendTime; }
	int32 GetPartNumber() { return jsonPart; }
	int32 GetDataPoints() { return snapshots.Num(); }
	int32 GetDynamicObjectCount() { return manifest.Num(); }
};
