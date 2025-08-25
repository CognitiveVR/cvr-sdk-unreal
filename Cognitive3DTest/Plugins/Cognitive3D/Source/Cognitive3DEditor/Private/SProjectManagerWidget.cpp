// Fill out your copyright notice in the Description page of Project Settings.


#include "SProjectManagerWidget.h"
#include "SlateOptMacros.h"
#include "CognitiveEditorTools.h"
#include "IPluginManager.h"
#include "Analytics.h"
#if ENGINE_MAJOR_VERSION == 5
#include "SDockTab.h"
#elif ENGINE_MAJOR_VERSION == 4
#include "SDockableTab.h"
#endif
#include "SExpandableArea.h"
#include "STextBlock.h"
#include "SScrollBox.h"
#include "SBox.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Images/SImage.h"
#include "CognitiveEditorStyle.h"
#include "Cognitive3D/Public/DynamicObject.h"
#include "SegmentAnalytics.h"

#define LOCTEXT_NAMESPACE "BaseToolEditor"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SProjectManagerWidget::Construct(const FArguments& InArgs)
{
	FCognitiveEditorTools::CheckIniConfigured();
	DisplayAPIKey = FCognitiveEditorTools::GetInstance()->GetApplicationKey().ToString();
	DisplayDeveloperKey = FCognitiveEditorTools::GetInstance()->GetDeveloperKey().ToString();
	DisplayExportDirectory = FCognitiveEditorTools::GetInstance()->GetBaseExportDirectory();

	// Initialize each SDK’s state by checking build.cs
	SDKCheckboxStates.Add(TEXT("MetaXR"),
		IsSDKEnabledInBuildCs(TEXT("MetaXRPlugin")));
	SDKCheckboxStates.Add(TEXT("MetaXRPlatform"),
		IsSDKEnabledInBuildCs(TEXT("MetaXRPlatform")));
	SDKCheckboxStates.Add(TEXT("PicoXR"),
		IsSDKEnabledInBuildCs(TEXT("PicoXR")));       
	SDKCheckboxStates.Add(TEXT("WaveVR"),
		IsSDKEnabledInBuildCs(TEXT("WaveVR")));       

	CollectAllMaps();

	// Start with whatever the server / config already gave you:
	SceneListItems = FCognitiveEditorTools::GetInstance()->SceneData;

	// Build a quick lookup of already-exported package paths 
	TSet<FString> ExportedPackagePaths;
	for (auto& Entry : SceneListItems)
	{
		// Entry->Path is "/Game/Maps" and Entry->Name is "VRMap"
		ExportedPackagePaths.Add(Entry->Path + "/" + Entry->Name);
	}

	// Add every map in LevelSelectionMap whose package path isn't yet exported
	TSet<FString> SeenPackages;
	for (auto& Pair : LevelSelectionMap)
	{
		const FString& FullObjectPath = Pair.Key;
		// eg "/Game/Maps/VRMap.VRMap"
		FString PackagePath, AssetName;
		if (!FullObjectPath.Split(
			TEXT("."),
			&PackagePath,    // "/Game/Maps/VRMap"
			&AssetName,      // "VRMap"
			ESearchCase::IgnoreCase,
			ESearchDir::FromStart))
		{
			// shouldn't ever happen, but fallback
			PackagePath = FullObjectPath;
			AssetName = FPackageName::GetShortName(FullObjectPath);
		}

		//remove the map name from PackagePath
		TArray<FString> PathParts;
		PackagePath.ParseIntoArray(PathParts, TEXT("/"), true);
		//keep all parts except the last one
		if (PathParts.Num() > 1)
		{
			PathParts.RemoveAt(PathParts.Num() - 1, 1); // remove last part
			PackagePath = FString::Join(PathParts, TEXT("/")); // rejoin
			PackagePath = FString(TEXT("/")) + PackagePath; // ensure it starts with "/"
		}


		// skip if already exported or already queued
		if (ExportedPackagePaths.Contains(PackagePath + "/" + AssetName) ||
			SeenPackages.Contains(PackagePath + "/" + AssetName))
		{
			continue;
		}

		SeenPackages.Add(PackagePath + "/" + AssetName);

		// names and paths are package-based now
		SceneListItems.Add(MakeShared<FEditorSceneData>(
			AssetName,     // eg "VRMap"
			PackagePath,   // eg "/Game/Maps"
			TEXT(""),      // Id
			0,             // VersionNumber
			0              // VersionId
		));
	}

	//  Sort by Name then Path
	SceneListItems.Sort([](auto& A, auto& B) {
		int32 C = A->Name.Compare(B->Name, ESearchCase::IgnoreCase);
		return (C != 0) ? (C < 0)
			: (A->Path.Compare(B->Path, ESearchCase::IgnoreCase) < 0);
		});
	
	//segment analytics
	USegmentAnalytics::Get()->TrackEvent(TEXT("FullProjectSetupWindow_Opened"), TEXT("FullProjectSetupWindow"));

    ChildSlot  
    [  
		SNew(SVerticalBox)
		//scrollable region
		+SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
			//Make the entire page scrollable
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			[
				//Vertical stack banner text then  content
				SNew(SVerticalBox)

					// Banner Image at the very top
					+SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(0, 0, 0, 10)
					[
						SNew(SImage)
							.Image(FCognitiveEditorStyle::GetBrush(TEXT("CognitiveEditor.Banner")))
					]

					
					//Introductory Text under the banner
					+SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(0, 0, 0, 20)
					[
						SNew(STextBlock)
							.AutoWrapText(true)
							.Text(FText::FromString(
								TEXT("Welcome to the Cognitive3D setup. Use the panels below to configure authentication, "
									"select your third-party SDKs, and prepare your scenes for export.")))
					]

					
					+SVerticalBox::Slot()
					.AutoHeight()
					[

						//header
						SNew(SOverlay)
						+ SOverlay::Slot()
						.VAlign(VAlign_Top)
						.HAlign(HAlign_Left)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							[
								SNew(SBox)
								//.WidthOverride(128)
								.HeightOverride(32)
								[
									SNew(SRichTextBlock)
									.Justification(ETextJustify::Left)
									.DecoratorStyleSet(&FEditorStyle::Get())
									.Text(FText::FromString("Cognitive3D Full Project Setup"))
								]
							]
						]
							//body
						+ SOverlay::Slot()
						[
							SNew(SVerticalBox)

							+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0,0,0,5)
								[
									SNew(SExpandableArea)
										.InitiallyCollapsed(true)
										.HeaderContent()
										[
											SNew(STextBlock)
												.Text(FText::FromString(TEXT("Authentication")))
												.Font(FEditorStyle::GetFontStyle("DetailsView.CategoryFontStyle"))
										]
										.BodyContent()
										[
											SNew(SVerticalBox)

												+ SVerticalBox::Slot()
												.AutoHeight()
												.Padding(0, 0, 0, 5)
												[
													SNew(SRichTextBlock)
														.Visibility(EVisibility::Visible)
														.AutoWrapText(true)
														.Justification(ETextJustify::Center)
														.DecoratorStyleSet(&FEditorStyle::Get())
														.Text(FText::FromString("Please add your <RichTextBlock.BoldHighlight>Cognitive3D Project API Keys</> below to continue\nThese are available on the Project Dashboard"))
												]
												//dev key
												+ SVerticalBox::Slot()
												.MaxHeight(32)
												.AutoHeight()
												.Padding(0, 0, 0, 5)
												[
													SNew(SHorizontalBox)
														.Visibility(EVisibility::Visible)
														+ SHorizontalBox::Slot()
														[
															SNew(SBox)
																.Visibility(EVisibility::Visible)
																.WidthOverride(128)
																.HeightOverride(32)
																[
																	SNew(STextBlock)
																		.Visibility(EVisibility::Visible)
																		.Text(FText::FromString("Developer Key\nThis key is secret and should be kept safe, make sure not to commit to version control"))
																]
														]
														+ SHorizontalBox::Slot()
														[
															SNew(SBox)
																.Visibility(EVisibility::Visible)
																.WidthOverride(128)
																.HeightOverride(32)
																[
																	SNew(SEditableTextBox)
																		.Visibility(EVisibility::Visible)
																		.Text(this, &SProjectManagerWidget::GetDisplayDeveloperKey)
																		.OnTextChanged(this, &SProjectManagerWidget::OnDeveloperKeyChanged)
																		//.OnTextCommitted()
																]
														]
												]
												+ SVerticalBox::Slot()
												.AutoHeight()
												.Padding(0, 0, 0, 5)
												[
													SNew(SSeparator)
														.Visibility(EVisibility::Visible)
												]

												//app key
												+ SVerticalBox::Slot()
												.AutoHeight()
												.Padding(0, 0, 0, 5)
												[
													SAssignNew(OrgNameTextBlock, STextBlock)
														.Visibility(EVisibility::Visible)
														.Justification(ETextJustify::Center)
														.Text(FText::FromString("Unknown Organization Name"))
												]

												+ SVerticalBox::Slot()
												.AutoHeight()
												.Padding(0, 0, 0, 5)
												[
													SAssignNew(OrgTrialTextBlock, STextBlock)
														.Visibility(EVisibility::Visible)
														.Justification(ETextJustify::Center)
														.Text(FText::FromString("Unknown Organization Trial Status"))
												]

												+ SVerticalBox::Slot()
												.AutoHeight()
												.Padding(0, 0, 0, 5)
												[
													SAssignNew(OrgExpiryTextBlock, STextBlock)
														.Visibility(EVisibility::Visible)
														.Justification(ETextJustify::Center)
														.Text(FText::FromString("Unknown Organization Expirey Date"))
												]

												+ SVerticalBox::Slot()
												.MaxHeight(32)
												.AutoHeight()
												.Padding(0, 0, 0, 5)
												[
													SNew(SHorizontalBox)
														.Visibility(EVisibility::Visible)
														+ SHorizontalBox::Slot()
														[
															SNew(SBox)
																.Visibility(EVisibility::Visible)
																.WidthOverride(128)
																.HeightOverride(32)
																[
																	SNew(STextBlock)
																		.Visibility(EVisibility::Visible)
																		.Text(FText::FromString("Application Key\nThis key identifies and authorizes the session data being uploaded to your project"))
																]
														]

														+ SHorizontalBox::Slot()
														[
															SNew(SBox)
																.Visibility(EVisibility::Visible)
																.WidthOverride(128)
																.HeightOverride(32)
																[
																	SNew(SEditableTextBox)
																		.Visibility(EVisibility::Visible)
																		.Text(this, &SProjectManagerWidget::GetDisplayAPIKey)
																		.OnTextChanged(this, &SProjectManagerWidget::OnAPIKeyChanged)
																]
														]
												]

												//validate keys button
												+ SVerticalBox::Slot()
												.AutoHeight()
												.Padding(0, 0, 0, 5)
												[
													SNew(SBox)
														.HAlign(HAlign_Center)
														.VAlign(VAlign_Center)
														//set width/height override to match the image size
														//.HeightOverride(150)
														.WidthOverride(256)
														.HeightOverride(32)
														.Visibility(EVisibility::Visible)
														[
															SNew(SButton)
																//.ContentPadding(0)
																.Text(FText::FromString("Validate Developer Key and Fetch Application Key"))
																.OnClicked(this, &SProjectManagerWidget::ValidateKeys)
														]
												]
								

										]
								]

									+ SVerticalBox::Slot()
									.AutoHeight()
									.Padding(0, 0, 0, 5)
									[
										SNew(SSeparator)
											.Visibility(EVisibility::Visible)
									]


										+ SVerticalBox::Slot()
										.AutoHeight()
										.Padding(0, 0, 0, 5)
										[
											SNew(SExpandableArea)
												.InitiallyCollapsed(true)
												.HeaderContent()
												[
													SNew(STextBlock)
														.Text(FText::FromString(TEXT("Player Setup")))
														.Font(FEditorStyle::GetFontStyle("DetailsView.CategoryFontStyle"))
												]
												.BodyContent()
												[
													SNew(SVerticalBox)

														+ SVerticalBox::Slot()
														.AutoHeight()
														.Padding(0, 0, 0, 5)
														[
															SNew(SHorizontalBox)
																.Visibility(EVisibility::Visible)
																+ SHorizontalBox::Slot()
																[
																	SNew(SRichTextBlock)
																		.Visibility(EVisibility::Visible)
																		.AutoWrapText(true)
																		.Justification(ETextJustify::Center)
																		.DecoratorStyleSet(&FCognitiveEditorTools::GetSlateStyle())
																		.Text(FText::FromString("The Cognitive3DActor Blueprint automatically finds your player's controllers in run-time and assigns dynamic objects to them to be tracked for valuable insights on our dashboard.\n\n You do not have to set those up manually, or export and upload a mesh for them to be tracked like other dynamic objects."))
																]
														]
														+SVerticalBox::Slot()
														.AutoHeight()
														.Padding(0, 0, 0, 5)
														[
															SNew(SSeparator)
																.Visibility(EVisibility::Visible)
														]

														+ SVerticalBox::Slot()
														.AutoHeight()
														.Padding(0, 0, 0, 5)
														.HAlign(EHorizontalAlignment::HAlign_Center)
														[
															SNew(SRichTextBlock)
																.Visibility(EVisibility::Visible)
																.AutoWrapText(true)
																.Justification(ETextJustify::Center)
																.DecoratorStyleSet(&FCognitiveEditorTools::GetSlateStyle())
																.Text(this, &SProjectManagerWidget::GetInputClassText)
														]

														+ SVerticalBox::Slot()
														.AutoHeight()
														.Padding(0, 0, 0, 5)
														.HAlign(EHorizontalAlignment::HAlign_Center)
														[
															SNew(SBox)
																.Visibility(this, &SProjectManagerWidget::GetDefaultInputClassEnhanced)
																[
																	SNew(SRichTextBlock)
																		.Visibility(this, &SProjectManagerWidget::GetAppendedInputsFoundVisibility)
																		.AutoWrapText(true)
																		.Justification(ETextJustify::Center)
																		.DecoratorStyleSet(&FCognitiveEditorTools::GetSlateStyle())
																		.Text(FText::FromString("The Cognitive3D action maps have been added to DefaultInputs.ini"))
																]
														]

														+ SVerticalBox::Slot()
														.AutoHeight()
														.Padding(0, 0, 0, 5)
														.HAlign(EHorizontalAlignment::HAlign_Center)
														[
															SNew(SBox)
																.Visibility(this, &SProjectManagerWidget::GetDefaultInputClassEnhanced)
																[
																	SNew(SHorizontalBox)
																		+ SHorizontalBox::Slot()
																		.AutoWidth()
																		[
																			SNew(SBox)
																				.Visibility(EVisibility::Visible)
																				.WidthOverride(256)
																				.HeightOverride(32)
																				[
																					SNew(SButton)
																						.Text(FText::FromString("Append Input Data to Input.ini"))
																						.OnClicked(this, &SProjectManagerWidget::AppendInputs)
																				]
																		]
																]
														]

												]
										]
											+ SVerticalBox::Slot()
											.AutoHeight()
											.Padding(0, 0, 0, 5)
											[
												SNew(SSeparator)
													.Visibility(EVisibility::Visible)
											]
											+ SVerticalBox::Slot()
											.AutoHeight()
											.Padding(0, 0, 0, 5)
											[
												SNew(SExpandableArea)
													.InitiallyCollapsed(true)
													.HeaderContent()
													[
														SNew(STextBlock)
															.Text(FText::FromString(TEXT("Scene Setup")))
															.Font(FEditorStyle::GetFontStyle("DetailsView.CategoryFontStyle"))
													]
													.BodyContent()
													[
														SNew(SVerticalBox)

															//export directory
															+ SVerticalBox::Slot()
															.AutoHeight()
															.Padding(0, 0, 0, 5)
															[
																SNew(STextBlock)
																	.Visibility(EVisibility::Visible)
																	.Justification(ETextJustify::Center)
																	.AutoWrapText(true)
																	.Text(FText::FromString("When uploading your level to the dashboard, we use Unreal's GLTF exporter to automatically prepare the scene.\nThis includes exporting images as .pngs\n\nWe also need a temporary Export Directory to save Unreal files to while we process them."))
															]



															+SVerticalBox::Slot()
															.VAlign(VAlign_Center)
															.AutoHeight()
															.Padding(0, 0, 0, 5)
															[
																SNew(STextBlock)
																	.Visibility(EVisibility::Visible)
																	.Justification(ETextJustify::Center)
																	.Text(FText::FromString("You will need to create a temporary directory to store the exported files."))
															]

															+ SVerticalBox::Slot()
															.AutoHeight()
															.Padding(0, 0, 0, 5)
															[
																SNew(SSeparator)
																	.Visibility(EVisibility::Visible)
															]

															//path to export directory
															+ SVerticalBox::Slot()
															.MaxHeight(32)
															.AutoHeight()
															.Padding(0, 0, 0, 5)
															[
																SNew(SHorizontalBox)
																	.Visibility(EVisibility::Visible)
																	+ SHorizontalBox::Slot()
																	.MaxWidth(200)
																	[
																		SNew(SBox)
																			.Visibility(EVisibility::Visible)
																			.HeightOverride(32)
																			[
																				SNew(STextBlock)
																					.Visibility(EVisibility::Visible)
																					.IsEnabled_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::HasDeveloperKey)
																					.Text(FText::FromString("Path to Export Directory"))
																			]
																	]
																	+ SHorizontalBox::Slot()
																	.FillWidth(3)
																	.Padding(1)
																	[
																		SNew(SBox)
																			.Visibility(EVisibility::Visible)
																			.HeightOverride(32)
																			.MaxDesiredHeight(32)
																			[
																				SNew(SEditableTextBox)
																					.Visibility(EVisibility::Visible)
																					.Text_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::GetBaseExportDirectoryDisplay)
																					.OnTextChanged(this, &SProjectManagerWidget::OnExportPathChanged)
																					//SNew(STextBlock)
																					//
																			]
																	]
																	+ SHorizontalBox::Slot()
																	.MaxWidth(17)
																	[
																		SNew(SHorizontalBox)
																			+ SHorizontalBox::Slot()
																			.AutoWidth()
																			.Padding(FMargin(4.0f, 0.0f, 0.0f, 0.0f))
																			.VAlign(VAlign_Center)
																			[
																				SNew(SBox)
																					.Visibility(EVisibility::Visible)
																					.HeightOverride(17)
																					.WidthOverride(17)
																					[
																						SNew(SButton)
																							.Visibility(EVisibility::Visible)
																							//PickerWidget = SAssignNew(BrowseButton, SButton)
																							.ButtonStyle(FEditorStyle::Get(), "HoverHintOnly")
																							.ToolTipText(LOCTEXT("FolderButtonToolTipText", "Choose a directory from this computer"))
																							.OnClicked_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::SelectBaseExportDirectory)
																							.ContentPadding(2.0f)
																							.ForegroundColor(FSlateColor::UseForeground())
																							.IsFocusable(false)
																							[
																								SNew(SImage)
																									.Image(FEditorStyle::GetBrush("PropertyWindow.Button_Ellipsis"))
																									.ColorAndOpacity(FSlateColor::UseForeground())
																							]
																					]
																			]
																	]
																	+ SHorizontalBox::Slot()
																	.MaxWidth(17)
																	[
																		SNew(SHorizontalBox)
																			+ SHorizontalBox::Slot()
																			.AutoWidth()
																			.Padding(FMargin(4.0f, 0.0f, 0.0f, 0.0f))
																			.VAlign(VAlign_Center)
																			[
																				SNew(SBox)
																					.Visibility(EVisibility::Visible)
																					.HeightOverride(17)
																					.WidthOverride(17)
																					[
																						SNew(SImage)
																							.Visibility(EVisibility::Visible)
																							.Image(this, &SProjectManagerWidget::GetExportPathStateIcon)
																							.ToolTipText(this, &SProjectManagerWidget::GetExportPathTooltipText)
																							.ColorAndOpacity(FSlateColor::UseForeground())
																					]
																			]
																	]
															]

															//texture compression checkbox

															//compress Textures toggle
																+SVerticalBox::Slot()
																.AutoHeight()
																.Padding(0, 5)
																[
																	SNew(SCheckBox)
																		.IsChecked(this, &SProjectManagerWidget::IsCompressTexturesChecked)
																		.OnCheckStateChanged(this, &SProjectManagerWidget::OnCompressTexturesChanged)
																		[
																			SNew(STextBlock)
																				.Text(FText::FromString(TEXT("Compress Textures After Export")))
																				.ToolTipText(FText::FromString(
																					TEXT("When checked, all exported textures will be run through a post-export compressor.")))
																		]
																]

																//export dynamics with scene toggle

																+SVerticalBox::Slot()
																	.AutoHeight()
																	.Padding(0, 5)
																	[
																		SNew(SCheckBox)
																			.IsChecked(this, &SProjectManagerWidget::IsExportDynamicsChecked)
																			.OnCheckStateChanged(this, &SProjectManagerWidget::OnExportDynamicsChanged)
																			[
																				SNew(STextBlock)
																					.Text(FText::FromString(TEXT("Export and Upload Dynamics with Scene")))
																					.ToolTipText(FText::FromString(
																						TEXT("When checked, all dynamic objects will be exported and uploaded with the respective scene.")))
																			]
																	]

															//scene list
															+SVerticalBox::Slot()
															.AutoHeight()
															.Padding(0, 0, 0, 10)
															[
																SNew(SBox)
																	.MaxDesiredHeight(200)
																	[
																		SAssignNew(SceneListView,
																			SListView< TSharedPtr<FEditorSceneData> >)
																			.ItemHeight(24)
																			.ListItemsSource(&SceneListItems)
																			.OnGenerateRow(this, &SProjectManagerWidget::OnGenerateSceneRow)
																			.HeaderRow(
																				SNew(SHeaderRow)

																				+ SHeaderRow::Column("Select")
																				.FixedWidth(30)
																				.DefaultLabel(FText::GetEmpty())

																				+ SHeaderRow::Column("Name")
																				.FillWidth(1.0f)
																				.DefaultLabel(FText::FromString("Name"))

																				+ SHeaderRow::Column("Path")
																				.FillWidth(2.0f)
																				.DefaultLabel(FText::FromString("Path"))

																				+ SHeaderRow::Column("Id")
																				.FillWidth(1.0f)
																				.DefaultLabel(FText::FromString("Id"))

																				+ SHeaderRow::Column("Ver#")
																				.FillWidth(0.3f)
																				.DefaultLabel(FText::FromString("Version"))

																				+ SHeaderRow::Column("VerId")
																				.FillWidth(0.3f)
																				.DefaultLabel(FText::FromString("Ver Id"))
																			)
																	]
															]
													]
											]

											+ SVerticalBox::Slot()
												.AutoHeight()
												.Padding(0, 0, 0, 5)
												[
													SNew(SSeparator)
														.Visibility(EVisibility::Visible)
												]

											// ===== New Third-Party SDKs Section =====
											+ SVerticalBox::Slot()
												.AutoHeight()
												.Padding(0, 0, 0, 5)
												[
													SNew(SExpandableArea)
														.InitiallyCollapsed(true)
														.HeaderContent()
														[
															SNew(STextBlock)
																.Text(FText::FromString(TEXT("Third-Party SDKs")))
																.Font(FEditorStyle::GetFontStyle("DetailsView.CategoryFontStyle"))
														]
														.BodyContent()
														[
															SNew(SVerticalBox)

																// MetaXR
																+ SVerticalBox::Slot()
																.AutoHeight()
																.Padding(5, 2)
																[
																		SNew(SCheckBox)
																		.IsChecked_Lambda([this]() {
																		return SDKCheckboxStates.FindRef(TEXT("MetaXR"))
																			? ECheckBoxState::Checked
																			: ECheckBoxState::Unchecked;
																			})
																			.OnCheckStateChanged_Lambda([this](ECheckBoxState NewState) {
																			const bool bEnabled = (NewState == ECheckBoxState::Checked);
																			SDKCheckboxStates.Add(TEXT("MetaXR"), bEnabled);
																			this->ApplySDKToggle(TEXT("MetaXR"), bEnabled);
																				})
																		[
																			SNew(STextBlock)
																				.Text(FText::FromString(TEXT("MetaXR")))
																		]
														
																]

																// MetaXRPlatform (only enabled if MetaXR is checked)
																+ SVerticalBox::Slot()
																.AutoHeight()
																.Padding(20, 2)
																[
																	SNew(SCheckBox)
																		.IsEnabled_Lambda([this]() {
																		return SDKCheckboxStates.FindRef(TEXT("MetaXR"));
																			})
																		.IsChecked_Lambda([this]() {
																		return SDKCheckboxStates.FindRef(TEXT("MetaXRPlatform"))
																			? ECheckBoxState::Checked
																			: ECheckBoxState::Unchecked;
																			})
																		.OnCheckStateChanged_Lambda([this](ECheckBoxState NewState) {
																		const bool bEnabled = (NewState == ECheckBoxState::Checked);
																		SDKCheckboxStates.Add(TEXT("MetaXRPlatform"), bEnabled);
																		this->ApplySDKToggle(TEXT("MetaXRPlatform"), bEnabled);
																			})
																		[
																			SNew(STextBlock)
																				.Text(FText::FromString(TEXT("MetaXRPlatform")))
																		]
																]

																// PicoXR
																+ SVerticalBox::Slot()
																.AutoHeight()
																.Padding(5, 2)
																[
																	SNew(SCheckBox)
																		.IsChecked_Lambda([this]() {
																		return SDKCheckboxStates.FindRef(TEXT("PicoXR"))
																			? ECheckBoxState::Checked
																			: ECheckBoxState::Unchecked;
																			})
																		.OnCheckStateChanged_Lambda([this](ECheckBoxState NewState) {
																		const bool bEnabled = (NewState == ECheckBoxState::Checked);
																		SDKCheckboxStates.Add(TEXT("PicoXR"), bEnabled);
																		this->ApplySDKToggle(TEXT("PicoXR"), bEnabled);
																			})
																		[
																			SNew(STextBlock)
																				.Text(FText::FromString(TEXT("PicoXR")))
																		]
																]

																// WaveVR
																+ SVerticalBox::Slot()
																.AutoHeight()
																.Padding(5, 2)
																[
																	SNew(SCheckBox)
																		.IsChecked_Lambda([this]() {
																		return SDKCheckboxStates.FindRef(TEXT("WaveVR"))
																			? ECheckBoxState::Checked
																			: ECheckBoxState::Unchecked;
																			})
																		.OnCheckStateChanged_Lambda([this](ECheckBoxState NewState) {
																		const bool bEnabled = (NewState == ECheckBoxState::Checked);
																		SDKCheckboxStates.Add(TEXT("WaveVR"), bEnabled);
																		this->ApplySDKToggle(TEXT("WaveVR"), bEnabled);
																			})
																		[
																			SNew(STextBlock)
																				.Text(FText::FromString(TEXT("WaveVR")))
																		]
																]
														]
												]
						] // end of the overlay slot
				] // end of vertical box
			] // end of scroll box slot
		] // end of vertical box slot
		//finish setup button
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(0, 10)
			[
				SNew(SBox)
					.WidthOverride(200)
					.HeightOverride(32)
					[
						SNew(SButton)
							.Text(FText::FromString(TEXT("Finish Setup")))
							.OnClicked_Lambda([this]() -> FReply {
							this->FinalizeProjectSetup();
							return FReply::Handled();
								})
					]
			]
	]; // end of child slot
	
}
FReply SProjectManagerWidget::OpenFullC3DSetupWindow()
{
	FCognitive3DEditorModule::SpawnFullC3DSetup();
	return FReply::Handled();
}

TSharedRef<ITableRow> SProjectManagerWidget::OnGenerateSceneRow(TSharedPtr<FEditorSceneData> Item, const TSharedRef<STableViewBase>& OwnerTable)
{

	const FString& FullPath = Item->Path + "/" + Item->Name;

	return SNew(STableRow< TSharedPtr<FEditorSceneData> >, OwnerTable)
		[
			SNew(SHorizontalBox)

				// Checkbox
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(SCheckBox)
						.IsChecked_Lambda([this, FullPath]() {
						return (LevelSelectionMap.FindRef(FullPath)
							? ECheckBoxState::Checked
							: ECheckBoxState::Unchecked);
							})
						.OnCheckStateChanged_Lambda([this, FullPath](ECheckBoxState State) {
						LevelSelectionMap.Add(FullPath, State == ECheckBoxState::Checked);
							})
				]

				// Name column
				+ SHorizontalBox::Slot()
				.FillWidth(1)
				.Padding(5, 0)
				[
					SNew(STextBlock)
						.Text(FText::FromString(Item->Name))
				]

				// (you can add a small Path column if you like:)
				+ SHorizontalBox::Slot()
				.FillWidth(1)
				.Padding(5, 0)
				[
					SNew(STextBlock)
						.Text(FText::FromString(Item->Path))
						.Font(FEditorStyle::Get().GetFontStyle("MonoFont"))
				]

				// Id, VersionNumber, VersionId...
				+ SHorizontalBox::Slot().FillWidth(0.3f).Padding(5, 0)
				[
					SNew(STextBlock)
						.Text(FText::FromString(Item->Id))
				]
				+ SHorizontalBox::Slot().FillWidth(0.3f).Padding(5, 0)
				[
					SNew(STextBlock)
						.Text(FText::AsNumber(Item->VersionNumber))
				]
				+ SHorizontalBox::Slot().FillWidth(0.3f).Padding(5, 0)
				[
					SNew(STextBlock)
						.Text(FText::AsNumber(Item->VersionId))
				]
		];
}

FReply SProjectManagerWidget::ValidateKeys()
{
	CheckForExpiredDeveloperKey(DisplayDeveloperKey);

	FCognitiveEditorTools::GetInstance()->SaveDeveloperKeyToFile(DisplayDeveloperKey);
	FCognitiveEditorTools::GetInstance()->CurrentSceneVersionRequest();
	FetchApplicationKey(DisplayDeveloperKey);
	FetchOrganizationDetails(DisplayDeveloperKey);

	return FReply::Handled();
}

void SProjectManagerWidget::CheckForExpiredDeveloperKey(FString developerKey)
{
	FString C3DSettingsPath = FCognitiveEditorTools::GetInstance()->GetSettingsFilePath();
	GConfig->Flush(true, C3DSettingsPath);
	auto Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindRaw(this, &SProjectManagerWidget::OnDeveloperKeyResponseReceived);
	FString gateway = FAnalytics::Get().GetConfigValueFromIni(C3DSettingsPath, "/Script/Cognitive3D.Cognitive3DSettings", "Gateway", false);
	FString url = "https://" + gateway + "/v0/apiKeys/verify";
	Request->SetURL(url);
	Request->SetVerb("GET");
	Request->SetHeader(TEXT("Authorization"), "APIKEY:DEVELOPER " + developerKey);
	Request->ProcessRequest();
}

void SProjectManagerWidget::OnDeveloperKeyResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (Response.IsValid() == false)
	{
		GLog->Log("SProjectSetupWidget::OnDeveloperKeyResponseReceived invalid response");
		return;
	}

	int32 responseCode = Response->GetResponseCode();
	if (responseCode == 200)
	{
		GLog->Log("Developer Key Response Code is 200");
		//CurrentPageEnum = EProjectSetupPage::OrganizationDetails;
	}
	else
	{
		SGenericDialogWidget::OpenDialog(FText::FromString("Your developer key has expired"), SNew(STextBlock).Text(FText::FromString("Please log in to the dashboard, select your project, and generate a new developer key.\n\nNote:\nDeveloper keys allow you to upload and modify Scenes, and the keys expire after 90 days.\nApplication keys authorize your app to send data to our server, and they never expire.")));
		GLog->Log("Developer Key Response Code is " + FString::FromInt(responseCode) + ". Developer key may be invalid or expired");
	}
}

void SProjectManagerWidget::FetchApplicationKey(FString developerKey)
{
	auto HttpRequest = FHttpModule::Get().CreateRequest();

	FString C3DSettingsPath = FCognitiveEditorTools::GetInstance()->GetSettingsFilePath();
	GConfig->LoadFile(C3DSettingsPath);
	FString Gateway = FAnalytics::Get().GetConfigValueFromIni(C3DSettingsPath, "/Script/Cognitive3D.Cognitive3DSettings", "Gateway", false);

	FString url = FString("https://" + Gateway + "/v0/applicationKey");
	HttpRequest->SetURL(url);
	HttpRequest->SetVerb("GET");
	FString AuthValue = "APIKEY:DEVELOPER " + FCognitiveEditorTools::GetInstance()->DeveloperKey;
	HttpRequest->SetHeader("Authorization", AuthValue);
	HttpRequest->SetHeader("Content-Type", "application/json");
	HttpRequest->OnProcessRequestComplete().BindRaw(this, &SProjectManagerWidget::GetApplicationKeyResponse);
	HttpRequest->ProcessRequest();
}

void SProjectManagerWidget::GetApplicationKeyResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (Response.IsValid() == false)
	{
		GLog->Log("SProjectSetupWidget::GetApplicationKeyResponse invalid response");
		return;
	}
	int32 responseCode = Response->GetResponseCode();
	if (responseCode != 200)
	{
		GLog->Log("Application Key Response Code is " + FString::FromInt(responseCode));
		return;
	}

	FSuppressableWarningDialog::FSetupInfo Info(LOCTEXT("UpdateApplicationKeyBody", "Found Application Key on the Dashboard, we recommend using this key."), LOCTEXT("UpdateApplicationKeyTitle", "Found Application Key"), "Cognitive3dApplicationKey");
	Info.ConfirmText = LOCTEXT("Yes", "Ok");
	//Info.CancelText = LOCTEXT("No", "No");
	Info.CheckBoxText = FText();
	FSuppressableWarningDialog WarnAboutCoordinatesSystem(Info);
	FSuppressableWarningDialog::EResult result = WarnAboutCoordinatesSystem.ShowModal();

	if (result == FSuppressableWarningDialog::EResult::Confirm)
	{
		auto content = Response->GetContentAsString();
		FApplicationKeyResponse responseObject;
		FJsonObjectConverter::JsonObjectStringToUStruct(content, &responseObject, 0, 0);

		FCognitiveEditorTools::GetInstance()->SaveApplicationKeyToFile(responseObject.apikey);
		DisplayAPIKey = responseObject.apikey;
	}
}

void SProjectManagerWidget::FetchOrganizationDetails(FString developerKey)
{
	auto HttpRequest = FHttpModule::Get().CreateRequest();

	FString C3DSettingsPath = FCognitiveEditorTools::GetInstance()->GetSettingsFilePath();
	GConfig->LoadFile(C3DSettingsPath);
	FString Gateway = FAnalytics::Get().GetConfigValueFromIni(C3DSettingsPath, "/Script/Cognitive3D.Cognitive3DSettings", "Gateway", false);

	FString url = FString("https://" + Gateway + "/v0/subscriptions");
	HttpRequest->SetURL(url);
	HttpRequest->SetVerb("GET");
	FString AuthValue = "APIKEY:DEVELOPER " + FCognitiveEditorTools::GetInstance()->DeveloperKey;
	HttpRequest->SetHeader("Authorization", AuthValue);
	HttpRequest->SetHeader("Content-Type", "application/json");
	HttpRequest->OnProcessRequestComplete().BindRaw(this, &SProjectManagerWidget::GetOrganizationDetailsResponse);
	HttpRequest->ProcessRequest();
}

void SProjectManagerWidget::GetOrganizationDetailsResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (Response.IsValid() == false)
	{
		GLog->Log("SProjectSetupWidget::GetOrganizationDetailsResponse invalid response");
		return;
	}

	int32 responseCode = Response->GetResponseCode();
	if (responseCode != 200)
	{
		GLog->Log("Organization Details Response Code is " + FString::FromInt(responseCode));
		return;
	}

	auto content = Response->GetContentAsString();
	FOrganizationDataResponse responseObject;
	FJsonObjectConverter::JsonObjectStringToUStruct(content, &responseObject, 0, 0);
	OrgNameTextBlock->SetText(FText::FromString("Organization Name: " + responseObject.organizationName));

	if (responseObject.subscriptions.Num() == 0)
	{
		GLog->Log("No Cognitive3D Subscription");
	}
	else
	{
		FDateTime expiryDate = FDateTime::FromUnixTimestamp(responseObject.subscriptions[0].expiration / 1000);

		FString expiryPrettyDate;
		expiryPrettyDate.Append(FString::FromInt(expiryDate.GetDay()));
		expiryPrettyDate.Append(" ");
		FString MonthStr;
		switch (expiryDate.GetMonthOfYear())
		{
		case EMonthOfYear::January:		MonthStr = TEXT("Jan");	break;
		case EMonthOfYear::February:	MonthStr = TEXT("Feb");	break;
		case EMonthOfYear::March:		MonthStr = TEXT("Mar");	break;
		case EMonthOfYear::April:		MonthStr = TEXT("Apr");	break;
		case EMonthOfYear::May:			MonthStr = TEXT("May");	break;
		case EMonthOfYear::June:		MonthStr = TEXT("Jun");	break;
		case EMonthOfYear::July:		MonthStr = TEXT("Jul");	break;
		case EMonthOfYear::August:		MonthStr = TEXT("Aug");	break;
		case EMonthOfYear::September:	MonthStr = TEXT("Sep");	break;
		case EMonthOfYear::October:		MonthStr = TEXT("Oct");	break;
		case EMonthOfYear::November:	MonthStr = TEXT("Nov");	break;
		case EMonthOfYear::December:	MonthStr = TEXT("Dec");	break;
		}
		expiryPrettyDate.Append(MonthStr);
		expiryPrettyDate.Append(", ");
		expiryPrettyDate.Append(FString::FromInt(expiryDate.GetYear()));

		OrgExpiryTextBlock->SetText(FText::FromString("Expiry Date: " + expiryPrettyDate));
		OrgTrialTextBlock->SetText(responseObject.subscriptions[0].isFreeTrial ? FText::FromString("Is Trial: True") : FText::FromString("Is Trial: False"));
	}
}

FText SProjectManagerWidget::GetDisplayAPIKey() const
{
	return FText::FromString(DisplayAPIKey);
}

void SProjectManagerWidget::OnAPIKeyChanged(const FText& Text)
{
	DisplayAPIKey = Text.ToString();
}

FText SProjectManagerWidget::GetDisplayDeveloperKey() const
{
	return FText::FromString(DisplayDeveloperKey);
}

void SProjectManagerWidget::OnDeveloperKeyChanged(const FText& Text)
{
	DisplayDeveloperKey = Text.ToString();
}

const FSlateBrush* SProjectManagerWidget::GetExportPathStateIcon() const
{
	if (FCognitiveEditorTools::GetInstance()->HasSetExportDirectory())
	{
		return FCognitiveEditorTools::GetInstance()->BoxCheckIcon;
	}
	return FCognitiveEditorTools::GetInstance()->BoxEmptyIcon;
}

FText SProjectManagerWidget::GetExportPathTooltipText() const
{
	if (FCognitiveEditorTools::GetInstance()->HasSetExportDirectory())
	{
		return FText::FromString("");
	}
	return FText::FromString("Temporary export directory is not set");
}

void SProjectManagerWidget::OnExportPathChanged(const FText& Text)
{
	DisplayExportDirectory = Text.ToString();
}

FText SProjectManagerWidget::GetInputClassText() const
{
	//get text based on whether or not enhanced input is the default input class
	if (GetDefaultInputClassEnhanced() == EVisibility::Collapsed)
	{
		return FText::FromString("Project is using Enhanced Input, C3D will automatically track controller inputs.\nIf you would like us not to track controller inputs, remove the EnhancedInputTracker component from the Cognitive3D blueprint in the level");
	}
	else if (GetAppendedInputsFoundHidden() == EVisibility::Visible)
	{
		return FText::FromString("You can append inputs to your DefaultInput.ini file. This will allow you to visualize the button presses of the player.");
	}
	else
	{
		return FText::FromString("");
	}
}

FReply SProjectManagerWidget::AppendInputs()
{
	FString InputIni = FPaths::Combine(*(FPaths::ProjectDir()), TEXT("Config/DefaultInput.ini"));


	TArray<FString> actionMapping;
	TArray<FString> axisMapping;

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 2
	const FString NormalizedInputIni = GConfig->NormalizeConfigIniPath(InputIni);
	GConfig->GetArray(TEXT("/Script/Engine.InputSettings"), TEXT("+ActionMappings"), actionMapping, NormalizedInputIni);
	GConfig->GetArray(TEXT("/Script/Engine.InputSettings"), TEXT("+AxisMappings"), axisMapping, NormalizedInputIni);
#else
	GConfig->GetArray(TEXT("/Script/Engine.InputSettings"), TEXT("+ActionMappings"), actionMapping, InputIni);
	GConfig->GetArray(TEXT("/Script/Engine.InputSettings"), TEXT("+AxisMappings"), axisMapping, InputIni);
#endif

	if (actionMapping.Contains("(ActionName=\"C3D_LeftGrip\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=OculusTouch_Left_Grip_Click)"))
	{
		GLog->Log("SSceneSetupWidget::AppendInputs already includes Cognitive3D Inputs");
		//already added! don't append again
		return FReply::Handled();
	}

#if defined INCLUDE_PICOMOBILE_PLUGIN
	actionMapping.Add("(ActionName=\"C3D_LeftTrigger\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=PicoNeoController_L_TriggerAxis)");
	actionMapping.Add("(ActionName=\"C3D_RightGrip\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=PicoNeoController_R_LGrip)");
	actionMapping.Add("(ActionName=\"C3D_LeftGrip\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=PicoNeoController_L_RGrip)");
	actionMapping.Add("(ActionName=\"C3D_RightTrigger\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=PicoNeoController_R_TriggerAxis)");
	actionMapping.Add("(ActionName=\"C3D_LeftJoystick\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=PicoNeoController_L_RockerC)");
	actionMapping.Add("(ActionName=\"C3D_RightJoystick\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=PicoNeoController_R_RockerC)");
	actionMapping.Add("(ActionName=\"C3D_LeftFaceButtonOne\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=PicoNeoController_L_X)");
	actionMapping.Add("(ActionName=\"C3D_LeftFaceButtonTwo\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=PicoNeoController_L_Y)");
	actionMapping.Add("(ActionName=\"C3D_RightFaceButtonOne\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=PicoNeoController_R_A)");
	actionMapping.Add("(ActionName=\"C3D_RightFaceButtonTwo\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=PicoNeoController_R_B)");
	actionMapping.Add("(ActionName=\"C3D_LeftMenuButton\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=PicoNeoController_L_Home)");
	actionMapping.Add("(ActionName=\"C3D_RightMenuButton\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=PicoNeoController_R_Home)");
	axisMapping.Add("(AxisName=\"C3D_LeftJoystickH\",Scale=1.000000,Key=PicoNeoController_L_RockerX)");
	axisMapping.Add("(AxisName=\"C3D_LeftJoystickV\",Scale=-1.000000,Key=PicoNeoController_L_RockerY)");
	axisMapping.Add("(AxisName=\"C3D_RightJoystickH\",Scale=1.000000,Key=PicoNeoController_R_RockerX)");
	axisMapping.Add("(AxisName=\"C3D_RightJoystickV\",Scale=-1.000000,Key=PicoNeoController_R_RockerY)");
	axisMapping.Add("(AxisName=\"C3D_LeftTriggerAxis\",Scale=1.000000,Key=PicoNeoController_L_TriggerAxis)");
	axisMapping.Add("(AxisName=\"C3D_RightTriggerAxis\",Scale=1.000000,Key=PicoNeoController_R_TriggerAxis)");
#endif

	actionMapping.Add("(ActionName=\"C3D_LeftGrip\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=OculusTouch_Left_Grip_Click)");
	actionMapping.Add("(ActionName=\"C3D_LeftGrip\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=ValveIndex_Left_Grip_Axis)");
	actionMapping.Add("(ActionName=\"C3D_LeftGrip\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=MixedReality_Left_Grip_Click)");
	actionMapping.Add("(ActionName=\"C3D_LeftGrip\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=Vive_Left_Grip_Click)");
	actionMapping.Add("(ActionName=\"C3D_RightGrip\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=OculusTouch_Right_Grip_Click)");
	actionMapping.Add("(ActionName=\"C3D_RightGrip\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=ValveIndex_Right_Grip_Axis)");
	actionMapping.Add("(ActionName=\"C3D_RightGrip\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=MixedReality_Right_Grip_Click)");
	actionMapping.Add("(ActionName=\"C3D_RightGrip\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=Vive_Right_Grip_Click)");
	actionMapping.Add("(ActionName=\"C3D_LeftTrigger\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=OculusTouch_Left_Trigger_Click)");
	actionMapping.Add("(ActionName=\"C3D_LeftTrigger\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=ValveIndex_Left_Trigger_Click)");
	actionMapping.Add("(ActionName=\"C3D_LeftTrigger\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=MixedReality_Left_Trigger_Click)");
	actionMapping.Add("(ActionName=\"C3D_LeftTrigger\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=Vive_Left_Trigger_Click)");
	actionMapping.Add("(ActionName=\"C3D_RightTrigger\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=OculusTouch_Right_Trigger_Click)");
	actionMapping.Add("(ActionName=\"C3D_RightTrigger\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=ValveIndex_Right_Trigger_Click)");
	actionMapping.Add("(ActionName=\"C3D_RightTrigger\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=MixedReality_Right_Trigger_Click)");
	actionMapping.Add("(ActionName=\"C3D_RightTrigger\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=Vive_Right_Trigger_Click)");
	actionMapping.Add("(ActionName=\"C3D_LeftTouchpadTouch\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=MixedReality_Left_Trackpad_Touch)");
	actionMapping.Add("(ActionName=\"C3D_LeftTouchpadTouch\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=ValveIndex_Left_Trackpad_Touch)");
	actionMapping.Add("(ActionName=\"C3D_LeftTouchpadTouch\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=Vive_Left_Trackpad_Touch)");
	actionMapping.Add("(ActionName=\"C3D_RightTouchpadTouch\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=MixedReality_Right_Trackpad_Touch)");
	actionMapping.Add("(ActionName=\"C3D_RightTouchpadTouch\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=ValveIndex_Right_Trackpad_Touch)");
	actionMapping.Add("(ActionName=\"C3D_RightTouchpadTouch\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=Vive_Right_Trackpad_Touch)");
	actionMapping.Add("(ActionName=\"C3D_LeftTouchpadPress\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=MixedReality_Left_Trackpad_Click)");
	actionMapping.Add("(ActionName=\"C3D_LeftTouchpadPress\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=ValveIndex_Left_Trackpad_Force)");
	actionMapping.Add("(ActionName=\"C3D_LeftTouchpadPress\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=Vive_Left_Trackpad_Click)");
	actionMapping.Add("(ActionName=\"C3D_RightTouchpadPress\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=MixedReality_Right_Trackpad_Click)");
	actionMapping.Add("(ActionName=\"C3D_RightTouchpadPress\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=ValveIndex_Right_Trackpad_Force)");
	actionMapping.Add("(ActionName=\"C3D_RightTouchpadPress\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=Vive_Right_Trackpad_Click)");
	actionMapping.Add("(ActionName=\"C3D_LeftJoystick\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=MixedReality_Left_Thumbstick_Click)");
	actionMapping.Add("(ActionName=\"C3D_LeftJoystick\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=ValveIndex_Left_Thumbstick_Click)");
	actionMapping.Add("(ActionName=\"C3D_LeftJoystick\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=OculusTouch_Left_Thumbstick_Click)");
	actionMapping.Add("(ActionName=\"C3D_RightJoystick\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=MixedReality_Right_Thumbstick_Click)");
	actionMapping.Add("(ActionName=\"C3D_RightJoystick\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=ValveIndex_Right_Thumbstick_Click)");
	actionMapping.Add("(ActionName=\"C3D_RightJoystick\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=OculusTouch_Right_Thumbstick_Click)");
	actionMapping.Add("(ActionName=\"C3D_LeftFaceButtonOne\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=MixedReality_Left_Trackpad_Up)");
	actionMapping.Add("(ActionName=\"C3D_LeftFaceButtonOne\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=OculusTouch_Left_X_Click)");
	actionMapping.Add("(ActionName=\"C3D_LeftFaceButtonOne\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=ValveIndex_Left_A_Click)");
	actionMapping.Add("(ActionName=\"C3D_LeftFaceButtonTwo\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=MixedReality_Left_Trackpad_Right)");
	actionMapping.Add("(ActionName=\"C3D_LeftFaceButtonTwo\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=OculusTouch_Left_Y_Click)");
	actionMapping.Add("(ActionName=\"C3D_LeftFaceButtonTwo\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=ValveIndex_Left_B_Click)");
	actionMapping.Add("(ActionName=\"C3D_RightFaceButtonOne\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=MixedReality_Right_Trackpad_Up)");
	actionMapping.Add("(ActionName=\"C3D_RightFaceButtonOne\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=OculusTouch_Right_A_Click)");
	actionMapping.Add("(ActionName=\"C3D_RightFaceButtonOne\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=ValveIndex_Right_A_Click)");
	actionMapping.Add("(ActionName=\"C3D_RightFaceButtonTwo\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=MixedReality_Right_Trackpad_Right)");
	actionMapping.Add("(ActionName=\"C3D_RightFaceButtonTwo\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=OculusTouch_Right_B_Click)");
	actionMapping.Add("(ActionName=\"C3D_RightFaceButtonTwo\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=ValveIndex_Right_B_Click)");
	actionMapping.Add("(ActionName=\"C3D_LeftMenuButton\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=OculusTouch_Left_Menu_Click)");
	actionMapping.Add("(ActionName=\"C3D_LeftMenuButton\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=ValveIndex_Left_System_Click)");
	actionMapping.Add("(ActionName=\"C3D_LeftMenuButton\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=MixedReality_Left_Menu_Click)");
	actionMapping.Add("(ActionName=\"C3D_LeftMenuButton\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=Vive_Left_Menu_Click)");
	actionMapping.Add("(ActionName=\"C3D_RightMenuButton\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=ValveIndex_Right_System_Click)");
	actionMapping.Add("(ActionName=\"C3D_RightMenuButton\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=MixedReality_Right_Menu_Click)");
	actionMapping.Add("(ActionName=\"C3D_RightMenuButton\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=Vive_Right_Menu_Click)");
	actionMapping.Add("(ActionName=\"C3D_RightMenuButton\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=MixedReality_Right_Menu_Click)");

	axisMapping.Add("(AxisName=\"C3D_LeftTouchpadH\",Scale=1.000000,Key=MixedReality_Left_Trackpad_X)");
	axisMapping.Add("(AxisName=\"C3D_LeftTouchpadH\",Scale=1.000000,Key=Vive_Left_Trackpad_X)");
	axisMapping.Add("(AxisName=\"C3D_LeftTouchpadV\",Scale=1.000000,Key=MixedReality_Left_Trackpad_Y)");
	axisMapping.Add("(AxisName=\"C3D_LeftTouchpadV\",Scale=1.000000,Key=Vive_Left_Trackpad_Y)");
	axisMapping.Add("(AxisName=\"C3D_RightTouchpadH\",Scale=1.000000,Key=MixedReality_Right_Trackpad_X)");
	axisMapping.Add("(AxisName=\"C3D_RightTouchpadH\",Scale=1.000000,Key=Vive_Right_Trackpad_X)");
	axisMapping.Add("(AxisName=\"C3D_RightTouchpadV\",Scale=1.000000,Key=MixedReality_Right_Trackpad_Y)");
	axisMapping.Add("(AxisName=\"C3D_RightTouchpadV\",Scale=1.000000,Key=Vive_Right_Trackpad_Y)");
	axisMapping.Add("(AxisName=\"C3D_LeftJoystickH\",Scale=1.000000,Key=OculusTouch_Left_Thumbstick_X)");
	axisMapping.Add("(AxisName=\"C3D_LeftJoystickH\",Scale=1.000000,Key=ValveIndex_Left_Thumbstick_X)");
	axisMapping.Add("(AxisName=\"C3D_LeftJoystickH\",Scale=1.000000,Key=MixedReality_Left_Thumbstick_X)");
	axisMapping.Add("(AxisName=\"C3D_LeftJoystickV\",Scale=1.000000,Key=OculusTouch_Left_Thumbstick_Y)");
	axisMapping.Add("(AxisName=\"C3D_LeftJoystickV\",Scale=1.000000,Key=ValveIndex_Left_Thumbstick_Y)");
	axisMapping.Add("(AxisName=\"C3D_LeftJoystickV\",Scale=1.000000,Key=MixedReality_Left_Thumbstick_Y)");
	axisMapping.Add("(AxisName=\"C3D_RightJoystickH\",Scale=1.000000,Key=OculusTouch_Right_Thumbstick_X)");
	axisMapping.Add("(AxisName=\"C3D_RightJoystickH\",Scale=1.000000,Key=ValveIndex_Right_Thumbstick_X)");
	axisMapping.Add("(AxisName=\"C3D_RightJoystickH\",Scale=1.000000,Key=MixedReality_Right_Thumbstick_X)");
	axisMapping.Add("(AxisName=\"C3D_RightJoystickV\",Scale=1.000000,Key=OculusTouch_Right_Thumbstick_Y)");
	axisMapping.Add("(AxisName=\"C3D_RightJoystickV\",Scale=1.000000,Key=ValveIndex_Right_Thumbstick_Y)");
	axisMapping.Add("(AxisName=\"C3D_RightJoystickV\",Scale=1.000000,Key=MixedReality_Right_Thumbstick_Y)");
	axisMapping.Add("(AxisName=\"C3D_LeftGripAxis\",Scale=1.000000,Key=OculusTouch_Left_Grip_Axis)");
	axisMapping.Add("(AxisName=\"C3D_LeftGripAxis\",Scale=1.000000,Key=ValveIndex_Left_Grip_Axis)");
	axisMapping.Add("(AxisName=\"C3D_LeftGripAxis\",Scale=1.000000,Key=MixedReality_Left_Grip_Click)");
	axisMapping.Add("(AxisName=\"C3D_LeftGripAxis\",Scale=1.000000,Key=Vive_Left_Grip_Click)");
	axisMapping.Add("(AxisName=\"C3D_RightGripAxis\",Scale=1.000000,Key=OculusTouch_Right_Grip_Axis)");
	axisMapping.Add("(AxisName=\"C3D_RightGripAxis\",Scale=1.000000,Key=ValveIndex_Right_Grip_Axis)");
	axisMapping.Add("(AxisName=\"C3D_RightGripAxis\",Scale=1.000000,Key=MixedReality_Right_Grip_Click)");
	axisMapping.Add("(AxisName=\"C3D_RightGripAxis\",Scale=1.000000,Key=Vive_Right_Grip_Click)");
	axisMapping.Add("(AxisName=\"C3D_LeftTriggerAxis\",Scale=1.000000,Key=OculusTouch_Left_Trigger_Axis)");
	axisMapping.Add("(AxisName=\"C3D_LeftTriggerAxis\",Scale=1.000000,Key=ValveIndex_Left_Trigger_Axis)");
	axisMapping.Add("(AxisName=\"C3D_LeftTriggerAxis\",Scale=1.000000,Key=MixedReality_Left_Trigger_Axis)");
	axisMapping.Add("(AxisName=\"C3D_LeftTriggerAxis\",Scale=1.000000,Key=Vive_Left_Trigger_Axis)");
	axisMapping.Add("(AxisName=\"C3D_RightTriggerAxis\",Scale=1.000000,Key=OculusTouch_Right_Trigger_Axis)");
	axisMapping.Add("(AxisName=\"C3D_RightTriggerAxis\",Scale=1.000000,Key=ValveIndex_Right_Trigger_Axis)");
	axisMapping.Add("(AxisName=\"C3D_RightTriggerAxis\",Scale=1.000000,Key=MixedReality_Right_Trigger_Axis)");
	axisMapping.Add("(AxisName=\"C3D_RightTriggerAxis\",Scale=1.000000,Key=Vive_Right_Trigger_Axis)");

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 2
	GConfig->SetArray(TEXT("/Script/Engine.InputSettings"), TEXT("+ActionMappings"), actionMapping, NormalizedInputIni);
	GConfig->SetArray(TEXT("/Script/Engine.InputSettings"), TEXT("+AxisMappings"), axisMapping, NormalizedInputIni);

	GConfig->Flush(false, NormalizedInputIni);
#else
	GConfig->SetArray(TEXT("/Script/Engine.InputSettings"), TEXT("+ActionMappings"), actionMapping, InputIni);
	GConfig->SetArray(TEXT("/Script/Engine.InputSettings"), TEXT("+AxisMappings"), axisMapping, InputIni);

	GConfig->Flush(false, InputIni);
#endif

	GLog->Log("SSceneSetupWidget::AppendInputs complete");

	return FReply::Handled();
}

EVisibility SProjectManagerWidget::GetAppendedInputsFoundHidden() const
{
	FString InputIni = FPaths::Combine(*(FPaths::ProjectDir()), TEXT("Config/DefaultInput.ini"));

	TArray<FString> actionMapping;

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 2
	const FString NormalizedInputIni = GConfig->NormalizeConfigIniPath(InputIni);
	GConfig->GetArray(TEXT("/Script/Engine.InputSettings"), TEXT("+ActionMappings"), actionMapping, NormalizedInputIni);
#else
	GConfig->GetArray(TEXT("/Script/Engine.InputSettings"), TEXT("+ActionMappings"), actionMapping, InputIni);
#endif

	if (actionMapping.Contains("(ActionName=\"C3D_LeftGrip\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=OculusTouch_Left_Grip_Click)"))
	{
		return EVisibility::Collapsed;
	}
	return EVisibility::Visible;
}

EVisibility SProjectManagerWidget::GetAppendedInputsFoundVisibility() const
{
	FString InputIni = FPaths::Combine(*(FPaths::ProjectDir()), TEXT("Config/DefaultInput.ini"));

	TArray<FString> actionMapping;

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 2
	const FString NormalizedInputIni = GConfig->NormalizeConfigIniPath(InputIni);
	GConfig->GetArray(TEXT("/Script/Engine.InputSettings"), TEXT("+ActionMappings"), actionMapping, NormalizedInputIni);
#else
	GConfig->GetArray(TEXT("/Script/Engine.InputSettings"), TEXT("+ActionMappings"), actionMapping, InputIni);
#endif

	if (actionMapping.Contains("(ActionName=\"C3D_LeftGrip\",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=OculusTouch_Left_Grip_Click)"))
	{
		return EVisibility::Visible;
	}
	return EVisibility::Collapsed;
}

EVisibility SProjectManagerWidget::GetDefaultInputClassEnhanced() const
{
	//show AppendInput button if its not using enhanced input
	FString DefaultPlayerInputClass;
	FString DefaultInputComponentClass;

	// Retrieve the settings from DefaultInput.ini
	bool bFoundPlayerInputClass = GConfig->GetString(
		TEXT("/Script/Engine.InputSettings"),
		TEXT("DefaultPlayerInputClass"),
		DefaultPlayerInputClass,
		GInputIni
	);

	bool bFoundInputComponentClass = GConfig->GetString(
		TEXT("/Script/Engine.InputSettings"),
		TEXT("DefaultInputComponentClass"),
		DefaultInputComponentClass,
		GInputIni
	);

	// Check if either of the classes are set to Enhanced Input classes
	//if its using it, collapse the box showing the append input button
	bool bIsEnhancedInput = false;

	if (bFoundPlayerInputClass && bFoundInputComponentClass)
	{
		bIsEnhancedInput = DefaultPlayerInputClass.Contains("EnhancedPlayerInput") ||
			DefaultInputComponentClass.Contains("EnhancedInputComponent");
	}

	if (bIsEnhancedInput)
	{
		return EVisibility::Collapsed;
	}
	else
	{
		return EVisibility::Visible;
	}
}

void SProjectManagerWidget::CollectAllMaps()
{
	UE_LOG(LogTemp, Warning, TEXT("CollectAllMaps called"));
	TArray<FAssetData> MapAssets;
	FARFilter Filter;
#if ENGINE_MAJOR_VERSION == 4
	Filter.ClassNames.Add(UWorld::StaticClass()->GetFName());
#elif ENGINE_MAJOR_VERSION == 5 && (ENGINE_MINOR_VERSION == 0 || ENGINE_MINOR_VERSION == 1)
	Filter.ClassNames.Add(UWorld::StaticClass()->GetFName());
#elif ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 2 
	Filter.ClassPaths.Add(UWorld::StaticClass()->GetClassPathName());
#endif
	Filter.bRecursivePaths = true;
	Filter.PackagePaths.Add(FName(TEXT("/Game"))); // Covers all

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	AssetRegistryModule.Get().GetAssets(Filter, MapAssets);
	UE_LOG(LogTemp, Warning, TEXT("Found %d map assets"), MapAssets.Num());

	for (const FAssetData& Asset : MapAssets)
	{
		UE_LOG(LogTemp, Warning, TEXT("Found map: %s"), *Asset.ObjectPath.ToString());
		LevelSelectionMap.Add(Asset.ObjectPath.ToString(), false);
	}
}

void SProjectManagerWidget::FinalizeProjectSetup()
{
	//empty dynamics map
	DynamicObjecstMap.Empty();
	//UE_LOG(LogTemp, Warning, TEXT("FinalizeProjectSetup called"));
	UWorld* World = GEditor->GetEditorWorldContext().World();
	FString OriginalMap;

	if (World)
	{
		OriginalMap = World->GetOutermost()->GetName(); // e.g. /Game/Maps/MyMap
		//UE_LOG(LogTemp, Warning, TEXT("Current World: %s"), *OriginalMap);
	}
	
	OnExportAllSceneGeometry.BindSP(this, &SProjectManagerWidget::OnLevelsExported);

	// Gather only the user-checked entries
	TArray<FString> SelectedKeys;
	for (const TPair<FString, bool>& Pair : LevelSelectionMap)
	{
		if (Pair.Value)
		{
			SelectedKeys.Add(Pair.Key);
			UE_LOG(LogTemp, Warning, TEXT("Selected map: %s"), *Pair.Key);
		}
	}


	//Auto-discover sublevel->parent mappings 
	TMap<FString, FString> SublevelToParent;
	{
		// We need a list of all potential persistent maps under /Game/Maps
		TArray<FAssetData> AllMaps;
		FARFilter MapFilter;
		MapFilter.bRecursivePaths = true;
		MapFilter.PackagePaths.Empty();
#if ENGINE_MAJOR_VERSION == 4
		MapFilter.ClassNames.Add(UWorld::StaticClass()->GetFName());
#elif ENGINE_MAJOR_VERSION == 5
		MapFilter.ClassPaths.Add(UWorld::StaticClass()->GetClassPathName());
#endif
		FAssetRegistryModule& ARM = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		ARM.Get().GetAssets(MapFilter, AllMaps);

		for (auto& Asset : AllMaps)
		{
			const FString PackageName = Asset.ObjectPath.ToString().LeftChop(Asset.AssetName.ToString().Len() + 1);
			if (!FEditorFileUtils::LoadMap(PackageName, false, false))
			{
				continue;
			}
			UWorld* W = GEditor->GetEditorWorldContext().World();
			if (!W) continue;

			for (ULevelStreaming* LS : W->GetStreamingLevels())
			{
				if (LS)
				{
					const FString ChildPkg = LS->GetWorldAssetPackageName();
					SublevelToParent.FindOrAdd(ChildPkg) = PackageName;
					UE_LOG(LogTemp, Warning, TEXT("Mapped sublevel %s ? parent %s"), *ChildPkg, *PackageName);
				}
			}
		}

		// Restore whatever map was open before tinkering
		if (!OriginalMap.IsEmpty())
		{
			FEditorFileUtils::LoadMap(OriginalMap, false, true);
		}
	}

	//Build lookup & buckets directly from SelectedKeys (which are full object paths)
	TMap<FString, FString> PackageToAssetName;
	TArray<FString> PureMaps;
	TArray<FString> SublevelKeys;

	for (const FString& Key : SelectedKeys)
	{
		// Key == "/Game/.../MyMap.MyMap"
		// This yields "/Game/.../MyMap"
		const FString PackagePath = FPackageName::ObjectPathToPackageName(Key);
		// This yields "MyMap"
		const FString AssetName = FPackageName::GetShortName(PackagePath);

		PackageToAssetName.Add(PackagePath, AssetName);

		if (SublevelToParent.Contains(PackagePath))
		{
			SublevelKeys.AddUnique(PackagePath);
			UE_LOG(LogTemp, Warning, TEXT("Queue sublevel: %s (pkg: %s)"),
				*AssetName, *PackagePath);
		}
		else
		{
			PureMaps.AddUnique(PackagePath);
			UE_LOG(LogTemp, Warning, TEXT("Queue pure map:   %s (pkg: %s)"),
				*AssetName, *PackagePath);
		}
	}

	TotalLevelCount = PureMaps.Num() + SublevelKeys.Num();
	FCognitiveEditorTools::GetInstance()->TotalLevelsToUpload = TotalLevelCount;
	FCognitiveEditorTools::GetInstance()->TotalSetOfDynamicsToUpload = TotalLevelCount;
	//Create the slow task for TOTAL items = PureMaps + SublevelKeys
	FScopedSlowTask ExportTask(PureMaps.Num() + SublevelKeys.Num(), LOCTEXT("ExportingMaps", "Exporting selected maps..."));
	ExportTask.MakeDialog();

	//Export each pure map
	for (const FString& PackageToLoad : PureMaps)
	{
		UE_LOG(LogTemp, Warning, TEXT("Exporting map: %s"), *PackageToLoad);
		
		const FString& NameToExport = PackageToAssetName[PackageToLoad];
		ExportTask.EnterProgressFrame(1.f, FText::FromString(NameToExport));

		if (!FEditorFileUtils::LoadMap(PackageToLoad, false, true))
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to load %s"), *PackageToLoad);
			continue;
		}

		UWorld* W = GEditor->GetEditorWorldContext().World();
		if (!W) continue;

		// disable all sublevels
		for (ULevelStreaming* LS : W->GetStreamingLevels())
		{
			if (LS) { LS->SetShouldBeLoaded(false); LS->SetShouldBeVisible(false);}
		}
		W->FlushLevelStreaming(EFlushLevelStreamingType::Full);

		bool bFoundC3DActor = false;

		//spawn c3d actor
		//check if there's a Cognitive3DActor already in the world
		for (TObjectIterator<ACognitive3DActor> Itr; Itr; ++Itr)
		{
			if (Itr->IsPendingKillPending())
			{
				//if a ACognitive3DActor was deleted from the world, it sticks around but is pending a kill. possibly in some undo buffer?
				continue;
			}
			bFoundC3DActor = true;
		}

		if (!bFoundC3DActor)
		{
			//spawn a Cognitive3DActor blueprint
			UClass* classPtr = LoadObject<UClass>(nullptr, TEXT("/Cognitive3D/BP_Cognitive3DActor.BP_Cognitive3DActor_C"));
			if (classPtr)
			{
				AActor* obj = W->SpawnActor<AActor>(classPtr);
				obj->OnConstruction(obj->GetTransform());
				obj->PostActorConstruction();
				GLog->Log("SSceneSetupWidget::SpawnCognitive3DActor spawned BP_Cognitive3DActor in world");
			}
			else
			{
				GLog->Log("SSceneSetupWidget::SpawnCognitive3DActor couldn't find BP_Cognitive3DActor class");
			}

			//Mark the level dirty so the editor knows it's changed
			W->PersistentLevel->MarkPackageDirty();

			//Save the map back to disk so your spawned actor is baked in
			{
				UPackage* LevelPackage = W->PersistentLevel->GetOutermost();
				TArray<UPackage*> ToSave = { LevelPackage };
				FEditorFileUtils::PromptForCheckoutAndSave(ToSave, false, false);
			}
		}

		// only grab actors from the *persistent* level, never from sublevels
		TArray<AActor*> ActorsToExport;
		TArray<UDynamicObject*> DynamicObjects; //ExportDynamicObjectArray
		if (ULevel* Persistent = W->PersistentLevel)
		{
			for (AActor* A : Persistent->Actors)
			{
				if (A && !A->IsPendingKillPending() && !A->IsTemplate())
				{
					ActorsToExport.Add(A);

					//check for dynamic object component on actors
					if(UDynamicObject* DynamicObject = Cast<UDynamicObject>(A->GetComponentByClass(UDynamicObject::StaticClass())))
					{
						if (DynamicObject != nullptr)
						{
							DynamicObjects.Add(DynamicObject);
						}
					}
				}
			}
		}

		FCognitiveEditorTools::GetInstance()->ExportScene(PackageToLoad, ActorsToExport);

		if (FCognitiveEditorTools::GetInstance()->UploadingDynamicsFromFullSetup)
		{
			FCognitiveEditorTools::GetInstance()->ExportDynamicObjectArray(DynamicObjects);
			DynamicObjecstMap.Add(PackageToLoad, DynamicObjects);
		}
	}

	//Export each sublevel under its own name, but loading its parent world
	for (const FString& SubPath : SublevelKeys)
	{
		const FString& NameToExport = PackageToAssetName[SubPath];
		ExportTask.EnterProgressFrame(1.f, FText::FromString(NameToExport));
		const FString& ParentMap = SublevelToParent[SubPath];

		if (!FEditorFileUtils::LoadMap(ParentMap, false, true))
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to load parent %s for sublevel %s"), *ParentMap, *SubPath);
			continue;
		}

		UWorld* W = GEditor->GetEditorWorldContext().World();
		if (!W) continue;

		// turn off all, then only SubPath
		for (ULevelStreaming* LS : W->GetStreamingLevels())
		{
			if (!LS) continue;
			const FString ChildPkg = LS->GetWorldAssetPackageName();
			LS->SetShouldBeLoaded(ChildPkg == SubPath);
			LS->SetShouldBeVisible(ChildPkg == SubPath);
		}
		W->FlushLevelStreaming(EFlushLevelStreamingType::Full);

		// build the allowed set of levels
		TSet<ULevel*> AllowedLevels;
		AllowedLevels.Add(W->PersistentLevel);

		// find the one streaming level that matches our SubPkg and add its level
		for (ULevelStreaming* LS : W->GetStreamingLevels())
		{
			if (!LS) continue;
			if (LS->GetWorldAssetPackageName() == SubPath)
			{
				if (ULevel* Loaded = LS->GetLoadedLevel())
				{
					AllowedLevels.Add(Loaded);
				}
				break;
			}
		}

		// now only export actors from those levels
		TArray<AActor*> ActorsToExport;
		//TArray<TSharedPtr<UDynamicObject*>> DynamicObjects; //ExportDynamicObjectArray
		TArray<UDynamicObject*> DynamicObjects;
		for (ULevel* Level : W->GetLevels())
		{
			if (!AllowedLevels.Contains(Level))
				continue;

			for (AActor* Actor : Level->Actors)
			{
				if (Actor && !Actor->IsPendingKillPending() && !Actor->IsTemplate())
				{
					ActorsToExport.Add(Actor);

					//check for dynamic object component on actors
					if (UDynamicObject* DynamicObject = Cast<UDynamicObject>(Actor->GetComponentByClass(UDynamicObject::StaticClass())))
					{
						if (DynamicObject != nullptr)
						{
							DynamicObjects.Add(DynamicObject);
						}
					}
				}
			}
		}

		// Export under the sublevel's name
		FCognitiveEditorTools::GetInstance()->ExportScene(SubPath, ActorsToExport);

		if (FCognitiveEditorTools::GetInstance()->UploadingDynamicsFromFullSetup)
		{
			FCognitiveEditorTools::GetInstance()->ExportDynamicObjectArray(DynamicObjects);
			DynamicObjecstMap.Add(SubPath, DynamicObjects);
		}
	}

	// Restore original map
	if (!OriginalMap.IsEmpty())
	{
		FEditorFileUtils::LoadMap(OriginalMap, false, true);
	}

	OnExportAllSceneGeometry.ExecuteIfBound(true);
}

void SProjectManagerWidget::OnLevelsExported(bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		UE_LOG(LogTemp, Warning, TEXT("SProjectManagerWidget::OnLevelsExported called with %d levels to upload"), TotalLevelCount);
		FScopedSlowTask UploadTask(TotalLevelCount, LOCTEXT("UploadingMaps", "Uploading selected maps..."));
		UploadTask.MakeDialog();

		UWorld* World = GEditor->GetEditorWorldContext().World();
		FString OriginalMap;

		if (World)
		{
			OriginalMap = World->GetOutermost()->GetName(); // e.g. /Game/Maps/MyMap
			//UE_LOG(LogTemp, Warning, TEXT("Current World: %s"), *OriginalMap);
		}

		FCognitiveEditorTools::GetInstance()->OnUploadAllSceneGeometry.BindSP(this, &SProjectManagerWidget::OnLevelsUploaded);
		FCognitiveEditorTools::GetInstance()->UploadingScenesFromFullSetup = true;

		for (const TPair<FString, bool>& Pair : LevelSelectionMap)
		{
			if (Pair.Value)
			{
				FString LevelName = FPackageName::GetShortName(Pair.Key);
				LevelName.Split(TEXT("."), nullptr, &LevelName); // Remove package extension
				//start task
				UploadTask.EnterProgressFrame(1.0f, FText::FromString(Pair.Key));
				//load the map for the current pair
				if (!FEditorFileUtils::LoadMap(Pair.Key, false, true))
				{
					UE_LOG(LogTemp, Warning, TEXT("Failed to load %s"), *Pair.Key);
					continue;
				}
				FCognitiveEditorTools::GetInstance()->RefreshDisplayDynamicObjectsCountInScene();
				//FString AdjustedLevelName = Pair.Key.Replace(TEXT("/"), TEXT("_")); // Replace slashes with underscores for upload
				FString AdjustedLevelName = FCognitiveEditorTools::GetInstance()->AdjustPathName(Pair.Key);
				AdjustedLevelName.Split(TEXT("."), &AdjustedLevelName, nullptr ); // Remove package extension
				UE_LOG(LogTemp, Warning, TEXT("SProjectManagerWidget::OnLevelsExported Uploading level: %s"), *AdjustedLevelName);
				FCognitiveEditorTools::GetInstance()->WizardUpload(AdjustedLevelName);
			}
		}

		// Restore original map
		if (!OriginalMap.IsEmpty())
		{
			FEditorFileUtils::LoadMap(OriginalMap, false, true);
		}
	}
}

void SProjectManagerWidget::OnLevelsUploaded(bool bWasSuccessful)
{
	
	if (bWasSuccessful)
	{
		if (!FCognitiveEditorTools::GetInstance()->UploadingDynamicsFromFullSetup)
		{
			UE_LOG(LogTemp, Warning, TEXT("SProjectManagerWidget::OnLevelsUploaded Uploading dynamics from full setup is disabled, skipping dynamics upload"));
			//RestartEditor();
			//go straight to the last step and prompt restart the editor
			OnUploadAllSceneGeometry.BindSP(this, &SProjectManagerWidget::OnDynamicsUploaded);
			OnUploadAllSceneGeometry.ExecuteIfBound(true);
			return;
		}
		UE_LOG(LogTemp, Warning, TEXT("SProjectManagerWidget::OnLevelsUploaded called with %d dynamics to upload"), TotalLevelCount);
		FCognitiveEditorTools::GetInstance()->OnUploadAllDynamics.BindSP(this, &SProjectManagerWidget::OnDynamicsUploaded);

		//FScopedSlowTask UploadDynTask(TotalLevelCount, LOCTEXT("UploadingDynamics", "Uploading level dynamics..."));
		//UploadDynTask.MakeDialog();

		UWorld* World = GEditor->GetEditorWorldContext().World();
		FString OriginalMap;

		if (World)
		{
			OriginalMap = World->GetOutermost()->GetName(); // e.g. /Game/Maps/MyMap
			//UE_LOG(LogTemp, Warning, TEXT("Current World: %s"), *OriginalMap);
		}

		//go through the map for <path, dynamics> and upload each dynamics manifest
		for (const TPair<FString, TArray<UDynamicObject*>>& Pair : DynamicObjecstMap)
		{
			//UploadDynTask.EnterProgressFrame(1.f, FText::FromString(Pair.Key));

			//load the map for the current pair
			if (!FEditorFileUtils::LoadMap(Pair.Key, false, true))
			{
				UE_LOG(LogTemp, Warning, TEXT("Failed to load %s"), *Pair.Key);
				continue;
			}
			FCognitiveEditorTools::GetInstance()->RefreshDisplayDynamicObjectsCountInScene();
			FString LevelName = FPackageName::GetShortName(Pair.Key);
			LevelName.Split(TEXT("."), nullptr, &LevelName); // Remove package extension
			//FString AdjustedLevelName = Pair.Key.Replace(TEXT("/"), TEXT("_")); // Replace slashes with underscores for upload
			FString AdjustedLevelName = FCognitiveEditorTools::GetInstance()->AdjustPathName(Pair.Key);
			AdjustedLevelName.Split(TEXT("."), &AdjustedLevelName, nullptr); // Remove package extension
			UE_LOG(LogTemp, Warning, TEXT("SProjectManagerWidget::OnLevelsExported Uploading dynamics for level: %s"), *AdjustedLevelName);
			FCognitiveEditorTools::GetInstance()->UploadDynamicsManifest(AdjustedLevelName);
		}
		// Restore original map
		if (!OriginalMap.IsEmpty())
		{
			FEditorFileUtils::LoadMap(OriginalMap, false, true);
		}
	}
}

void SProjectManagerWidget::OnDynamicsUploaded(bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		RestartEditor();
	}
}


void SProjectManagerWidget::ApplySDKToggle(const FString& SDKName, bool bEnable)
{
	// --- 1) Update build.cs --- 

	const FString BuildCsPath = FPaths::ProjectDir() / TEXT("Plugins/Cognitive3D/Source/Cognitive3D/Cognitive3D.build.cs");
	TArray<FString> Lines;
	if (FFileHelper::LoadFileToStringArray(Lines, *BuildCsPath))
	{
		auto FixLine = [&](FString& Line, const FString& Method)
			{
				const FString Commented = FString::Printf(TEXT("//%s();"), *Method);
				const FString UnComment = FString::Printf(TEXT("%s();"), *Method);

				if (bEnable && Line.Contains(Commented))
				{
					Line = Line.Replace(*Commented, *UnComment);
				}
				else if (!bEnable && Line.Contains(UnComment))
				{
					if (Line.Contains(TEXT("//") + Method))
					{
						
					}
					else
					{
						Line = Line.Replace(*UnComment, *Commented);
					}
				}
			};

		for (auto& L : Lines)
		{
			if (SDKName == TEXT("MetaXR"))
			{
				if (bEnable)
				{
					FixLine(L, TEXT("MetaXRPlugin"));
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 2
					FixLine(L, TEXT("MetaXRPassthrough"));
#endif
				}
				else
				{
					FixLine(L, TEXT("MetaXRPlugin"));
					FixLine(L, TEXT("MetaXRPassthrough"));
					FixLine(L, TEXT("MetaXRPlatform"));
				}
			}
			else if (SDKName == TEXT("MetaXRPlatform"))
			{
				FixLine(L, TEXT("MetaXRPlatform"));
			}
			else if (SDKName == TEXT("PicoXR"))
			{
				FixLine(L, TEXT("PICOXR"));
			}
			else if (SDKName == TEXT("WaveVR"))
			{
				FixLine(L, TEXT("WaveVREyeTracking"));
			}
		}

		FFileHelper::SaveStringArrayToFile(Lines, *BuildCsPath);
	}

	// --- 2) Update .uproject Plugins array ---
	const FString UProjectPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir() + FApp::GetProjectName() + TEXT(".uproject"));
	FString JsonRaw;
	if (FFileHelper::LoadFileToString(JsonRaw, *UProjectPath))
	{
		TSharedPtr<FJsonObject> RootObj;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonRaw);

		if (FJsonSerializer::Deserialize(Reader, RootObj) && RootObj.IsValid())
		{
			TArray<TSharedPtr<FJsonValue>> Plugins = RootObj->GetArrayField(TEXT("Plugins"));
			bool bModified = false;

			if (bEnable)
			{
				bool bFound = false;
				for (auto& Val : Plugins)
				{
					if (SDKName == TEXT("MetaXR"))
					{
#if ENGINE_MAJOR_VERSION == 5
						auto Obj = Val->AsObject();
						if (Obj->GetStringField(TEXT("Name")) == "OculusXR")
						{
							Obj->SetBoolField(TEXT("Enabled"), bEnable);
							Obj->SetArrayField(TEXT("SupportedTargetPlatforms"), { MakeShared<FJsonValueString>(TEXT("Win64")), MakeShared<FJsonValueString>(TEXT("Android")) });
							bFound = true;
							break;
						}
#elif ENGINE_MAJOR_VERSION == 4
						auto Obj = Val->AsObject();
						if (Obj->GetStringField(TEXT("Name")) == "OculusVR")
						{
							Obj->SetBoolField(TEXT("Enabled"), bEnable);
							Obj->SetArrayField(TEXT("SupportedTargetPlatforms"), { MakeShared<FJsonValueString>(TEXT("Win64")), MakeShared<FJsonValueString>(TEXT("Win32")), MakeShared<FJsonValueString>(TEXT("Android")) });
							bFound = true;
							break;
						}
#endif
					}
					else if (SDKName == TEXT("MetaXRPlatform"))
					{
						auto Obj = Val->AsObject();
						if (Obj->GetStringField(TEXT("Name")) == "OculusPlatform")
						{
							Obj->SetBoolField(TEXT("Enabled"), bEnable);
							Obj->SetArrayField(TEXT("SupportedTargetPlatforms"), { MakeShared<FJsonValueString>(TEXT("Win64")), MakeShared<FJsonValueString>(TEXT("Android")) });
							bFound = true;
							break;
						}
					}
					else if (SDKName == TEXT("PicoXR"))
					{
						auto Obj = Val->AsObject();
						if (Obj->GetStringField(TEXT("Name")) == "PICOXR")
						{
							Obj->SetBoolField(TEXT("Enabled"), bEnable);
							Obj->SetArrayField(TEXT("SupportedTargetPlatforms"), { MakeShared<FJsonValueString>(TEXT("Win64")), MakeShared<FJsonValueString>(TEXT("Android")) });
							bFound = true;
							break;
						}
					}
					else if (SDKName == TEXT("WaveVR"))
					{
						auto Obj = Val->AsObject();
						if (Obj->GetStringField(TEXT("Name")) == "WaveVR")
						{
							Obj->SetBoolField(TEXT("Enabled"), bEnable);
							bFound = true;
							break;
						}
					}
				}

				if (!bFound)
				{
					if (SDKName == TEXT("MetaXR"))
					{
#if ENGINE_MAJOR_VERSION == 5
						TSharedPtr<FJsonObject> NewPlugin = MakeShared<FJsonObject>();
						NewPlugin->SetStringField(TEXT("Name"), "OculusXR");
						NewPlugin->SetBoolField(TEXT("Enabled"), true);
						NewPlugin->SetArrayField(TEXT("SupportedTargetPlatforms"), { MakeShared<FJsonValueString>(TEXT("Win64")), MakeShared<FJsonValueString>(TEXT("Android")) });
						Plugins.Add(MakeShared<FJsonValueObject>(NewPlugin));
#elif ENGINE_MAJOR_VERSION == 4
						TSharedPtr<FJsonObject> NewPlugin = MakeShared<FJsonObject>();
						NewPlugin->SetStringField(TEXT("Name"), "OculusVR");
						NewPlugin->SetBoolField(TEXT("Enabled"), true);
						NewPlugin->SetArrayField(TEXT("SupportedTargetPlatforms"), { MakeShared<FJsonValueString>(TEXT("Win64")), MakeShared<FJsonValueString>(TEXT("Win32")), MakeShared<FJsonValueString>(TEXT("Android")) });
						Plugins.Add(MakeShared<FJsonValueObject>(NewPlugin));
#endif
					}
					else if (SDKName == TEXT("MetaXRPlatform"))
					{
						TSharedPtr<FJsonObject> NewPlugin = MakeShared<FJsonObject>();
						NewPlugin->SetStringField(TEXT("Name"), "OculusPlatform");
						NewPlugin->SetBoolField(TEXT("Enabled"), true);
						NewPlugin->SetArrayField(TEXT("SupportedTargetPlatforms"), { MakeShared<FJsonValueString>(TEXT("Win64")), MakeShared<FJsonValueString>(TEXT("Android")) });
						Plugins.Add(MakeShared<FJsonValueObject>(NewPlugin));
					}
					else if (SDKName == TEXT("PicoXR"))
					{
						TSharedPtr<FJsonObject> NewPlugin = MakeShared<FJsonObject>();
						NewPlugin->SetStringField(TEXT("Name"), "PICOXR");
						NewPlugin->SetBoolField(TEXT("Enabled"), true);
						NewPlugin->SetArrayField(TEXT("SupportedTargetPlatforms"), { MakeShared<FJsonValueString>(TEXT("Win64")), MakeShared<FJsonValueString>(TEXT("Android")) });
						Plugins.Add(MakeShared<FJsonValueObject>(NewPlugin));
					}
					else if (SDKName == TEXT("WaveVR"))
					{
						TSharedPtr<FJsonObject> NewPlugin = MakeShared<FJsonObject>();
						NewPlugin->SetStringField(TEXT("Name"), "WaveVR");
						NewPlugin->SetBoolField(TEXT("Enabled"), true);
						Plugins.Add(MakeShared<FJsonValueObject>(NewPlugin));
					}
				}
				bModified = true;
			}
			else
			{
				// If disabling: remove any object based on its name
				for (int32 Index = Plugins.Num() - 1; Index >= 0; --Index)
				{
					if (SDKName == TEXT("MetaXR"))
					{
#if ENGINE_MAJOR_VERSION == 5
						TSharedPtr<FJsonObject> Obj = Plugins[Index]->AsObject();
						if (Obj->GetStringField(TEXT("Name")) == TEXT("OculusXR") || Obj->GetStringField(TEXT("Name")) == TEXT("OculusPlatform"))
						{
							Obj->SetBoolField(TEXT("Enabled"), false);
							bModified = true;
						}
#elif ENGINE_MAJOR_VERSION == 4
						TSharedPtr<FJsonObject> Obj = Plugins[Index]->AsObject();
						if (Obj->GetStringField(TEXT("Name")) == TEXT("OculusVR") || Obj->GetStringField(TEXT("Name")) == TEXT("OculusPlatform"))
						{
							Obj->SetBoolField(TEXT("Enabled"), false);
							bModified = true;
						}
						SDKCheckboxStates.Add(TEXT("MetaXRPlatform"), false);
						IsSDKEnabledInBuildCs(TEXT("MetaXRPlatform"));
#endif
					}
					else if (SDKName == TEXT("MetaXRPlatform"))
					{
						TSharedPtr<FJsonObject> Obj = Plugins[Index]->AsObject();
						if (Obj->GetStringField(TEXT("Name")) == TEXT("OculusPlatform"))
						{
							Obj->SetBoolField(TEXT("Enabled"), false);
							bModified = true;
						}
					}
					else if (SDKName == TEXT("PicoXR"))
					{
						TSharedPtr<FJsonObject> Obj = Plugins[Index]->AsObject();
						if (Obj->GetStringField(TEXT("Name")) == TEXT("PICOXR"))
						{
							Obj->SetBoolField(TEXT("Enabled"), false);
							bModified = true;
						}
					}
					else if (SDKName == TEXT("WaveVR"))
					{
						TSharedPtr<FJsonObject> Obj = Plugins[Index]->AsObject();
						if (Obj->GetStringField(TEXT("Name")) == TEXT("WaveVR"))
						{
							Obj->SetBoolField(TEXT("Enabled"), false);
							bModified = true;
						}
					}
				}
			}

			if (bModified)
			{
				RootObj->SetArrayField(TEXT("Plugins"), Plugins);

				FString Output;
				TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Output);
				if (FJsonSerializer::Serialize(RootObj.ToSharedRef(), Writer))
				{
					FFileHelper::SaveStringToFile(Output, *UProjectPath);
				}
			}
		}
	}

	bDidChangeSDKs = true;

	FCognitiveEditorTools::GetInstance()->ShowNotification("Third-Party SDK changes made, editor will restart and rebuild the project upon finalizing setup", true);
	UE_LOG(LogTemp, Warning, TEXT("Third-Party SDK changes made, editor will restart and rebuild the project upon finalizing setup."));
}

bool SProjectManagerWidget::IsSDKEnabledInBuildCs(const FString& MethodName)
{
	const FString BuildCsPath =
		FPaths::ProjectDir() / TEXT("Plugins/Cognitive3D/Source/Cognitive3D/Cognitive3D.build.cs");

	TArray<FString> Lines;
	if (!FFileHelper::LoadFileToStringArray(Lines, *BuildCsPath))
	{
		// If we can’t read it, default to false
		return false;
	}

	const FString LiveCall = FString::Printf(TEXT("%s();"), *MethodName);
	const FString CommentPrefix = TEXT("//");

	for (const FString& L : Lines)
	{
		// Trim leading whitespace
		FString Trimmed = L;
		Trimmed.TrimStartInline();

		// If it starts with //MetaXRPlugin();, that means commented out, skip
		if (Trimmed.StartsWith(CommentPrefix + LiveCall))
		{
			continue;
		}

		// If the line contains MetaXRPlugin(); AND does NOT start with //, that means enabled:
		if (Trimmed.Contains(LiveCall) && !Trimmed.StartsWith(CommentPrefix))
		{
			return true;
		}
	}

	return false;
}

void SProjectManagerWidget::RestartEditor()
{
	if (!bDidChangeSDKs)
	{
		const EAppReturnType::Type Choice = FMessageDialog::Open(
			EAppMsgType::Ok,
			LOCTEXT("FinalizeSetupPrompt",
				"You are now ready to use the Cognitive3D plugin to collect spatial analytics for your project!")
		);

		if (Choice == EAppReturnType::Ok)
		{
			FCognitive3DEditorModule::CloseFullC3DSetup();
		}

		// No SDK changes, no need to restart
		return;
	}

	const EAppReturnType::Type Choice = FMessageDialog::Open(
		EAppMsgType::YesNo,
		LOCTEXT("RestartPrompt",
			"Your changes will only take effect after restarting the editor.\n\nRestart now?")
	);

	// Check if config file is already set to auto recompile on startup.
	bool bAutoRecompile = false;
	GConfig->GetBool(TEXT("/Script/UnrealEd.EditorLoadingSavingSettings"), TEXT("bForceCompilationAtStartup"), bAutoRecompile, GEditorPerProjectIni);
	FString C3DSettingsPath = FCognitiveEditorTools::GetInstance()->GetSettingsFilePath();
	GConfig->LoadFile(C3DSettingsPath);
	GConfig->SetString(TEXT("/Script/Cognitive3D.Cognitive3DSettings"), TEXT("RestartAfterSetup"), TEXT("False"), C3DSettingsPath);

	if (!bAutoRecompile)
	{
		GConfig->SetBool(TEXT("/Script/UnrealEd.EditorLoadingSavingSettings"), TEXT("bForceCompilationAtStartup"), true, GEditorPerProjectIni);
		GConfig->Flush(false, GEditorPerProjectIni);
		UE_LOG(LogTemp, Log, TEXT("Enabling Editor Auto Recompile at Startup"));
		GConfig->SetString(TEXT("/Script/Cognitive3D.Cognitive3DSettings"), TEXT("RestartAfterSetup"), TEXT("True"), C3DSettingsPath);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Editor Auto Recompile at Startup is already enabled"));
		GConfig->SetString(TEXT("/Script/Cognitive3D.Cognitive3DSettings"), TEXT("RestartAfterSetup"), TEXT("True"), C3DSettingsPath);
	}
	GConfig->Flush(false, C3DSettingsPath);

	if (Choice != EAppReturnType::Yes)
	{
		return;
	}

	FUnrealEdMisc::Get().RestartEditor(/*bWarn=*/false);
}


END_SLATE_FUNCTION_BUILD_OPTIMIZATION