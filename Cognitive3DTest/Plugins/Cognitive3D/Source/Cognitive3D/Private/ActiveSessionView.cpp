/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/

#include "ActiveSessionView.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Cognitive3D/Private/C3DComponents/FixationRecorder.h"

float AActiveSessionView::Remap(float num, float low1, float high1, float low2, float high2)
{
	return low2 + (num - low1) * (high2 - low2) / (high1 - low1);
}

// Sets default values
AActiveSessionView::AActiveSessionView()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FClassFinder<UUserWidget> LoadedSearchResultWidgetClass(TEXT("/Cognitive3D/Widgets/ActiveSessionViewWidget.ActiveSessionViewWidget_C"));
	if (LoadedSearchResultWidgetClass.Succeeded())
	{
		WidgetClass = LoadedSearchResultWidgetClass.Class;
		WidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("Widget"));
		WidgetComponent->SetWidgetClass(WidgetClass);
		WidgetComponent->SetDrawSize(FVector2D(SpectatorWidth, SpectatorHeight));
		WidgetComponent->SetBlendMode(EWidgetBlendMode::Transparent);
		WidgetComponent->TranslucencySortPriority = 100;
		WidgetComponent->SetTickWhenOffscreen(true);
		WidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		WidgetComponent->bRenderInMainPass = false; //hides the widget from the HMD
		RootComponent = WidgetComponent;
	}
}

// Called when the game starts or when spawned
void AActiveSessionView::BeginPlay()
{
	Super::BeginPlay();

	PlayerController = UGameplayStatics::GetGameInstance(this)->GetFirstLocalPlayerController();

	//pass reference of this active session view to widget, so it can pull values to display
	auto widgetInstance = WidgetComponent->GetUserWidgetObject();
	if (widgetInstance != NULL)
	{
		if (widgetInstance->GetClass()->ImplementsInterface(UActiveSessionViewRequired::StaticClass()))
		{
			IActiveSessionViewRequired::Execute_Initialize(widgetInstance, this);
		}
	}
	FTimerHandle OneSeconHandle;
	GetWorld()->GetGameInstance()->GetTimerManager().SetTimer(OneSeconHandle, this, &AActiveSessionView::DelaySetupWidget, 1, false);
}

void AActiveSessionView::DelaySetupWidget()
{
	auto cognitiveActor = ACognitive3DActor::GetCognitive3DActor();
	if (cognitiveActor == nullptr)
	{
		return;
	}
	FixationRecorder = Cast<UFixationRecorder>(cognitiveActor->GetComponentByClass(UFixationRecorder::StaticClass()));

	if (FixationRecorder == NULL)
	{
		GLog->Log("Cognitive3D::ActiveSessionView::BeginPlay cannot find FixationRecorder in scene!");
	}

	if (WidgetComponent != NULL)
	{
		auto widgetInstance = WidgetComponent->GetUserWidgetObject();
		UHeadMountedDisplayFunctionLibrary::SetSpectatorScreenModeTexturePlusEyeLayout(FVector2D(0, 0), FVector2D(1, 1), FVector2D(0, 0), FVector2D(1, 1),true,false,true);
		UTextureRenderTarget2D* widgetTextureRT2D = WidgetComponent->GetRenderTarget();
		UHeadMountedDisplayFunctionLibrary::SetSpectatorScreenTexture((UTexture*)widgetTextureRT2D);
		UHeadMountedDisplayFunctionLibrary::SetSpectatorScreenMode(ESpectatorScreenMode::TexturePlusEye);
	}
}

void AActiveSessionView::Tick(float delta)
{
	
}

TArray<FVector> AActiveSessionView::GetProjectedFixations()
{
	if (FixationRecorder == NULL) { return TArray<FVector>(); }
	if (PlayerController == NULL) { return TArray<FVector>(); }

	TArray<FVector> RecentFixations;

	TArray<FFixation> points = FixationRecorder->GetRecentFixationPoints();
	FVector worldPosition;

	for (int32 i = 0; i < points.Num(); i++)
	{
		if (points[i].IsLocal)
		{
			worldPosition = points[i].Transformation.TransformPosition(points[i].LocalPosition);
		}
		else
		{
			worldPosition = points[i].WorldPosition;
		}

		//project from world to screen
		FVector2D screenPosition;
		PlayerController->ProjectWorldLocationToScreen(worldPosition, screenPosition);

		//remap from hmd to spectator view
		float x = Remap(screenPosition.X, 0, 2880, 0, 1280);
		float y = Remap(screenPosition.Y, 0, 1600, 0, 720);

		//slightly shift the x value (because projection is off center)
		x += 64;

		//add to array
		RecentFixations.Add(FVector(x, y, points[i].MaxRadius));
	}

	return RecentFixations;
}

TArray<FVector2D> AActiveSessionView::GetProjectedSaccades()
{
	if (FixationRecorder == NULL) { return TArray<FVector2D>(); }
	if (PlayerController == NULL) { return TArray<FVector2D>(); }
	TArray<TSharedPtr<FC3DGazePoint>> eyePositions = FixationRecorder->GetRecentEyePositions();
	TArray<FVector2D> adjustedEyePositions;

	FVector worldPosition;

	for (int32 i = 0; i < eyePositions.Num(); i++)
	{
		if (eyePositions[i]->IsLocal && eyePositions[i]->Parent != NULL)
		{
			worldPosition = eyePositions[i]->Parent->GetComponentTransform().TransformPosition(eyePositions[i]->LocalPosition);
		}
		else
		{
			worldPosition = eyePositions[i]->WorldPosition;
		}
		
		FVector2D screenPosition;
		PlayerController->ProjectWorldLocationToScreen(worldPosition, screenPosition);

		//remap from hmd to spectator view
		float x = Remap(screenPosition.X, 0, 2880, 0, 1280);
		float y = Remap(screenPosition.Y, 0, 1600, 0, 720);

		//slightly shift the x value (because projection is off center)
		x += 64;

		//add to array
		adjustedEyePositions.Add(FVector2D(x, y));
	}

	return adjustedEyePositions;
}
