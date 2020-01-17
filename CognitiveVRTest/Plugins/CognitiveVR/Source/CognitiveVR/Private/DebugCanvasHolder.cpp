// Fill out your copyright notice in the Description page of Project Settings.

#include "DebugCanvasHolder.h"

// Sets default values
ADebugCanvasHolder::ADebugCanvasHolder()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	WidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("Widget"));
	RootComponent = WidgetComponent;

	WidgetComponent->SetWorldScale3D(FVector(0.1, 0.1, 0.1));

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	static ConstructorHelpers::FClassFinder<UUserWidget> LoadedSearchResultWidgetClass(TEXT("/CognitiveVR/DebugWidget"));
	if (LoadedSearchResultWidgetClass.Succeeded())
	{
		WidgetClass = LoadedSearchResultWidgetClass.Class;
		WidgetComponent->SetWidgetClass(WidgetClass);
		WidgetComponent->SetDrawSize(FVector2D(800, 800));
		WidgetComponent->SetBlendMode(EWidgetBlendMode::Transparent);
		WidgetComponent->TranslucencySortPriority = 100;
	}
#endif
}

// Called when the game starts or when spawned
void ADebugCanvasHolder::BeginPlay()
{
	Super::BeginPlay();

	FString ValueReceived = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "EnableDebugCanvas", false);

	//canvas disabled, skip remaining
	if (ValueReceived.Len() > 0 && ValueReceived == "false")
		return;

	//add widget found at path (see constructor) to screen
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	WidgetComponent->SetRelativeLocation(FVector(0, 0, 0));
	WidgetComponent->SetWorldScale3D(FVector(0.1, 0.1, 0.1));
	WidgetInstance = CreateWidget<UUserWidget>(GetWorld(), WidgetClass);
	WidgetInstance->SetVisibility(ESlateVisibility::Visible);

	TArray<APlayerController*, FDefaultAllocator> controllers;
	GEngine->GetAllLocalPlayerControllers(controllers);
	CameraManager = controllers[0]->PlayerCameraManager;
#endif
}

void ADebugCanvasHolder::Tick(float delta)
{
	if (CameraManager == nullptr) { return; }
	FVector destination;
	float forwardOffset = 100;

	FRotator captureRotation = CameraManager->GetCameraRotation();
	FRotator r = FRotator(0, 180, 0);

	//BUG doesn't work correctly if there isn't a pawn/controller in the scene (camera manager doesn't return correct position)
	FVector End = CameraManager->GetCameraLocation() + captureRotation.Vector() * forwardOffset;

	captureRotation = FRotator(captureRotation.Quaternion() * FQuat(r));

	TeleportTo(End, captureRotation, false, false);
}