/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/

#pragma once

#include "Cognitive3D/Public/C3DCommonTypes.h"
#include "Cognitive3D/Public/Cognitive3D.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "TimerManager.h"
#include "CoreMinimal.h"
#include "Cognitive3D/Public/CustomEvent.h"
#include "Cognitive3D/Public/DynamicIdPoolAsset.h"
#include "MotionControllerComponent.h"
#include "Cognitive3D/Public/DynamicObject.h"

class UCustomEvent;
class UCognitive3DBlueprints;

class COGNITIVE3D_API FDynamicObjectManager
{
	friend class FAnalyticsProviderCognitive3D;
	friend class FAnalyticsCognitive3D;

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

	TSharedPtr<FAnalyticsProviderCognitive3D> cogProvider;

	FDynamicObjectManager();
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
