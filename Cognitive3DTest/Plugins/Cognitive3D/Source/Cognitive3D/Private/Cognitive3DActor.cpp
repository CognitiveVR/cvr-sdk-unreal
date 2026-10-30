/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/

#include "Cognitive3DActor.h"
#include "Cognitive3D/Public/Cognitive3DProvider.h"
#include "Cognitive3D/Public/Cognitive3D.h"
#include "C3DUtil/CognitiveLog.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "MotionControllerComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "Components/SkeletalMeshComponent.h"
#include "Cognitive3DProvider.h"
#include "DynamicObject.h"
#include "Cognitive3D/Private/C3DUtil/Util.h"
#include "TimerManager.h"
#include "Engine/SkeletalMesh.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Engine/Engine.h"
#include "IXRTrackingSystem.h"
#if WITH_EDITOR
#include <Editor.h>
#endif

TMap<uint32, TWeakObjectPtr<ACognitive3DActor>> ACognitive3DActor::PerWorldInstanceMap;

// Sets default values
ACognitive3DActor::ACognitive3DActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void ACognitive3DActor::BeginPlay()
{
	UWorld* world = GetWorld();
	if (world == NULL) { FCognitiveLog::Error("ACognitiveActor::BeginPlay world is null!"); return; }
	if (world->WorldType != EWorldType::PIE && world->WorldType != EWorldType::Game) { return; } //editor world. skip

	cog = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	cog->OnSessionBegin.AddDynamic(this, &ACognitive3DActor::ReceiveBeginSession);
	cog->OnSessionBegin.AddUniqueDynamic(this, &ACognitive3DActor::InitializeControllers);
	cog->OnPreSessionEnd.AddDynamic(this, &ACognitive3DActor::ReceivePreEndSession);
	cog->OnPostSessionEnd.AddDynamic(this, &ACognitive3DActor::ReceivePostEndSession);

#if WITH_EDITOR
	if (GIsEditor)
	{
		// Bind once
		FEditorDelegates::EndPIE.AddUObject(this, &ACognitive3DActor::OnEndPIE);
	}
#endif

	InitializeControllers();

	Super::BeginPlay();
	world->GetTimerManager().SetTimer(ControllerDiscoveryHandle, this,
		&ACognitive3DActor::InitializeControllers, 0.5f, true);
}

void ACognitive3DActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ControllerDiscoveryHandle);
		TWeakObjectPtr<ACognitive3DActor>* CachedActor = PerWorldInstanceMap.Find(World->GetUniqueID());
		if (CachedActor && (!CachedActor->IsValid() || CachedActor->Get() == this)) PerWorldInstanceMap.Remove(World->GetUniqueID());
	}
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


#if WITH_EDITOR
	if (GIsEditor && GIsPlayInEditorWorld)
	{
		if (bIsEditorStoppingPIE)
		{
			shouldEndSession = true;
		}
		else
		{
			shouldEndSession = false;
		}
	}
#endif

	if (cog.IsValid())
	{
		if (shouldEndSession)
		{
			cog->EndSession();
		}
		cog->OnSessionBegin.RemoveDynamic(this, &ACognitive3DActor::ReceiveBeginSession);
		cog->OnSessionBegin.RemoveDynamic(this, &ACognitive3DActor::InitializeControllers);
		cog->OnPreSessionEnd.RemoveDynamic(this, &ACognitive3DActor::ReceivePreEndSession);
		cog->OnPostSessionEnd.RemoveDynamic(this, &ACognitive3DActor::ReceivePostEndSession);
		cog.Reset();
	}
}

void ACognitive3DActor::OnEndPIE(bool bIsSimulating)
{
	// Set the flag to true
	bIsEditorStoppingPIE = true;
}


namespace
{
EC3DControllerType GetAutomaticControllerType()
{
	const FString Name = FUtil::GetHMDDeviceName() + TEXT(" ") +
		UHeadMountedDisplayFunctionLibrary::GetHMDDeviceName().ToString();
	if (Name.Contains(TEXT("Pico Neo 2"))) return EC3DControllerType::PicoNeo2;
	if (Name.Contains(TEXT("Pico Neo 3"))) return EC3DControllerType::PicoNeo3;
	if (Name.Contains(TEXT("Pico"))) return EC3DControllerType::PicoNeo4;
	if (Name.Contains(TEXT("Quest Pro"))) return EC3DControllerType::QuestPro;
	if (Name.Contains(TEXT("Quest 2"))) return EC3DControllerType::Quest2;
	if (Name.Contains(TEXT("Quest 3"))) return EC3DControllerType::Quest3;
	if (Name.Contains(TEXT("Vive"))) return EC3DControllerType::Vive;
	if (Name.Contains(TEXT("WindowsMixedReality"))) return EC3DControllerType::WindowsMixedReality;
	if (Name.Contains(TEXT("Oculus")) || Name.Contains(TEXT("Meta Quest"))) return EC3DControllerType::OculusRift;
	return EC3DControllerType::Quest3;
}

void GatherLocalPawnActors(APawn* Pawn, TArray<AActor*>& OutActors)
{
	OutActors.Reset();
	if (!IsValid(Pawn)) return;
	OutActors.Add(Pawn);
	for (int32 Index = 0; Index < OutActors.Num(); ++Index)
	{
		AActor* Current = OutActors[Index];
		for (AActor* OwnedActor : Current->Children)
			if (IsValid(OwnedActor) && !OwnedActor->IsActorBeingDestroyed()) OutActors.AddUnique(OwnedActor);
		TArray<AActor*> AttachedActors;
		Current->GetAttachedActors(AttachedActors);
		for (AActor* AttachedActor : AttachedActors)
			if (IsValid(AttachedActor) && !AttachedActor->IsActorBeingDestroyed()) OutActors.AddUnique(AttachedActor);
	}
}

bool IsControllerBound(UMotionControllerComponent* Controller, UDynamicObject* Dynamic)
{
	return IsValid(Controller) && Controller->IsRegistered() && IsValid(Dynamic) && Dynamic->IsRegistered();
}
}

void ACognitive3DActor::InitializeControllers()
{
	UWorld* World = GetWorld();
	if (!World || !cog.IsValid() || !cog->HasStartedSession()) return;
	APlayerController* Player = UGameplayStatics::GetPlayerController(World, 0);
	APawn* Pawn = IsValid(Player) && Player->IsLocalController() ? Player->GetPawn() : nullptr;
	if (!IsValid(Pawn)) return;
	if (ControllerPawn.Get() == Pawn && IsControllerBound(LeftController, LeftDynamic) &&
		IsControllerBound(RightController, RightDynamic)) return;
	ControllerPawn = Pawn;
	TArray<AActor*> Candidates;
	GatherLocalPawnActors(Pawn, Candidates);
	const bool bHeadset = GEngine && GEngine->XRSystem.IsValid() && GEngine->XRSystem->IsHeadTrackingAllowed();
	UMotionControllerComponent* Selected[2] = { nullptr, nullptr };
	int32 Scores[2] = { -1, -1 };
	for (AActor* Actor : Candidates)
	{
		TArray<UMotionControllerComponent*> Components;
		Actor->GetComponents(Components);
		for (UMotionControllerComponent* Component : Components)
		{
			if (!IsValid(Component) || !Component->IsRegistered() || Component->PlayerIndex != 0) continue;
			const FString Source = Component->MotionSource.ToString();
			const bool bRight = Source == TEXT("Right") || Source == TEXT("RightGrip") || Source == TEXT("RightAim");
			const bool bLeft = Source == TEXT("Left") || Source == TEXT("LeftGrip") || Source == TEXT("LeftAim");
			if (!bRight && !bLeft) continue;
			const int32 Score = bHeadset
				? (FUtil::IsMotionControllerTracked(Component) ? 1000 : 0) + (Actor == Pawn ? 100 : 0) + (FindHandComponent(Component) ? 10 : 0)
				: (FindHandComponent(Component) ? 100 : 0) + (Actor != Pawn ? 10 : 0);
			const int32 Side = bRight ? 1 : 0;
			if (Score > Scores[Side]) { Selected[Side] = Component; Scores[Side] = Score; }
		}
	}

	auto Configure = [this](UMotionControllerComponent* SelectedController, bool bRight,
		UMotionControllerComponent*& Controller, USceneComponent*& Hand,
		UDynamicObject*& Dynamic, UDynamicObject*& Automatic)
	{
		if (Controller != SelectedController && IsValid(Automatic))
		{
			Automatic->DestroyComponent();
			Automatic = nullptr;
		}
		Controller = SelectedController;
		Hand = IsValid(Controller) ? FindHandComponent(Controller) : nullptr;
		Dynamic = IsValid(Controller) ? FindControllerDynamic(Controller, bRight) : nullptr;
		if (!IsValid(Controller) || IsValid(Dynamic)) return;
		AActor* ControllerOwner = Controller->GetOwner();
		if (!IsValid(ControllerOwner)) return;
		Automatic = NewObject<UDynamicObject>(ControllerOwner, NAME_None, RF_Transient);
		Automatic->IdSourceType = EIdSourceType::GeneratedId;
		Automatic->IsController = true;
		Automatic->IsRightController = bRight;
		Automatic->SyncUpdateWithPlayer = true;
		Automatic->ControllerType = GetAutomaticControllerType();
		Automatic->SetupAttachment(Controller);
		Automatic->ControllerSettleUntil = FPlatformTime::Seconds() + 0.35;
		ControllerOwner->AddInstanceComponent(Automatic);
		Automatic->RegisterComponent();
		Dynamic = Automatic;
		UE_LOG(LogTemp, Log, TEXT("Cognitive3D %s controller bound to %s"), bRight ? TEXT("right") : TEXT("left"), *Controller->GetPathName());
	};
	Configure(Selected[0], false, LeftController, LeftHandComponent, LeftDynamic, AutomaticLeftDynamic);
	Configure(Selected[1], true, RightController, RightHandComponent, RightDynamic, AutomaticRightDynamic);
}

UDynamicObject* ACognitive3DActor::GetControllerDynamic(bool bRight) const
{
	UDynamicObject* Dynamic = bRight ? RightDynamic : LeftDynamic;
	return IsValid(Dynamic) && Dynamic->IsRegistered() && Dynamic->GetWorld() == GetWorld() ? Dynamic : nullptr;
}

USceneComponent* ACognitive3DActor::FindHandComponent(USceneComponent* Parent)
{
	if (!Parent)
	{
		return nullptr;
	}

	// Get all child components
	TArray<USceneComponent*> ChildrenComponents;
	Parent->GetChildrenComponents(true, ChildrenComponents);

	for (USceneComponent* Child : ChildrenComponents)
	{
		// Check if this component is a Skeletal Mesh Component
		USkeletalMeshComponent* SkeletalMeshComponent = Cast<USkeletalMeshComponent>(Child);
#if ENGINE_MAJOR_VERSION == 4 || (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION < 1)
		USkeletalMesh* SkeletalMesh = SkeletalMeshComponent ? SkeletalMeshComponent->SkeletalMesh : nullptr;
#else
		USkeletalMesh* SkeletalMesh = SkeletalMeshComponent ? SkeletalMeshComponent->GetSkeletalMeshAsset() : nullptr;
#endif
		if (SkeletalMesh)
		{
			return Child;  // Return the scene component that holds the skeletal mesh
		}
	}

	return nullptr;  // No valid component found
}

UDynamicObject* ACognitive3DActor::FindControllerDynamic(USceneComponent* Parent, bool bRight) const
{
	if (!IsValid(Parent)) return nullptr;
	TArray<USceneComponent*> ChildComponents;
	Parent->GetChildrenComponents(true, ChildComponents);
	for (USceneComponent* Child : ChildComponents)
	{
		UDynamicObject* Dynamic = Cast<UDynamicObject>(Child);
		if (IsValid(Dynamic) && Dynamic->IsController && Dynamic->IsRightController == bRight)
			return Dynamic;
	}
	return nullptr;
}

UWorld* ACognitive3DActor::GetCognitiveSessionWorld()
{
	auto cognitiveActor = GetCognitive3DActor();
	if (cognitiveActor == nullptr) { return nullptr; }
	return cognitiveActor->GetWorld();
}

ACognitive3DActor* ACognitive3DActor::GetCognitive3DActor(const UObject *WorldContextObject)
{
    UWorld *RequestedWorld = nullptr;
    
    if (!IsValid(WorldContextObject)) {
        // Since we didn't get any context object we'll use the current local
        // player
        RequestedWorld = FUtil::GetCurrentActiveWorld();
    } else {
        RequestedWorld = WorldContextObject->GetWorld();
    }
 
    // If the world is null something has gone very wrong
	//check(IsValid(RequestedWorld));
	if (RequestedWorld == nullptr) {
		return nullptr;
	}
 
 	// Check per-world cache
	TWeakObjectPtr<ACognitive3DActor>* CachedActor = PerWorldInstanceMap.Find(RequestedWorld->GetUniqueID());
	ACognitive3DActor* AnalyticsActor = CachedActor ? CachedActor->Get() : nullptr;
	if (IsValid(AnalyticsActor) && AnalyticsActor->GetWorld() == RequestedWorld && !AnalyticsActor->IsActorBeingDestroyed()) {
		return AnalyticsActor;
 	}
	PerWorldInstanceMap.Remove(RequestedWorld->GetUniqueID());
 
     // This world has not been added yet so let's search it
 	for (TActorIterator<ACognitive3DActor> Itr(RequestedWorld); Itr; ++Itr) {
		if (!IsValid(*Itr) || Itr->IsActorBeingDestroyed() || Itr->GetWorld() != RequestedWorld) continue;
         // Found one so let's add this instance to the map
 		PerWorldInstanceMap.Add(RequestedWorld->GetUniqueID(), *Itr);
 		return *Itr;
 	} 

    // If we get here there are no Cognitive3DActors in any world
    return nullptr;

}
