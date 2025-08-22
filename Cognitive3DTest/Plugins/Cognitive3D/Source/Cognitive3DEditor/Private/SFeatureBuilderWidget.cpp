// Fill out your copyright notice in the Description page of Project Settings.


#include "SFeatureBuilderWidget.h"
#include "SlateOptMacros.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Misc/Paths.h"
#include "HAL/PlatformProcess.h"
#include "Engine/Blueprint.h"
#include "Editor.h"
#include "Styling/SlateStyleRegistry.h"
#include "CognitiveEditorStyle.h"
#include "Cognitive3D/Private/C3DComponents/RemoteControls.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "BlueprintEditorUtils.h"
#include <KismetEditorUtilities.h>

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SFeatureBuilderWidget::Construct(const FArguments& InArgs)
{
	// Initialize the list of features
	FeatureList = {
		{ TEXT("DynamicObjects"), FText::FromString("Dynamic Objects"),
		  FCognitiveEditorStyle::GetBrush(TEXT("CognitiveEditor.DynamicObjectsIcon")),
		  FText::FromString("Manage Dynamic Objects in your project.") },

		{ TEXT("ExitPoll"), FText::FromString("Exit Poll"),
		  FCognitiveEditorStyle::GetBrush(TEXT("CognitiveEditor.ExitPollIcon")),
		  FText::FromString("Set up ExitPoll surveys for your project.") },

		{ TEXT("RemoteControls"), FText::FromString("Remote Controls"),
		  FCognitiveEditorStyle::GetBrush(TEXT("CognitiveEditor.RemoteControlsIcon")),
		  FText::FromString("Set up remote control variables for your project") },

		{ TEXT("CustomEvents"), FText::FromString("Custom Events"),
		  FCognitiveEditorStyle::GetBrush(TEXT("CognitiveEditor.CustomEvents")),
		  FText::FromString("Samples showing how to record custom events in C++ and Blueprints") }
	};

	ChildSlot
		[
			SNew(SBox)
				.WidthOverride(600).HeightOverride(400)
				[
					SNew(SWidgetSwitcher)
						.WidgetIndex(this, &SFeatureBuilderWidget::GetPageIndex)

						// List view slot
						+ SWidgetSwitcher::Slot()
						[BuildFeatureList()]

					// Detail view slot
					+ SWidgetSwitcher::Slot()
						[BuildFeatureDetail()]
				]
		];
}
TSharedRef<SWidget> SFeatureBuilderWidget::BuildFeatureList()
{
	TSharedRef<SVerticalBox> Box = SNew(SVerticalBox);
	for (const FFeature& Feature : FeatureList)
	{
		Box->AddSlot().AutoHeight().Padding(5)
			[
				SNew(SButton)
					.HAlign(HAlign_Left)
					.OnClicked(this, &SFeatureBuilderWidget::OnFeatureClicked, Feature.Key)
					[
						SNew(SHorizontalBox)
							+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
							[SNew(SImage).Image(Feature.Icon)]
							+ SHorizontalBox::Slot().FillWidth(1).Padding(10, 0)
							[
								SNew(SVerticalBox)
									+ SVerticalBox::Slot().AutoHeight()
									[SNew(STextBlock)
									.Text(Feature.DisplayName)
									.Font(FEditorStyle::GetFontStyle("HeadingExtraSmall"))]
									+ SVerticalBox::Slot().AutoHeight()
									[SNew(STextBlock)
									.Text(Feature.Desc)
									.AutoWrapText(true)
									.Font(FEditorStyle::GetFontStyle("SmallText"))]
							]
					]
			];
	}
	return SNew(SScrollBox)
		+ SScrollBox::Slot()[Box];
}
TSharedRef<SWidget> SFeatureBuilderWidget::BuildFeatureDetail()
{
	return SNew(SVerticalBox)
		// Back button
		+ SVerticalBox::Slot().AutoHeight().Padding(5)
		[SNew(SButton)
		.Text(FText::FromString("<-Back"))
		.OnClicked(this, &SFeatureBuilderWidget::OnBackClicked)]

		// Dynamic detail area
		+ SVerticalBox::Slot().FillHeight(1).Padding(5)
		[
			SAssignNew(DetailBox, SBox)
				[CreateDetailWidget()]
		];
}
TSharedRef<SWidget> SFeatureBuilderWidget::CreateDetailWidget()
{
	UE_LOG(LogTemp, Warning, TEXT("Creating detail widget for feature: %s"), *SelectedFeature.ToString());
	if (SelectedFeature == TEXT("DynamicObjects"))
	{
		return SNew(SDynamicObjectManagerWidget);
	}
	else if (SelectedFeature == TEXT("ExitPoll"))
	{
		return SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(5)
			[SNew(STextBlock)
			.Text(FText::FromString("Exit Poll Setup"))
			.Font(FEditorStyle::GetFontStyle("Heading"))]
			+ SVerticalBox::Slot().AutoHeight().Padding(5)
			[SNew(STextBlock)
			.Text(FText::FromString("ExitPoll allows you to set up a survey for players at any point in the level progression.\nBelow is a button to open the documentation in a browser, and another to add the exitpoll object to the currently open level"))
			.AutoWrapText(true)]
			+ SVerticalBox::Slot().AutoHeight().Padding(5)
			[SNew(SButton)
			.Text(FText::FromString("ExitPoll Documentation"))
			.OnClicked(this, &SFeatureBuilderWidget::OnLaunchExitPoll)]
			+SVerticalBox::Slot().AutoHeight().Padding(5)
			[SNew(SButton)
			.Text(FText::FromString("Add ExitPoll Blueprint"))
			.OnClicked(this, &SFeatureBuilderWidget::OnAddExitPollBlueprint)];
	}
	else if (SelectedFeature == TEXT("RemoteControls"))
	{
		return SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(5)
			[SNew(STextBlock)
			.Text(FText::FromString("Remote Controls"))
			.Font(FEditorStyle::GetFontStyle("Heading"))]
			+ SVerticalBox::Slot().AutoHeight().Padding(5)
			[SNew(STextBlock)
			.Text(FText::FromString("Add Remote Controls component to the BP_Cognitive3DActor to allow for the use of remote control variables from the dashboard.\nBelow is a button to check out the documentation, and another to add the component."))
			.AutoWrapText(true)]
			+ SVerticalBox::Slot().AutoHeight().Padding(5)[SNew(SButton).Text(FText::FromString("RemoteControls Documentation")).OnClicked(this, &SFeatureBuilderWidget::OnLaunchRemoteControls)]
			+ SVerticalBox::Slot().AutoHeight().Padding(5)[SNew(SButton).Text(FText::FromString("Add RemoteControls Component")).OnClicked(this, &SFeatureBuilderWidget::OnAddRemoteControlsComponent)];
	}
	else if (SelectedFeature == TEXT("CustomEvents"))
	{
			return SNew(SScrollBox)
			+ SScrollBox::Slot()
			[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight().Padding(5)
					[SNew(STextBlock)
					.Text(FText::FromString("Custom Events"))
					.Font(FEditorStyle::GetFontStyle("Heading"))]
					+ SVerticalBox::Slot().AutoHeight().Padding(5)
					[SNew(SButton)
					.Text(FText::FromString("Custom Event Documentation"))
					.OnClicked(this, &SFeatureBuilderWidget::OnLaunchCustomEvents)]
					+ SVerticalBox::Slot().AutoHeight().Padding(5)
					[SNew(STextBlock)
					.Text(FText::FromString("Here is a Blueprint and C++ sample showing simple use of the custom event."))
					.AutoWrapText(true)]
					+ SVerticalBox::Slot()
						.Padding(5)
						.HAlign(HAlign_Center)
						.MaxHeight(200)
					[
						SNew(SHorizontalBox)
						+SHorizontalBox::Slot()
							.MaxWidth(500)
							.Padding(5)
						[
						SNew(SImage)
						.Image(FCognitiveEditorStyle::GetBrush(TEXT("CognitiveEditor.CustomEventsSimple")))
						]
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(5)
					[SNew(STextBlock)
					.Text(FText::FromString("TWeakPtr<FAnalyticsProviderCognitive3D> provider = FAnalyticsCognitive3D::Get().GetCognitive3DProvider();\nif (provider.IsValid())\n{\n	//send an event with a name\n	provider.Pin()->customEventRecorder->Send(\"My Event\");\n	//send an event with a name and a position\n	provider.Pin()->customEventRecorder->Send(\"My Event With Position\", FVector(0, 100, 0));\n}"))
					.AutoWrapText(true)]
			];
	}

	// Fallback
	return SNew(STextBlock)
		.Text(FText::FromString("Feature not implemented."));
}
FReply SFeatureBuilderWidget::OnFeatureClicked(FName FeatureKey)
{
	SelectedFeature = FeatureKey;
	CurrentMode = EPageMode::Detail;

	// Update the detail widget content
	if (DetailBox.IsValid())
	{
		DetailBox->SetContent(CreateDetailWidget());
	}
	return FReply::Handled();
}
FReply SFeatureBuilderWidget::OnLaunchExitPoll()
{
	const FString URL = TEXT("https://docs.cognitive3d.com/unreal/exitpoll/");
	FPlatformProcess::LaunchURL(*URL, nullptr, nullptr);
	return FReply::Handled();
}
FReply SFeatureBuilderWidget::OnAddExitPollBlueprint()
{
	if (GEditor)
	{
		UClass* BPClass = LoadObject<UClass>(nullptr, TEXT("/Cognitive3D/ExitPoll.ExitPoll_C"));
		UWorld* World = GEditor->GetEditorWorldContext().World();
		if (World && BPClass)
		{
			World->SpawnActor<AActor>(BPClass, FVector::ZeroVector, FRotator::ZeroRotator);

			// Save the current level/map
			UWorld* EditorWorld = GEditor->GetEditorWorldContext().World();
			if (EditorWorld && EditorWorld->GetCurrentLevel())
			{
				ULevel* CurrentLevel = EditorWorld->GetCurrentLevel();
				UPackage* LevelPackage = CurrentLevel->GetOutermost();
				FString LevelFilename = FPackageName::LongPackageNameToFilename(LevelPackage->GetName(), FPackageName::GetMapPackageExtension());
				bool bLevelSaved = UPackage::SavePackage(
					LevelPackage,
					nullptr,
					RF_Standalone,
					*LevelFilename,
					GError,
					nullptr,
					true,
					true,
					SAVE_None
				);
				if (bLevelSaved)
				{
					UE_LOG(LogTemp, Log, TEXT("Level auto-saved: %s"), *LevelFilename);
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("Failed to auto-save level: %s"), *LevelFilename);
				}
			}
		}
	}
	return FReply::Handled();
}
FReply SFeatureBuilderWidget::OnLaunchRemoteControls()
{
	FPlatformProcess::LaunchURL(TEXT("https://docs.cognitive3d.com/unreal/remote-controls/"), nullptr, nullptr);
	return FReply::Handled();
}
FReply SFeatureBuilderWidget::OnAddRemoteControlsComponent()
{
	UE_LOG(LogTemp, Log, TEXT("Adding RemoteControls component to BP_Cognitive3DActor blueprint"));
	// Load the Blueprint-generated class directly from plugin content
	static const FString ClassPath = TEXT("/Cognitive3D/BP_Cognitive3DActor.BP_Cognitive3DActor_C");
	UClass* ClassPtr = LoadObject<UClass>(nullptr, *ClassPath);
	if (ClassPtr)
	{
		UBlueprint* BP = Cast<UBlueprint>(ClassPtr->ClassGeneratedBy);
		if (BP)
		{
			BP->Modify();
			USimpleConstructionScript* SCS = BP->SimpleConstructionScript;
			if (SCS)
			{
				USCS_Node* NewNode = SCS->CreateNode(URemoteControls::StaticClass(), TEXT("RemoteControls"));
				SCS->AddNode(NewNode);
				FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(BP);
				UE_LOG(LogTemp, Log, TEXT("RemoteControls component added to blueprint asset at %s."), *ClassPath);

				// Now save the blueprint asset automatically
				UPackage* Package = BP->GetOutermost();
				FString PackageFileName = FPackageName::LongPackageNameToFilename(Package->GetName(), FPackageName::GetAssetPackageExtension());
				bool bSaved = UPackage::SavePackage(
					Package,
					BP,
					RF_Standalone,
					*PackageFileName,
					GError,
					nullptr,
					true,
					true,
					SAVE_None
				);
				if (bSaved)
				{
					UE_LOG(LogTemp, Log, TEXT("Blueprint auto-saved: %s"), *PackageFileName);

					// Compile the blueprint
					FKismetEditorUtilities::CompileBlueprint(BP);
					UE_LOG(LogTemp, Log, TEXT("Blueprint compiled: %s"), *BP->GetName());

					// Save the current level/map
					UWorld* EditorWorld = GEditor->GetEditorWorldContext().World();
					if (EditorWorld && EditorWorld->GetCurrentLevel())
					{
						ULevel* CurrentLevel = EditorWorld->GetCurrentLevel();
						UPackage* LevelPackage = CurrentLevel->GetOutermost();
						FString LevelFilename = FPackageName::LongPackageNameToFilename(LevelPackage->GetName(), FPackageName::GetMapPackageExtension());
						bool bLevelSaved = UPackage::SavePackage(
							LevelPackage,
							nullptr,
							RF_Standalone,
							*LevelFilename,
							GError,
							nullptr,
							true,
							true,
							SAVE_None
						);
						if (bLevelSaved)
						{
							UE_LOG(LogTemp, Log, TEXT("Level auto-saved: %s"), *LevelFilename);
						}
						else
						{
							UE_LOG(LogTemp, Error, TEXT("Failed to auto-save level: %s"), *LevelFilename);
						}
					}
					UE_LOG(LogTemp, Log, TEXT("Blueprint auto-saved: %s"), *PackageFileName);
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("Failed to auto-save blueprint: %s"), *PackageFileName);
				}
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Blueprint %s has no SimpleConstructionScript."), *ClassPath);
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Class %s is not generated from a UBlueprint."), *ClassPath);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load class at %s."), *ClassPath);
	}
	return FReply::Handled();
}

FReply SFeatureBuilderWidget::OnLaunchCustomEvents()
{
	FPlatformProcess::LaunchURL(TEXT("https://docs.cognitive3d.com/unreal/customevents/"), nullptr, nullptr);
	return FReply::Handled();
}

FReply SFeatureBuilderWidget::OnBackClicked()
{
	CurrentMode = EPageMode::List;
	return FReply::Handled();
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION
