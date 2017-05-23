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

class FDynamicObjectManifestEntry
{
public:
	int32 Id;
	FString Name;
	FString MeshName;
	TMap<FString, FString> StringProperties;

	FDynamicObjectManifestEntry(int32 id, FString name, FString mesh)
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
	int32 Id;
	bool Used = true;
	FString MeshName;

	FDynamicObjectId(int32 id, FString meshName)
	{
		Id = id;
		MeshName = meshName;
	}

	FDynamicObjectId() {}
};

USTRUCT()
struct FDynamicObjectSnapshot
{
	GENERATED_BODY()

public:
	FVector position;
	FQuat rotation;
	double time;
	int32 id;
	TMap<FString, FString> StringProperties;
	TMap<FString, int32> IntegerProperties;
	TMap<FString, double> DoubleProperties;
	TMap<FString, bool> BoolProperties;

	FDynamicObjectSnapshot* SnapshotProperty(FString key, FString value);
	FDynamicObjectSnapshot* SnapshotProperty(FString key, bool value);
	FDynamicObjectSnapshot* SnapshotProperty(FString key, int32 value);
	FDynamicObjectSnapshot* SnapshotProperty(FString key, double value);
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class COGNITIVEVR_API UDynamicObject : public UActorComponent
{
	GENERATED_BODY()

private:
	//TArray<TSharedPtr<FJsonObject>> snapshots;
	//extern TArray<TSharedPtr<FJsonObject>> snapshots;
	//extern int32 jsonPart = 0;
	//extern int32 MaxSnapshots = 64;

	float currentTime = 0;
	TSharedPtr<FAnalyticsProviderCognitiveVR> s;
	FDynamicObjectId ObjectID;
	FVector LastPosition;
	FVector LastForward;

public:	
	// Sets default values for this component's properties

	UPROPERTY(EditAnywhere)
	FString MeshName;

	/*UPROPERTY(EditAnywhere)
		float ExtraPitch;
	UPROPERTY(EditAnywhere)
		float ExtraYaw;
	UPROPERTY(EditAnywhere)
		float ExtraRoll;

	UPROPERTY(EditAnywhere)
		int32 XPos = 0;
	UPROPERTY(EditAnywhere)
		int32 YPos = 2;
	UPROPERTY(EditAnywhere)
		int32 ZPos = 1;
	UPROPERTY(EditAnywhere)
		int32 WPos = 3;*/

	UPROPERTY(EditAnywhere)
		bool SnapshotOnEnable = true;

	UPROPERTY(EditAnywhere)
	bool UpdateOnTick = true;

	UPROPERTY(EditAnywhere)
	bool ReleaseIdOnDestroy = true;

	//group and id

	UPROPERTY(EditAnywhere)
	bool UseCustomId;

	UPROPERTY(EditAnywhere)
	int32 CustomId = -1;

	UPROPERTY(EditAnywhere)
	FString GroupName;

	UPROPERTY(EditAnywhere)
	bool TrackGaze = false;

	//snapshots

	UPROPERTY(EditAnywhere)
	float SnapshotInterval = 0.1;

	UPROPERTY(EditAnywhere)
	float PositionThreshold = 2;

	UPROPERTY(EditAnywhere)
	float RotationThreshold = 10;



	UDynamicObject();
	
	virtual void OnComponentCreated() override;
	virtual void BeginPlay() override;

	FDynamicObjectId GetUniqueId(FString meshName);
	FDynamicObjectId GetObjectId();

	FDynamicObjectSnapshot MakeSnapshot();
	static TSharedPtr<FJsonValueObject> WriteSnapshotToJson(FDynamicObjectSnapshot snapshot);

	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;

	static void SendData();
	static void SendData(FString sceneName);
	static TArray<TSharedPtr<FJsonValueObject>> DynamicSnapshotsToString();
	static TSharedPtr<FJsonObject> DynamicObjectManifestToString();

	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Dynamic Object")
	FDynamicObjectSnapshot NewSnapshot();

	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Dynamic Object")
	FDynamicObjectSnapshot SnapshotStringProperty(FDynamicObjectSnapshot target, FString key, FString stringValue);

	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Dynamic Object")
		FDynamicObjectSnapshot SnapshotBoolProperty(FDynamicObjectSnapshot target, FString key, bool boolValue);

	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Dynamic Object")
		FDynamicObjectSnapshot SnapshotIntegerProperty(FDynamicObjectSnapshot target, FString key, int32 intValue);

	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Dynamic Object")
		FDynamicObjectSnapshot SnapshotFloatProperty(FDynamicObjectSnapshot target, FString key, float floatValue);
	
	//UFUNCTION(BlueprintCallable, Category = "Rendering", meta = (DisplayName = "Set Actor Hidden In Game", Keywords = "Visible Hidden Show Hide"))
	//void SetActorHiddenInGame(bool bNewHidden);

	void EndPlay(const EEndPlayReason::Type EndPlayReason);

};