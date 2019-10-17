// Fill out your copyright notice in the Description page of Project Settings.

#include "CognitiveVR.h"
#include "DebugCanvasHolder.h"
#include "Blueprint/UserWidget.h"
#include "UObject/ConstructorHelpers.h"
#include "Blueprint/WidgetBlueprintGeneratedClass.h"
#include "Components/Widget.h"

// Sets default values
ADebugCanvasHolder::ADebugCanvasHolder()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	static ConstructorHelpers::FClassFinder<UUserWidget> LoadedSearchResultWidgetClass(TEXT("/CognitiveVR/DebugWidget"));
	if (LoadedSearchResultWidgetClass.Succeeded())
	{
		WidgetClass = LoadedSearchResultWidgetClass.Class;
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
#endif
}

