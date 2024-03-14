/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/

#pragma once

#include "CoreMinimal.h"
#include "Cognitive3D/Public/Cognitive3D.h"
#include "Blueprint/UserWidget.h"
#include "UObject/ConstructorHelpers.h"
#include "Blueprint/WidgetBlueprintGeneratedClass.h"
#include "Components/Widget.h"
#include "Components/WidgetComponent.h"
#include "ActiveSessionViewRequired.h"
#include "Cognitive3D/Public/Cognitive3DActor.h"
#include "ActiveSessionView.generated.h"

//provides the interface to get data about fixations and eye tracking from fixation recorder to the ASV widget
//creates the active session view widget and calls 'initialize' to pass a reference to this

UCLASS()
class COGNITIVE3D_API AActiveSessionView : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AActiveSessionView();

	UPROPERTY(EditAnywhere, Category = "Cognitive3D Analytics")
		int32 HMDWidth = 2880;
	UPROPERTY(EditAnywhere, Category = "Cognitive3D Analytics")
		int32 HMDHeight = 1600;
	UPROPERTY(EditAnywhere, Category = "Cognitive3D Analytics")
		int32 SpectatorWidth = 1280;
	UPROPERTY(EditAnywhere, Category = "Cognitive3D Analytics")
		int32 SpectatorHeight = 720;

	//returns fixations as vector2d screen position. z value is radius
	UFUNCTION(BlueprintCallable, Category = "Cognitive3D Analytics")
		TArray<FVector> GetProjectedFixations();

	//returns eye positions for saccades
	UFUNCTION(BlueprintCallable, Category = "Cognitive3D Analytics")
		TArray<FVector2D> GetProjectedSaccades();

protected:
	APlayerController* PlayerController;
	UFixationRecorder* FixationRecorder;
	UClass* WidgetClass;
	UWidgetComponent* WidgetComponent;

	virtual void BeginPlay() override;
	virtual void Tick(float delta) override;
	void DelaySetupWidget();

private:
	float Remap(float num, float low1, float high1, float low2, float high2);
};
