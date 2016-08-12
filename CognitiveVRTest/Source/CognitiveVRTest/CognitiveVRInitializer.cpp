// Fill out your copyright notice in the Description page of Project Settings.

#include "CognitiveVRTest.h"
#include "CognitiveVRInitializer.h"


// Sets default values
ACognitiveVRInitializer::ACognitiveVRInitializer()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ACognitiveVRInitializer::BeginPlay()
{
	Super::BeginPlay();
	FCognitiveVRAnalytics::Get().Init("compnayname1234-productname-test", "test username", "test device id");
}

// Called every frame
void ACognitiveVRInitializer::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}

