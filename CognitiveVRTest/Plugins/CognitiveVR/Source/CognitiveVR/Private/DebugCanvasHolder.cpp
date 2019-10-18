// Fill out your copyright notice in the Description page of Project Settings.

#include "CognitiveVR.h"
#include "DebugCanvasHolder.h"
#include "Blueprint/UserWidget.h"
#include "UObject/ConstructorHelpers.h"
#include "Blueprint/WidgetBlueprintGeneratedClass.h"

// Sets default values
ADebugCanvasHolder::ADebugCanvasHolder()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	WidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("Widget"));

	RootComponent->SetWorldScale3D(FVector(0.1, 0.1, 0.1));

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
	UUserWidget* SearchResultWidget = CreateWidget<UUserWidget>(GWorld->GetWorld(), WidgetClass);
	SearchResultWidget->AddToViewport();
	SearchResultWidget->SetVisibility(ESlateVisibility::Visible);

	//make a 3d widget component. parent that to the main camera

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

	FVector End = CameraManager->GetCameraLocation() + captureRotation.Vector() * forwardOffset;

	captureRotation = FRotator(captureRotation.Quaternion() * FQuat(r));

	TeleportTo(End, captureRotation, false, false);
}