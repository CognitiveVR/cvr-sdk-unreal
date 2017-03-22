// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/SceneComponent.h"
//#include "Json.h"
#include "Engine/SceneCapture2D.h"
#include "Engine/Texture.h"
#include "Engine/Texture2D.h"
#include "SceneView.h"
#include "Engine/TextureRenderTarget2D.h"
#include "DynamicObject.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class COGNITIVEVR_API UDynamicObject : public UActorComponent
{
	GENERATED_BODY()

private:
	float currentTime = 0;
	TArray<TSharedPtr<FJsonObject>> snapshots;
	int32 jsonPart;
	int32 MaxSnapshots;
	bool tickEnabled = true;
	TSharedPtr<FAnalyticsProviderCognitiveVR> s;

public:	
	// Sets default values for this component's properties

	UPROPERTY(EditAnywhere)
	FString MeshName;

	UPROPERTY(EditAnywhere)
		bool SnapshotOnEnable = true;

	UPROPERTY(EditAnywhere)
	bool UpdateTickOnEnable = true;

	UPROPERTY(EditAnywhere)
	bool ReleaseIdOnDestroy = true;

	//group and id

	UPROPERTY(EditAnywhere)
	int32 CustomId = -1;

	UPROPERTY(EditAnywhere)
	FString GroupName;


	//snapshots

	UPROPERTY(EditAnywhere)
	float SnapshotInterval = 0.1;

	UPROPERTY(EditAnywhere)
	float PositionThreshold = 1;

	UPROPERTY(EditAnywhere)
	float RotationThreshold = 30;



	UDynamicObject();
	
	virtual void BeginPlay() override;

	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;

	void SendData();
	void SendData(FString sceneName);

	FString DynamicSnapshotsToString();
};
