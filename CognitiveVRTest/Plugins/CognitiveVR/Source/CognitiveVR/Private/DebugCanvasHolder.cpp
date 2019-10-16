// Fill out your copyright notice in the Description page of Project Settings.

#include "CognitiveVR.h"
#include "DebugCanvasHolder.h"


// Sets default values
ADebugCanvasHolder::ADebugCanvasHolder()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ADebugCanvasHolder::BeginPlay()
{
	Super::BeginPlay();
	//should be extended in blueprint - add debug widget to screen
}

// Called every frame
void ADebugCanvasHolder::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

