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

class UCustomEvent;
class UCognitiveVRBlueprints;

class COGNITIVEVR_API UDynamicObjectManager
{
	friend class FAnalyticsProviderCognitiveVR;
	friend class FAnalyticsCognitiveVR;

private:
	TArray<FDynamicObjectSnapshot> snapshots;
	TArray<FDynamicObjectManifestEntry> manifest;
	TArray<FDynamicObjectManifestEntry> newManifest;
	TArray<TSharedPtr<FDynamicObjectId>> allObjectIds;
	int32 jsonPart = 1;
	int32 MaxSnapshots = 64;

	int32 AutoTimer = 10;
	float LastSendTime = -60;
	FTimerHandle AutoSendHandle;
	FString DynamicObjectFileType;

	TSharedPtr<FAnalyticsProviderCognitiveVR> cogProvider;

	UDynamicObjectManager();
	UFUNCTION()
	void OnSessionBegin();
	UFUNCTION()
	void OnPreSessionEnd();
	UFUNCTION()
	void OnPostSessionEnd();

public:
	void AddSnapshot(FDynamicObjectSnapshot snapshot);
	bool HasRegisteredObjectId(const FString id);
	void RegisterObjectId(FString MeshName, FString Id, FString ActorName, bool IsController, bool IsRightController, FString ControllerName);
	void CacheControllerPointer(UDynamicObject* object, bool isRight);
	UFUNCTION()
	void SendData(bool copyDataToCache);

private:
	TArray<TSharedPtr<FJsonValueObject>> DynamicSnapshotsToString();
	TSharedPtr<FJsonObject> DynamicObjectManifestToString();
	TSharedPtr<FDynamicObjectId> GetUniqueId(FString meshName);
	TSharedPtr<FJsonValueObject> WriteSnapshotToJson(FDynamicObjectSnapshot snapshot);

public:
	float GetLastSendTime() { return LastSendTime; }
	int32 GetPartNumber() { return jsonPart; }
	int32 GetDataPoints() { return snapshots.Num(); }
	int32 GetDynamicObjectCount() { return manifest.Num(); }

private:
	TWeakObjectPtr<UDynamicObject> LeftHandController;
	TWeakObjectPtr<UDynamicObject> RightHandController;
};
