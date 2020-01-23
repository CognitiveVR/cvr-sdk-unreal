// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CognitiveVR.h"
#include "Blueprint/UserWidget.h"
#include "UObject/ConstructorHelpers.h"
#include "Blueprint/WidgetBlueprintGeneratedClass.h"
#include "Components/Widget.h"
#include "WidgetComponent.h"
#include "DebugCanvasHolder.generated.h"

UCLASS()
class COGNITIVEVR_API ADebugCanvasHolder : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ADebugCanvasHolder();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	UClass* WidgetClass;
	UUserWidget* WidgetInstance;
	APlayerCameraManager* CameraManager;
	UWidgetComponent* WidgetComponent;
	virtual void Tick(float delta) override;
};
