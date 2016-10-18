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
	FHttpModule* Http;
	

	UPROPERTY(editanywhere)
	UMaterial* SceneDepthMat;

	UPROPERTY(editanywhere)
	int ticksTillSend = 50;

	UPROPERTY(editanywhere)
	bool sendToServer;

public:	
	// Sets default values for this component's properties

	TMap<FString,FString> SceneKeys;

	//UPROPERTY(EditAnywhere)
	UTextureRenderTarget2D* renderTarget;

	UPlayerTracker();
	
	virtual void BeginPlay() override;
	
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;

	//auto myworld = GetWorld()->GetMapName();
	void SendData(FString sceneName);

	FVector GetGazePoint();
	float GetPixelDepth(float minvalue, float maxvalue);
	
};
