// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/SceneComponent.h"
//#include "Json.h"
#include "Engine/SceneCapture2D.h"
#include "Engine/Texture.h"
#include "Engine/Texture2D.h"
#include "SceneView.h"
#include "Engine/TextureRenderTarget2D.h"
#include "PlayerTracker.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class COGNITIVEVR_API UPlayerTracker : public USceneComponent
{
	GENERATED_BODY()

private:
	float currentTime = 0;
	TArray<TSharedPtr<FJsonObject>> snapshots;
	TArray<TSharedPtr<FJsonObject>> events;
	FHttpModule* Http;
	float maxDistance = 8192;
	int32 jsonEventPart;
	int32 jsonGazePart;


	//UPROPERTY(editanywhere)
	//UTextureRenderTarget2D* manualTexture;
	UMaterial* SceneDepthMat;

	FString materialPath = "/CognitiveVR/DepthPostProcessing";
	TSharedPtr<FAnalyticsProviderCognitiveVR> s;

public:	
	// Sets default values for this component's properties

	
	//UTextureRenderTarget2D* renderTarget;
	USceneCaptureComponent2D* sceneCapture;

	UPROPERTY(EditAnywhere)
	float PlayerSnapshotInterval = 0.1;
	int32 GazeBatchSize = 100;

	UPlayerTracker();
	
	virtual void BeginPlay() override;
	
	void InitializePlayerTracker();

	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;

	void SendGazeEventDataToSceneExplorer();
	void SendGazeEventDataToSceneExplorer(FString sceneName);

	//void static BlueprintSendData();

	FString GetSceneKey(FString sceneName);

	FVector GetGazePoint();
	float GetPixelDepth(float minvalue, float maxvalue);
	
	void AddJsonEvent(FJsonObject* newEvent);

	FString GazeSnapshotsToString();
	FString EventSnapshotsToString();

	///send json data to scene explorer
	static void SendJson(FString endpoint, FString Json);

	//UPROPERTY(EditAnywhere)
	//bool SendDataOnEndPlay = true;

	//UPROPERTY(EditAnywhere)
	//bool EndSessionOnEndPlay = true;

	//virtual void EndPlay(const EEndPlayReason::Type EndPlayReason);
};