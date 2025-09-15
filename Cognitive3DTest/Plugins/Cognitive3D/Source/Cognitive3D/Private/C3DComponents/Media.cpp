// Fill out your copyright notice in the Description page of Project Settings.


#include "C3DComponents/Media.h"

// Sets default values for this component's properties
UMedia::UMedia()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UMedia::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UMedia::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

#if WITH_EDITOR
void UMedia::OnRegister()
{
	Super::OnRegister();

	// Check if the owner actor has a Dynamic Object component
	if (AActor* Owner = GetOwner())
	{
		UDynamicObject* ExistingDynamicObject = Owner->FindComponentByClass<UDynamicObject>();
		if (!ExistingDynamicObject)
		{
			// Add a Dynamic Object component if it doesn't exist
			UDynamicObject* NewDynamicObject = NewObject<UDynamicObject>(Owner, UDynamicObject::StaticClass(), TEXT("DynamicObject"));
			if (NewDynamicObject)
			{
				Owner->AddInstanceComponent(NewDynamicObject);

				// Find a mesh component to attach to, otherwise use root
				USceneComponent* AttachParent = Owner->GetRootComponent();

				// Look for any mesh component (StaticMesh, SkeletalMesh, etc.)
				UMeshComponent* MeshComp = Owner->FindComponentByClass<UMeshComponent>();
				if (MeshComp)
				{
					AttachParent = MeshComp;
				}

				if (AttachParent)
				{
					NewDynamicObject->AttachToComponent(AttachParent, FAttachmentTransformRules::KeepRelativeTransform);
				}

				NewDynamicObject->RegisterComponent();
			}
		}
	}
}
#endif

