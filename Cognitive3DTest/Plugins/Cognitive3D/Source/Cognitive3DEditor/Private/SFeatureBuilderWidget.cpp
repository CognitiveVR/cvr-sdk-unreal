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
#include "SegmentAnalytics.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "CognitiveEditorTools.h"

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

    // Validate developer key on startup
    FString developerKey = FCognitiveEditorTools::GetInstance()->GetDeveloperKey().ToString();
	if (!developerKey.IsEmpty())
	{
		CheckForExpiredDeveloperKey(developerKey);
	}

	ChildSlot
		[
			SNew(SBox)
				.WidthOverride(600).HeightOverride(400)
				[
					SNew(SVerticalBox)
						+SVerticalBox::Slot()
						.FillHeight(1)
						.Padding(5)
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
				]
		];
}
TSharedRef<SWidget> SFeatureBuilderWidget::BuildFeatureList()
{
	TSharedRef<SVerticalBox> Box = SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(5)
		[
			SNew(STextBlock)
			.Text(FText::FromString("Explore the features of our platform. Each feature unlocks powerful capabilities you can use in your experience, from Dynamic Objects to live control and more."))
			.Font(FEditorStyle::GetFontStyle("Heading"))
		];
	for (const FFeature& Feature : FeatureList)
	{
		Box->AddSlot()
			.AutoHeight()
			.Padding(5)
			[
				SNew(SButton)
					.ButtonStyle(FCognitiveEditorStyle::GetStyleSet().Get(), "CognitiveEditor.FeatureButton")
					.HAlign(HAlign_Left)
					.IsEnabled_Lambda([this]() {
						return bIsDeveloperKeyValid;
					})
					.OnClicked(this, &SFeatureBuilderWidget::OnFeatureClicked, Feature.Key)
					[
						SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							[
								SNew(SImage).Image(Feature.Icon)]
								+ SHorizontalBox::Slot()
								.FillWidth(1)
								.Padding(10, 0)
								.VAlign(VAlign_Center)
								[
									SNew(SVerticalBox)
										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
										.Text(Feature.DisplayName)
										.TextStyle(FCognitiveEditorStyle::GetStyleSet().Get(), "CognitiveEditor.FeatureButtonTitle")
										]
											+ SVerticalBox::Slot()
											.AutoHeight()
										[
											SNew(STextBlock)
										.Text(Feature.Desc)
										.AutoWrapText(true)
										.Font(FEditorStyle::GetFontStyle("SmallText"))
										.TextStyle(FCognitiveEditorStyle::GetStyleSet().Get(), "CognitiveEditor.FeatureButtonText")
										]
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
		// Back button and header
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Left)
		.Padding(5)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0, 0, 15, 0)
			[
				SNew(SBox)
				.WidthOverride(FOptionalSize())
				[
					SNew(SButton)
					.Text(FText::FromString("<-Back"))
					.OnClicked(this, &SFeatureBuilderWidget::OnBackClicked)
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(this, &SFeatureBuilderWidget::GetSelectedFeatureDisplayName)
				.TextStyle(FCognitiveEditorStyle::GetStyleSet().Get(), "CognitiveEditor.FeatureButtonTitle")
			]
		]

		// Dynamic detail area
		+ SVerticalBox::Slot()
		.FillHeight(1)
		.Padding(5)
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
		USegmentAnalytics::Get()->TrackEvent(TEXT("DynamicManagerWindow_Opened"), TEXT("FeatureBuilderWindow"));
		return SNew(SDynamicObjectManagerWidget);
	}
	else if (SelectedFeature == TEXT("ExitPoll"))
	{
		USegmentAnalytics::Get()->TrackEvent(TEXT("ExitpollWindow_Opened"), TEXT("FeatureBuilderWindow"));
		return SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Left)
			.Padding(5)
			[
				SNew(STextBlock)
				.Text(FText::FromString("Exit Poll allows you to gather feedback from users at any point during their session. Create custom questionnaires to understand user experience, collect ratings, and gain insights into user behavior and preferences."))
				.AutoWrapText(true)
				.Font(FEditorStyle::GetFontStyle("RegularText"))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(5)
			[
				SNew(SBox)
				.WidthOverride(FOptionalSize())
				[
					SNew(SButton)
					.Text(FText::FromString("ExitPoll Documentation"))
					.OnClicked(this, &SFeatureBuilderWidget::OnLaunchExitPoll)
				]
			]
			+SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(5)
			[
				SNew(SBox)
				.WidthOverride(FOptionalSize())
				[
					SNew(SButton)
					.Text(FText::FromString("Add ExitPoll Blueprint"))
					.OnClicked(this, &SFeatureBuilderWidget::OnAddExitPollBlueprint)
				]
			];
	}
	else if (SelectedFeature == TEXT("RemoteControls"))
	{
		USegmentAnalytics::Get()->TrackEvent(TEXT("RemoteControlsWindow_Opened"), TEXT("FeatureBuilderWindow"));
		return SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Left)
			.Padding(5)
			[
				SNew(STextBlock)
				.Text(FText::FromString("Remote Controls allow you to modify variables in your application in real-time from the Cognitive3D dashboard. Change gameplay parameters, adjust difficulty, or update content without rebuilding your application."))
				.AutoWrapText(true)
				.Font(FEditorStyle::GetFontStyle("RegularText"))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(5)
			[
				SNew(SBox)
				.WidthOverride(FOptionalSize())
				[
					SNew(SButton)
					.Text(FText::FromString("RemoteControls Documentation"))
					.OnClicked(this, &SFeatureBuilderWidget::OnLaunchRemoteControls)
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(5)
			[
				SNew(SBox)
				.WidthOverride(FOptionalSize())
				[
					SNew(SButton)
					.Text_Lambda([this]() {
						return HasRemoteControlsComponent() 
							? FText::FromString("Remove RemoteControls Component")
							: FText::FromString("Add RemoteControls Component");
					})
					.OnClicked(this, &SFeatureBuilderWidget::OnToggleRemoteControlsComponent)
				]
			];
	}
	else if (SelectedFeature == TEXT("CustomEvents"))
	{
		USegmentAnalytics::Get()->TrackEvent(TEXT("CustomEventsWindow_Opened"), TEXT("FeatureBuilderWindow"));
			return SNew(SScrollBox)
			+ SScrollBox::Slot()
			[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(HAlign_Left)
						.Padding(5)
					[
						SNew(STextBlock)
						.Text(FText::FromString("Custom Events let you track specific interactions, achievements, or moments in your application. Send events with custom data to analyze user behavior, measure engagement, and understand how users interact with your content."))
						.AutoWrapText(true)
						.Font(FEditorStyle::GetFontStyle("RegularText"))
					]
					+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(HAlign_Center)
						.Padding(5)
					[
						SNew(SBox)
						.WidthOverride(FOptionalSize())
						[
							SNew(SButton)
							.Text(FText::FromString("Custom Event Documentation"))
							.OnClicked(this, &SFeatureBuilderWidget::OnLaunchCustomEvents)
						]
					]
					+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(HAlign_Left)
						.Padding(5)
					[
						SNew(STextBlock)
						.Text(FText::FromString("Blueprint Example:"))
						.AutoWrapText(true)
						.Font(FEditorStyle::GetFontStyle("HeadingSmall"))
					]
					+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(HAlign_Center)
						.Padding(5)
					[
						SNew(SBox)
						.WidthOverride(FOptionalSize())
						.HeightOverride(FOptionalSize())
						[
							SNew(SImage)
							.Image(FCognitiveEditorStyle::GetBrush(TEXT("CognitiveEditor.CustomEventsSimple")))
						]
					]
					+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(HAlign_Left)
						.Padding(5, 15, 5, 5)
					[
						SNew(STextBlock)
						.Text(FText::FromString("C++ Example:"))
						.Font(FEditorStyle::GetFontStyle("HeadingSmall"))
					]
					+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(HAlign_Center)
						.Padding(5)
					[
						SNew(SBox)
						.WidthOverride(FOptionalSize())
						[
							SNew(SBorder)
							.BorderImage(FCognitiveEditorStyle::GetBrush(TEXT("CognitiveEditor.CodeBox")))
							.Padding(8)
							[
								SNew(SMultiLineEditableText)
								.Text(FText::FromString("TWeakPtr<FAnalyticsProviderCognitive3D> provider = FAnalyticsCognitive3D::Get().GetCognitive3DProvider();\nif (provider.IsValid())\n{\n\t//send an event with a name\n\tprovider.Pin()->customEventRecorder->Send(\"My Event\");\n\t//send an event with a name and a position\n\tprovider.Pin()->customEventRecorder->Send(\"My Event With Position\", FVector(0, 100, 0));\n}"))
								.IsReadOnly(true)
								.SelectAllTextWhenFocused(false)
								.Font(FEditorStyle::Get().GetFontStyle("MonoFont"))
								.AutoWrapText(false)
							]
						]
					]
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


bool SFeatureBuilderWidget::HasRemoteControlsComponent() const
{
	static const FString ClassPath = TEXT("/Cognitive3D/BP_Cognitive3DActor.BP_Cognitive3DActor_C");
	UClass* ClassPtr = LoadObject<UClass>(nullptr, *ClassPath);
	if (ClassPtr)
	{
		UBlueprint* BP = Cast<UBlueprint>(ClassPtr->ClassGeneratedBy);
		if (BP && BP->SimpleConstructionScript)
		{
			USimpleConstructionScript* SCS = BP->SimpleConstructionScript;
			// Check if RemoteControls component already exists
			for (USCS_Node* Node : SCS->GetAllNodes())
			{
				if (Node && Node->ComponentClass && Node->ComponentClass == URemoteControls::StaticClass())
				{
					return true;
				}
			}
		}
	}
	return false;
}

FReply SFeatureBuilderWidget::OnToggleRemoteControlsComponent()
{
	if (HasRemoteControlsComponent())
	{
		// Remove the component
		UE_LOG(LogTemp, Log, TEXT("Removing RemoteControls component from BP_Cognitive3DActor blueprint"));
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
					// Find and remove RemoteControls component
					USCS_Node* NodeToRemove = nullptr;
					for (USCS_Node* Node : SCS->GetAllNodes())
					{
						if (Node && Node->ComponentClass && Node->ComponentClass == URemoteControls::StaticClass())
						{
							NodeToRemove = Node;
							break;
						}
					}
					
					if (NodeToRemove)
					{
						SCS->RemoveNode(NodeToRemove);
						FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(BP);
						UE_LOG(LogTemp, Log, TEXT("RemoteControls component removed from blueprint asset at %s."), *ClassPath);
						
						// Save and compile the blueprint
						UPackage* Package = BP->GetOutermost();
						FString PackageFileName = FPackageName::LongPackageNameToFilename(Package->GetName(), FPackageName::GetAssetPackageExtension());
						bool bSaved = UPackage::SavePackage(Package, BP, RF_Standalone, *PackageFileName, GError, nullptr, true, true, SAVE_None);
						if (bSaved)
						{
							FKismetEditorUtilities::CompileBlueprint(BP);
							UE_LOG(LogTemp, Log, TEXT("Blueprint saved and compiled after removing RemoteControls component"));

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
				}
			}
		}
	}
	else
	{
		// Add the component (reuse existing logic)
		return OnAddRemoteControlsComponent();
	}
	
	return FReply::Handled();
}

void SFeatureBuilderWidget::CheckForExpiredDeveloperKey(FString developerKey)
{
	FString C3DSettingsPath = FCognitiveEditorTools::GetInstance()->GetSettingsFilePath();
	GConfig->Flush(true, C3DSettingsPath);
	auto Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindRaw(this, &SFeatureBuilderWidget::OnDeveloperKeyResponseReceived);
	FString gateway = FAnalytics::Get().GetConfigValueFromIni(C3DSettingsPath, "/Script/Cognitive3D.Cognitive3DSettings", "Gateway", false);
	FString url = "https://" + gateway + "/v0/apiKeys/verify";
	Request->SetURL(url);
	Request->SetVerb("GET");
	Request->SetHeader(TEXT("Authorization"), "APIKEY:DEVELOPER " + developerKey);
	Request->ProcessRequest();
}

void SFeatureBuilderWidget::OnDeveloperKeyResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (Response.IsValid() == false)
	{
		GLog->Log("SFeatureBuilderWidget::OnDeveloperKeyResponseReceived invalid response");
		return;
	}

	int32 responseCode = Response->GetResponseCode();
	if (responseCode == 200)
	{
		bIsDeveloperKeyValid = true;
		GLog->Log("Feature Builder: Developer Key Response Code is 200");
	}
	else
	{
		bIsDeveloperKeyValid = false;
		GLog->Log("Feature Builder: Developer Key Response Code is " + FString::FromInt(responseCode) + ". Developer key may be invalid or expired");
	}
}

FText SFeatureBuilderWidget::GetSelectedFeatureDisplayName() const
{
	for (const FFeature& Feature : FeatureList)
	{
		if (Feature.Key == SelectedFeature)
		{
			return Feature.DisplayName;
		}
	}
	return FText::FromString("Unknown Feature");
}

FReply SFeatureBuilderWidget::OnBackClicked()
{
	USegmentAnalytics::Get()->TrackEvent(TEXT("BackButton_Clicked"), TEXT("FeatureBuilderWindow"));
	CurrentMode = EPageMode::List;
	return FReply::Handled();
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION
