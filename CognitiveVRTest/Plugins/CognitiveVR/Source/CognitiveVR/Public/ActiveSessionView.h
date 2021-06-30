// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CognitiveVR/Public/CognitiveVR.h"
#include "Blueprint/UserWidget.h"
#include "UObject/ConstructorHelpers.h"
#include "Blueprint/WidgetBlueprintGeneratedClass.h"
#include "Components/Widget.h"
#include "Components/WidgetComponent.h"
#include "IActiveSessionViewRequired.h"
#include "ActiveSessionView.generated.h"

//provides the interface to get data about fixations and eye tracking from fixation recorder to the ASV widget
//creates the active session view widget and calls 'initialize' to pass a reference to this

UCLASS()
class COGNITIVEVR_API AActiveSessionView : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AActiveSessionView();

	UPROPERTY(editanywhere)
		int32 HMDWidth = 2880;
	UPROPERTY(editanywhere)
		int32 HMDHeight = 1600;
	UPROPERTY(editanywhere)
		int32 SpectatorWidth = 1280;
	UPROPERTY(editanywhere)
		int32 SpectatorHeight = 720;

	//returns fixations as vector2d screen position. z value is radius
	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics")
		TArray<FVector> GetProjectedFixations();

	//returns eye positions for saccades
	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics")
		TArray<FVector2D> GetProjectedSaccades();

protected:

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	APlayerController* PlayerController;
	UFixationRecorder* FixationRecorder;
	UClass* WidgetClass;
	UWidgetComponent* WidgetComponent;
	virtual void Tick(float delta) override;
	void DelaySetupWidget();
};
