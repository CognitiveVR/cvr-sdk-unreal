// Fill out your copyright notice in the Description page of Project Settings.


#include "C3DComponents/OculusPassthrough.h"

// Sets default values for this component's properties
UOculusPassthrough::UOculusPassthrough()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UOculusPassthrough::BeginPlay()
{
	Super::BeginPlay();

	// ...
#ifdef INCLUDE_OCULUS_PASSTHROUGH
	auto cognitive = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	FindOculusXRPassthroughLayer(PassthroughLayer);
	IsPassthroughVisible = PassthroughLayer->IsVisible();
#endif
	
}


// Called every frame
void UOculusPassthrough::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
#ifdef INCLUDE_OCULUS_PASSTHROUGH
    currentTime += DeltaTime;

    if (currentTime > PassthroughSendInterval)
    {
        currentTime = 0;

        if (PassthroughLayer != NULL)
        {
            auto cognitive = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
            cognitive->sensors->RecordSensor("c3d.app.passthroughEnabled", static_cast<double>(PassthroughLayer->IsVisible()));
            if (IsPassthroughVisible != PassthroughLayer->IsVisible())
            {
                TSharedPtr<FJsonObject> properties = MakeShareable(new FJsonObject());
                properties->SetBoolField("New State", PassthroughLayer->IsVisible());
                float duration = GetWorld()->GetTimeSeconds() - lastEventTime;
                properties->SetNumberField("Duration", duration);
                cognitive->customEventRecorder->Send("Passthrough Layer Changed", properties);
                lastEventTime = GetWorld()->GetTimeSeconds();
            }
            IsPassthroughVisible = PassthroughLayer->IsVisible();
        }
        else
        {
            FindOculusXRPassthroughLayer(PassthroughLayer);
        }

    }
#endif
}

#ifdef INCLUDE_OCULUS_PASSTHROUGH
void UOculusPassthrough::FindOculusXRPassthroughLayer(UOculusXRPassthroughLayerComponent*& OculusPassthroughLayer)
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Loop through all actors in the scene
    for (TActorIterator<AActor> ActorIt(World); ActorIt; ++ActorIt)
    {
        AActor* Actor = *ActorIt;
        if (!Actor) continue; // Null check for each actor

        // Try to find the Oculus XR Passthrough Layer Component in the current actor
        UOculusXRPassthroughLayerComponent* OculusComponent = Cast<UOculusXRPassthroughLayerComponent>(Actor->GetComponentByClass(UOculusXRPassthroughLayerComponent::StaticClass()));

        if (!OculusComponent) continue; // Null check for the component

        // If the component is not null, you can now safely use OculusComponent
        OculusPassthroughLayer = OculusComponent;
    }

    // Repeat for all pawns in the scene (if necessary)
    for (TActorIterator<APawn> PawnIt(World); PawnIt; ++PawnIt)
    {
        APawn* Pawn = *PawnIt;
        if (!Pawn) continue; // Null check for each pawn

        // Similar component retrieval and check as done with actors
        UOculusXRPassthroughLayerComponent* OculusComponent = Cast<UOculusXRPassthroughLayerComponent>(Pawn->GetComponentByClass(UOculusXRPassthroughLayerComponent::StaticClass()));

        if (!OculusComponent) continue; // Null check for the component

        // Operations on the OculusComponent can be performed here
        OculusPassthroughLayer = OculusComponent;
    }
}
#endif
