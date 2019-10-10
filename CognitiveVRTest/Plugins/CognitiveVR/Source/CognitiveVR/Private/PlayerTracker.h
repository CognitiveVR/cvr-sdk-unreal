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
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Runtime/HeadMountedDisplay/Public/IXRTrackingSystem.h"
#if defined TOBII_EYETRACKING_ACTIVE
#include "TobiiTypes.h"
#include "ITobiiCore.h"
#include "ITobiiEyetracker.h"
#endif
#if defined SRANIPAL_API
#include "SRanipal_Eye.h"
#include "ViveSR_Enums.h"
#include "SRanipal_Eyes_Enums.h"
#include "SRanipal_FunctionLibrary_Eye.h"
#endif
#if defined VARJOEYETRACKER_API
#include "VarjoEyeTrackerFunctionLibrary.h"
#endif
#include "PlayerTracker.generated.h"

//multicast delegates cannot be static. use static pointer to playertracker instance in BP
//multicast also can't be used as argument in BP function (to implement custom bind function)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCognitiveSessionBegin, bool, Successful);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class COGNITIVEVR_API UPlayerTracker : public UActorComponent
{
	GENERATED_BODY()

private:
	float currentTime = 0;
	TArray<TSharedPtr<FJsonObject>> snapshots;

	int32 jsonGazePart = 1;

	TSharedPtr<FAnalyticsProviderCognitiveVR> cog;
	void BuildSnapshot(FVector position, FVector gaze, FRotator rotation, double time, bool didHitFloor, FVector floorHitPos, FString objectId = "");
	void BuildSnapshot(FVector position, FRotator rotation, double time, bool didHitFloor, FVector floorHitPos);

	FVector GetWorldGazeEnd(FVector start);
	FVector LastDirection;
	TArray<APlayerController*, FDefaultAllocator> controllers;

	static UPlayerTracker* instance;

public:

	UPROPERTY(EditAnywhere)
		float PlayerSnapshotInterval = 0.1;

	int32 GazeBatchSize = 100;

	UPlayerTracker();

	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void SendData();

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintPure, Category = "CognitiveVR Analytics")
		static UPlayerTracker* GetPlayerTracker();

	UPROPERTY(BlueprintAssignable, Category = "CognitiveVR Analytics")
		FOnCognitiveSessionBegin OnSessionBegin;
};
