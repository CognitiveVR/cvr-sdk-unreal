// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
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
};
