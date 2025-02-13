/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/

#include "Cognitive3DActor.h"
#include "C3DUtil/CognitiveLog.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "MotionControllerComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Cognitive3DProvider.h"
#include "DynamicObject.h"
#include "Cognitive3D/Private/C3DUtil/Util.h"

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

	InitializeControllers();

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


void ACognitive3DActor::InitializeControllers()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("World is null!"));
		return;
	}

	// Get all actors in the world
	TArray<AActor*> AllActors;
	UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);

	for (AActor* Actor : AllActors)
	{
		TArray<UMotionControllerComponent*> Components;
		Actor->GetComponents(Components);

		for (UMotionControllerComponent* Component : Components)
		{
			if (Component->GetTrackingSource() == EControllerHand::Left)
			{
				LeftController = Component;
				UE_LOG(LogTemp, Log, TEXT("Left Controller found: %s"), *Component->GetName());

				// Find the scene component that contains a skeletal mesh
				LeftHandComponent = FindHandComponent(LeftController);
				if (LeftHandComponent)
				{
					UE_LOG(LogTemp, Log, TEXT("Left Hand Component found: %s"), *LeftHandComponent->GetName());
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("No Left Hand Component found."));
				}
			}
			else if (Component->GetTrackingSource() == EControllerHand::Right)
			{
				RightController = Component;
				UE_LOG(LogTemp, Log, TEXT("Right Controller found: %s"), *Component->GetName());

				// Find the scene component that contains a skeletal mesh
				RightHandComponent = FindHandComponent(RightController);
				if (RightHandComponent)
				{
					UE_LOG(LogTemp, Log, TEXT("Right Hand Component found: %s"), *RightHandComponent->GetName());
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("No Right Hand Component found."));
				}
			}

			if (LeftHandComponent && RightHandComponent)
			{
				break;
			}
		}
	}

	if (!LeftController || !RightController)
	{
		UE_LOG(LogTemp, Warning, TEXT("MotionControllerComponents not found in the scene."));
	}

	if (LeftController && !HasDynamicObjectComponent(LeftController))
	{
		//create dynamic objects and assign them during runtime
		UDynamicObject* LeftHandDyn = NewObject<UDynamicObject>(LeftController);
		if (LeftHandComponent)
		{
			LeftHandDyn->AttachToComponent(LeftHandComponent, FAttachmentTransformRules::KeepRelativeTransform);
		}
		else
		{
			LeftHandDyn->AttachToComponent(LeftController, FAttachmentTransformRules::KeepRelativeTransform);
		}
		//LeftHandDyn->MeshName = TEXT("LeftHandMesh");
		LeftHandDyn->IdSourceType = EIdSourceType::GeneratedId;
		LeftHandDyn->IsController = true;
		LeftHandDyn->IsRightController = false;
		LeftHandDyn->SyncUpdateWithPlayer = true;

		FString HMDDeviceName = FUtil::GetHMDDeviceName();

		if (HMDDeviceName.Contains(TEXT("Meta Quest 2"), ESearchCase::IgnoreCase))
		{
			LeftHandDyn->ControllerType = EC3DControllerType::Quest2;
		}
		else if (HMDDeviceName.Contains(TEXT("Meta Quest 3"), ESearchCase::IgnoreCase))
		{
			LeftHandDyn->ControllerType = EC3DControllerType::Quest3;
		}
		else if (HMDDeviceName.Contains(TEXT("Meta Quest Pro"), ESearchCase::IgnoreCase))
		{
			LeftHandDyn->ControllerType = EC3DControllerType::QuestPro;
		}
		else if (HMDDeviceName.Contains(TEXT("Meta Quest"), ESearchCase::IgnoreCase))
		{
			LeftHandDyn->ControllerType = EC3DControllerType::OculusRift;
		}
		else if (HMDDeviceName.Contains(TEXT("Pico Neo 2"), ESearchCase::IgnoreCase))
		{
			LeftHandDyn->ControllerType = EC3DControllerType::PicoNeo2;
		}
		else if (HMDDeviceName.Contains(TEXT("Pico Neo 3"), ESearchCase::IgnoreCase))
		{
			LeftHandDyn->ControllerType = EC3DControllerType::PicoNeo3;
		}
		else if (HMDDeviceName.Contains(TEXT("Pico 4"), ESearchCase::IgnoreCase))
		{
			LeftHandDyn->ControllerType = EC3DControllerType::PicoNeo4;
		}
		else if (HMDDeviceName.Contains(TEXT("HTC Vive"), ESearchCase::IgnoreCase))
		{
			LeftHandDyn->ControllerType = EC3DControllerType::Vive;
		}
		else
		{
			LeftHandDyn->ControllerType = EC3DControllerType::Quest3;
		}

		LeftHandDyn->RegisterComponent();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("LeftHandDyn already exists. Skipping creation."));
	}

	if (RightController && !HasDynamicObjectComponent(RightController))
	{
		UDynamicObject* RightHandDyn = NewObject<UDynamicObject>(RightController);
		if (RightHandComponent)
		{
			RightHandDyn->AttachToComponent(RightHandComponent, FAttachmentTransformRules::KeepRelativeTransform);
		}
		else
		{
			RightHandDyn->AttachToComponent(RightController, FAttachmentTransformRules::KeepRelativeTransform);
		}
		//RightHandDyn->MeshName = TEXT("RightHandMesh");
		RightHandDyn->IdSourceType = EIdSourceType::GeneratedId;
		RightHandDyn->IsController = true;
		RightHandDyn->IsRightController = true;
		RightHandDyn->SyncUpdateWithPlayer = true;

		FString HMDDeviceName = FUtil::GetHMDDeviceName();
		if (HMDDeviceName.Contains(TEXT("Meta Quest 2"), ESearchCase::IgnoreCase))
		{
			RightHandDyn->ControllerType = EC3DControllerType::Quest2;
		}
		else if (HMDDeviceName.Contains(TEXT("Meta Quest 3"), ESearchCase::IgnoreCase))
		{
			RightHandDyn->ControllerType = EC3DControllerType::Quest3;
		}
		else if (HMDDeviceName.Contains(TEXT("Meta Quest Pro"), ESearchCase::IgnoreCase))
		{
			RightHandDyn->ControllerType = EC3DControllerType::QuestPro;
		}
		else if (HMDDeviceName.Contains(TEXT("Meta Quest"), ESearchCase::IgnoreCase))
		{
			RightHandDyn->ControllerType = EC3DControllerType::OculusRift;
		}
		else if (HMDDeviceName.Contains(TEXT("Pico Neo 2"), ESearchCase::IgnoreCase))
		{
			RightHandDyn->ControllerType = EC3DControllerType::PicoNeo2;
		}
		else if (HMDDeviceName.Contains(TEXT("Pico Neo 3"), ESearchCase::IgnoreCase))
		{
			RightHandDyn->ControllerType = EC3DControllerType::PicoNeo3;
		}
		else if (HMDDeviceName.Contains(TEXT("Pico 4"), ESearchCase::IgnoreCase))
		{
			RightHandDyn->ControllerType = EC3DControllerType::PicoNeo4;
		}
		else if (HMDDeviceName.Contains(TEXT("HTC Vive"), ESearchCase::IgnoreCase))
		{
			RightHandDyn->ControllerType = EC3DControllerType::Vive;
		}
		else
		{
			RightHandDyn->ControllerType = EC3DControllerType::Quest3;
		}

		RightHandDyn->RegisterComponent();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("RightHandDyn already exists. Skipping creation."));
	}
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
		if (SkeletalMeshComponent && SkeletalMeshComponent->SkeletalMesh)
		{
			UE_LOG(LogTemp, Log, TEXT("Found Hand Scene Component: %s (Contains Skeletal Mesh: %s)"),
				*Child->GetName(), *SkeletalMeshComponent->SkeletalMesh->GetName());
			return Child;  // Return the scene component that holds the skeletal mesh
		}
	}

	return nullptr;  // No valid component found
}

bool ACognitive3DActor::HasDynamicObjectComponent(USceneComponent* Parent)
{
	if (!Parent)
	{
		return false;
	}

	// Get all child components recursively
	TArray<USceneComponent*> ChildrenComponents;
	Parent->GetChildrenComponents(true, ChildrenComponents);

	for (USceneComponent* Child : ChildrenComponents)
	{
		if (Cast<UDynamicObject>(Child)) // Check if the child is a UDynamicObject
		{
			return true;
		}
	}

	return false; // No existing UDynamicObject found
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