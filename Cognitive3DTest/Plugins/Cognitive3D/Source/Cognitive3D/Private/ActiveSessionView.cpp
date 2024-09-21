/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/

#include "ActiveSessionView.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "GameFramework/GameUserSettings.h"

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
		UHeadMountedDisplayFunctionLibrary::SetSpectatorScreenModeTexturePlusEyeLayout(FVector2D(0, 0), FVector2D(1, 1), FVector2D(0, 0), FVector2D(1, 1), true, false, true);
		UTextureRenderTarget2D* widgetTextureRT2D = WidgetComponent->GetRenderTarget();
		UHeadMountedDisplayFunctionLibrary::SetSpectatorScreenTexture((UTexture*)widgetTextureRT2D);
		UHeadMountedDisplayFunctionLibrary::SetSpectatorScreenMode(ESpectatorScreenMode::TexturePlusEye);
	}


	//screen size debugging
	/*
	auto provider = FAnalyticsCognitive3D::Get().GetCognitive3DProvider();

	UGameUserSettings* MyGameSettings = GEngine->GetGameUserSettings();
	FIntPoint screenResolution = MyGameSettings->GetScreenResolution();
	provider.Pin()->SetSessionProperty("ScreenResolution", FString::FromInt(screenResolution.X) + "  " + FString::FromInt(screenResolution.Y));
	FIntPoint desktopResolution = MyGameSettings->GetDesktopResolution();
	provider.Pin()->SetSessionProperty("DesktopResolution", FString::FromInt(desktopResolution.X) + "  " + FString::FromInt(desktopResolution.Y));

	FVector WorldPosition;
	FVector2D ScreenPosition;
	ULocalPlayer* const LP = PlayerController ? PlayerController->GetLocalPlayer() : nullptr;
	if (LP && LP->ViewportClient)
	{
		// get the projection data
		FSceneViewProjectionData ProjectionData;
		if (LP->GetProjectionData(LP->ViewportClient->Viewport, ProjectionData))
		{
			FMatrix const ViewProjectionMatrix = ProjectionData.ComputeViewProjectionMatrix();
			bool bResult = FSceneView::ProjectWorldToScreen(WorldPosition, ProjectionData.GetConstrainedViewRect(), ViewProjectionMatrix, ScreenPosition);
			provider.Pin()->SetSessionProperty("ProjectionRect", FString::FromInt(ProjectionData.GetViewRect().Width()) + "  " + FString::FromInt(ProjectionData.GetViewRect().Height()));
		}
	}*/
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
		float x = Remap(screenPosition.X, 0, GetHMDWidth(), 0, SpectatorWidth);
		float y = Remap(screenPosition.Y, 0, GetHMDHeight(), 0, SpectatorHeight);

		//slightly shift the x value (because projection is off center)
		x += GetCenterXOffset();
		y += GetCenterYOffset();

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
		PlayerController->ProjectWorldLocationToScreen(worldPosition, screenPosition, false);

		//remap from hmd to spectator view
		float x = Remap(screenPosition.X, 0, GetHMDWidth(), 0, SpectatorWidth);
		float y = Remap(screenPosition.Y, 0, GetHMDHeight(), 0, SpectatorHeight);

		//slightly shift the x value (because projection is off center)
		x += GetCenterXOffset();
		y += GetCenterYOffset();

		//add to array
		adjustedEyePositions.Add(FVector2D(x, y));
	}

	//current eye tracking point
	/*int32 Index = eyePositions.Num() - 1;
	if (Index > 0)
	{
		if (eyePositions[Index]->IsLocal && eyePositions[Index]->Parent != NULL)
		{
			worldPosition = eyePositions[Index]->Parent->GetComponentTransform().TransformPosition(eyePositions[Index]->LocalPosition);
		}
		else
		{
			worldPosition = eyePositions[Index]->WorldPosition;
		}
		FVector2D screenPosition;
		PlayerController->ProjectWorldLocationToScreen(worldPosition, screenPosition);

		//auto provider = FAnalyticsCognitive3D::Get().GetCognitive3DProvider();
		//provider.Pin()->sensors->RecordSensor("ScreenPosition.X", screenPosition.X);
		//provider.Pin()->sensors->RecordSensor("ScreenPosition.Y", screenPosition.Y);
	}
	*/

	return adjustedEyePositions;
}
