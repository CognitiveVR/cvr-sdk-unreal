// Fill out your copyright notice in the Description page of Project Settings.

#include "CognitiveVRActor.h"

ACognitiveVRActor* ACognitiveVRActor::instance;

// Sets default values
ACognitiveVRActor::ACognitiveVRActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void ACognitiveVRActor::BeginPlay()
{
	UWorld* world = GetWorld();
	if (world == NULL) { CognitiveLog::Error("ACognitiveActor::BeginPlay world is null!"); return; }
	if (world->WorldType != EWorldType::PIE && world->WorldType != EWorldType::Game) { return; } //editor world. skip

	Super::BeginPlay();

	cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
}

void ACognitiveVRActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (!cog.IsValid())
	{
		return;
	}

	Super::EndPlay(EndPlayReason);

	FString reason;
	bool shouldEndSession = true;
	switch (EndPlayReason)
	{
	case EEndPlayReason::Destroyed: reason = "destroyed";
		//this should normally never be destroyed. 4.19 bug - this is called instead of level transition
		cog->FlushEvents();
		//shouldEndSession = false;
		break;
	case EEndPlayReason::EndPlayInEditor: reason = "end PIE";
		break;
	case EEndPlayReason::LevelTransition: reason = "level transition";
		//this is called correctly in 4.24. possibly earlier versions
		cog->FlushEvents();
		shouldEndSession = false;
		break;
	case EEndPlayReason::Quit: reason = "quit";
		break;
	case EEndPlayReason::RemovedFromWorld: reason = "removed from world";
		break;
	default:
		reason = "default";
		break;
	}

	if (cog.IsValid())
	{
		if (shouldEndSession)
		{
			cog->EndSession();
		}
		cog.Reset();
	}
}

ACognitiveVRActor* ACognitiveVRActor::GetCognitiveVRActor()
{
	if (instance == NULL)
	{
		for (TObjectIterator<ACognitiveVRActor> Itr; Itr; ++Itr)
		{
			UWorld* tempWorld = Itr->GetWorld();
			if (tempWorld == NULL) { continue; }
			if (tempWorld->WorldType != EWorldType::PIE && tempWorld->WorldType != EWorldType::Game) { continue; } //editor world. skip
			instance = *Itr;
			break;
		}
	}

	return instance;
}