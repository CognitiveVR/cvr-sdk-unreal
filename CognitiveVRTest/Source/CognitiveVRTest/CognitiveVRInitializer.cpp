// Fill out your copyright notice in the Description page of Project Settings.

#include "CognitiveVRTest.h"
#include "CognitiveVRInitializer.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Runtime/Core/Public/GenericPlatform/GenericPlatformMemory.h"

// Sets default values
ACognitiveVRInitializer::ACognitiveVRInitializer()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void ACognitiveVRInitializer::BeginPlay()
{
	Super::BeginPlay();

	TSharedPtr<FJsonObject> properties = MakeShareable(new FJsonObject);
	properties->SetStringField("platform", UGameplayStatics::GetPlatformName());
	properties->SetNumberField("ram", FGenericPlatformMemory::GetPhysicalGBRam());

	//FGenericPlatformMemoryStats stats = FGenericPlatformMemory::GetStats();
	//stats.TotalPhysical

	FCognitiveVRAnalytics::Get().Init("test username", "test device id", properties);
}
