/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/

#include "Cognitive3DActor.h"
#include "C3DUtil/CognitiveLog.h"

ACognitive3DActor* ACognitive3DActor::instance = nullptr;

// Sets default values
ACognitive3DActor::ACognitive3DActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	instance = nullptr;

}

// Called when the game starts or when spawned
void ACognitive3DActor::BeginPlay()
{
	UWorld* world = GetWorld();
	if (world == NULL) { FCognitiveLog::Error("ACognitiveActor::BeginPlay world is null!"); return; }
	if (world->WorldType != EWorldType::PIE && world->WorldType != EWorldType::Game) { return; } //editor world. skip

	cog = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	cog->OnSessionBegin.AddDynamic(this, &ACognitive3DActor::ReceiveBeginSession);
	cog->OnPreSessionEnd.AddDynamic(this, &ACognitive3DActor::ReceivePreEndSession);
	cog->OnPostSessionEnd.AddDynamic(this, &ACognitive3DActor::ReceivePostEndSession);


	Super::BeginPlay();
}

void ACognitive3DActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (!cog.IsValid())
	{
		return;
	}

	Super::EndPlay(EndPlayReason);

	bool shouldEndSession = true;
	switch (EndPlayReason)
	{
	case EEndPlayReason::Destroyed:
		//this should normally never be destroyed. 4.19 bug - this is called instead of level transition
		cog->FlushEvents();
		break;
	case EEndPlayReason::EndPlayInEditor:
		break;
	case EEndPlayReason::LevelTransition:
		cog->FlushEvents();
		shouldEndSession = false;
		break;
	case EEndPlayReason::Quit:
		break;
	case EEndPlayReason::RemovedFromWorld:
		//removed from a sublevel unloading
		cog->FlushEvents();
		shouldEndSession = false;
		break;
	default:
		break;
	}

	if (cog.IsValid())
	{
		if (shouldEndSession)
		{
			cog->EndSession();
		}
		cog->OnSessionBegin.RemoveDynamic(this, &ACognitive3DActor::ReceiveBeginSession);
		cog->OnPreSessionEnd.RemoveDynamic(this, &ACognitive3DActor::ReceivePreEndSession);
		cog->OnPostSessionEnd.RemoveDynamic(this, &ACognitive3DActor::ReceivePostEndSession);
		cog.Reset();
	}
}

UWorld* ACognitive3DActor::GetCognitiveSessionWorld()
{
	auto cognitiveActor = GetCognitive3DActor();
	if (cognitiveActor == nullptr) { return nullptr; }
	return cognitiveActor->GetWorld();
}

ACognitive3DActor* ACognitive3DActor::GetCognitive3DActor()
{
	if (instance == NULL)
	{
		for (TObjectIterator<ACognitive3DActor> Itr; Itr; ++Itr)
		{
			UWorld* tempWorld = Itr->GetWorld();
			if (tempWorld == NULL) { continue; }
			if (tempWorld->WorldType != EWorldType::PIE && tempWorld->WorldType != EWorldType::Game) { continue; }
			instance = *Itr;
			break;
		}
	}

	return instance;
}