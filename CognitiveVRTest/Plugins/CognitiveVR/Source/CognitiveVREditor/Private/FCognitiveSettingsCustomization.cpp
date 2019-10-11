
#include "CognitiveVREditorPrivatePCH.h"
#include "CognitiveEditorTools.h"
#include "FCognitiveSettingsCustomization.h"

#define LOCTEXT_NAMESPACE "BaseToolEditor"

//deals with all the interface stuff in the editor preferences
//includes any details needed to make the ui work

//TSharedRef<FCognitiveTools> ToolsInstance;
FCognitiveSettingsCustomization* FCognitiveSettingsCustomization::CognitiveSettingsCustomizationInstance;

TSharedRef<IDetailCustomization> FCognitiveSettingsCustomization::MakeInstance()
{
	CognitiveSettingsCustomizationInstance = new FCognitiveSettingsCustomization;
	return MakeShareable(CognitiveSettingsCustomizationInstance);
}

TArray<TSharedPtr<cognitivevrapi::FEditorSceneData>> GetSceneData()
{
	return FCognitiveEditorTools::GetInstance()->GetSceneData();
}

TArray<TSharedPtr<cognitivevrapi::FDynamicData>> FCognitiveSettingsCustomization::GetSceneDynamics()
{
	return FCognitiveEditorTools::GetInstance()->GetSceneDynamics();
}

/*TArray<TSharedPtr<FString>> GetSubDirectoryNames()
{
	return FCognitiveEditorTools::GetInstance()->GetSubDirectoryNames();
}*/

void FCognitiveSettingsCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	/*TSet<UClass*> Classes;

	TArray<TWeakObjectPtr<UObject>> ObjectsBeingCustomized;
	DetailBuilder.GetObjectsBeingCustomized(ObjectsBeingCustomized);

	DetailLayoutPtr = &DetailBuilder;

	UClass* Class = NULL;

	for (auto WeakObject : ObjectsBeingCustomized)
	{
		if (UObject* instance = WeakObject.Get())
		{
			Class = instance->GetClass();
			break;
		}
	}*/

	/*IDetailCategoryBuilder& SettingsCategory = DetailBuilder.EditCategory(TEXT("Export Settings"));

	FCognitiveEditorTools::GetInstance()->MinPolygonProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UCognitiveVRSettings, MinPolygons));
	FCognitiveEditorTools::GetInstance()->MaxPolygonProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UCognitiveVRSettings, MaxPolygons));
	FCognitiveEditorTools::GetInstance()->StaticOnlyProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UCognitiveVRSettings, staticOnly));
	FCognitiveEditorTools::GetInstance()->MinSizeProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UCognitiveVRSettings, MinimumSize));
	FCognitiveEditorTools::GetInstance()->MaxSizeProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UCognitiveVRSettings, MaximumSize));
	FCognitiveEditorTools::GetInstance()->TextureResizeProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UCognitiveVRSettings, TextureResizeFactor));
	FCognitiveEditorTools::GetInstance()->ExcludeMeshProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UCognitiveVRSettings, ExcludeMeshes));*/

	// Create a commands category
	IDetailCategoryBuilder& LoginCategory = DetailBuilder.EditCategory(TEXT("Account"),FText::GetEmpty(),ECategoryPriority::Important);

	//simple spacer
	/*LoginCategory.AddCustomRow(FText::FromString("Commands"))
	.ValueContent()
	.HAlign(HAlign_Fill)
	[
		SNew(SHorizontalBox)
		.Visibility(EVisibility::Hidden)
	];*/
	

	
	LoginCategory.AddCustomRow(FText::FromString("Dev Key"))
	.ValueContent()
	.HAlign(HAlign_Fill)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.MaxWidth(96)
		[
			SNew(STextBlock)
			.Text(FText::FromString("Developer Key"))
		]

		+ SHorizontalBox::Slot()
		.MaxWidth(128)
		[
			SNew(SEditableTextBox)
			.MinDesiredWidth(128)
			.Text_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::GetDeveloperKey)
			.OnTextChanged_Raw(FCognitiveEditorTools::GetInstance(),&FCognitiveEditorTools::OnDeveloperKeyChanged)
		]
	];

	LoginCategory.AddCustomRow(FText::FromString("API Key"))
	.ValueContent()
	.HAlign(HAlign_Fill)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.MaxWidth(96)
		[
			SNew(STextBlock)
			.Text(FText::FromString("API Key"))
		]

		+ SHorizontalBox::Slot()
		.MaxWidth(128)
		[
			SNew(SEditableTextBox)
			.MinDesiredWidth(128)
			.Text_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::GetAPIKey)
			.OnTextChanged_Raw(FCognitiveEditorTools::GetInstance(),&FCognitiveEditorTools::OnAPIKeyChanged)
		]
	];

	LoginCategory.AddCustomRow(FText::FromString("Save"))
	.ValueContent()
	.MinDesiredWidth(256)
	[
		SNew(SButton)
		.Text(FText::FromString("Save"))
		//.IsEnabled(this,&FCognitiveTools::HasSelectedValidProduct)
		.OnClicked_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::SaveAPIDeveloperKeysToFile)
	];

	/*LoginCategory.AddCustomRow(FText::FromString("Commands"))
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.HAlign(EHorizontalAlignment::HAlign_Center)
		[
			SNew(STextBlock)
			.Visibility_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::ConfigFileChangedVisibility)
			.ColorAndOpacity(FLinearColor::Yellow)
			.Text(FText::FromString("Config files changed. Data displayed below may be incorrect until you restart Unreal Editor!"))
		]
	];*/

	IDetailCategoryBuilder& ExportedSceneData = DetailBuilder.EditCategory(TEXT("Exported Scene Data"));

	ExportedSceneData.AddCustomRow(FText::FromString("Scene Management Buttons"))
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.HAlign(EHorizontalAlignment::HAlign_Center)
		[
			SNew(SButton)
			.HAlign(EHorizontalAlignment::HAlign_Center)
			.IsEnabled_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::HasDeveloperKey)
			.Text(FText::FromString("Get Latest Scene Version Data"))
			.ToolTip(SNew(SToolTip).Text(LOCTEXT("get scene data tip", "This will get the latest scene version data from Scene Explorer and saves it to your config files. Must restart Unreal Editor to see the changes in your config files here")))
			.OnClicked_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::ButtonCurrentSceneVersionRequest)
		]
	];

	ExportedSceneData.AddCustomRow(FText::FromString("Scene Management List"))
	[
			SNew(SBox)
			.MaxDesiredHeight(200)
			[
			SNew(SListView<TSharedPtr<cognitivevrapi::FEditorSceneData>>)
			.ItemHeight(24)
			.ListItemsSource(&FCognitiveEditorTools::GetInstance()->SceneData) //BIND
			.OnGenerateRow(this, &FCognitiveSettingsCustomization::OnGenerateWorkspaceRow)
			.HeaderRow(
				SNew(SHeaderRow)
				+ SHeaderRow::Column("name")
				.FillWidth(1)
				[
					SNew(SRichTextBlock)
					.DecoratorStyleSet(&FEditorStyle::Get())
				.Text(FText::FromString("<RichTextBlock.BoldHighlight>Name</>"))
				]

				+ SHeaderRow::Column("id")
				.FillWidth(1)
				[
					SNew(SRichTextBlock)
					.DecoratorStyleSet(&FEditorStyle::Get())
					.Text(FText::FromString("<RichTextBlock.BoldHighlight>Id</>"))
				]

				+ SHeaderRow::Column("version number")
				.FillWidth(0.3)
				[
					SNew(SRichTextBlock)
					.DecoratorStyleSet(&FEditorStyle::Get())
					.Text(FText::FromString("<RichTextBlock.BoldHighlight>Version Number</>"))
				]

				+ SHeaderRow::Column("version id")
				.FillWidth(0.3)
				[
					SNew(SRichTextBlock)
					.DecoratorStyleSet(&FEditorStyle::Get())
					.Text(FText::FromString("<RichTextBlock.BoldHighlight>Version Id</>"))
				]

				+ SHeaderRow::Column("open")
				[
					SNew(SRichTextBlock)
					.DecoratorStyleSet(&FEditorStyle::Get())
					.Text(FText::FromString("<RichTextBlock.BoldHighlight>Scene Explorer</>"))
				]
			)
		]
	];

	IDetailCategoryBuilder& DynamicWorkflow = DetailBuilder.EditCategory(TEXT("Dynamic Mesh Upload"));

	DynamicWorkflow.InitiallyCollapsed(true);

	DynamicWorkflow.AddCustomRow(FText::FromString("Paths"))
	[
		SNew(SVerticalBox)
		+SVerticalBox::Slot()
		.MaxHeight(17)
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.MaxWidth(200)
			[
				SNew(SBox)
				.HeightOverride(17)
				[
					SNew(STextBlock)
					.IsEnabled_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::HasDeveloperKey)
					.Text(FText::FromString("Path to Blender.exe"))
					//.OnClicked_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::Select_Blender)
				]
			]
			+ SHorizontalBox::Slot()
				//.MaxWidth(400)
				.Padding(1)
			[
				SNew(SBox)
				.HeightOverride(17)
				[
					SNew(SEditableTextBox)
					.Text_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::GetBlenderPath)
					.OnTextChanged(this, &FCognitiveSettingsCustomization::OnBlenderPathChanged)
					//SNew(STextBlock)
					//
				]
			]
			+SHorizontalBox::Slot()
			.MaxWidth(17)
			[
				SNew(SBox)
				.HeightOverride(17)
				.WidthOverride(17)
				[
					SNew(SButton)
					//PickerWidget = SAssignNew(BrowseButton, SButton)
					.ButtonStyle(FEditorStyle::Get(), "HoverHintOnly")
					.ToolTipText(LOCTEXT("FolderButtonToolTipText", "Choose a directory from this computer"))
					//.OnClicked(FOnClicked::CreateSP(this, &FDirectoryPathStructCustomization::OnPickDirectory, PathProperty.ToSharedRef(), bRelativeToGameContentDir, bUseRelativePath, bLongPackageName))
					.OnClicked_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::Select_Blender)
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
			//+SHorizontalBox::Slot()
			//.MaxWidth(17)
			//[
			//	SNew(SBox)
			//	.HeightOverride(17)
			//	.WidthOverride(17)
			//	.Visibility_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::BlenderValidVisibility)
			//	[
			//		SNew(SImage)
			//		.Image(FEditorStyle::GetBrush("SettingsEditor.WarningIcon"))
			//	]
			//]
			//+SHorizontalBox::Slot()
			//.MaxWidth(17)
			//[
			//	SNew(SBox)
			//	.HeightOverride(17)
			//	.WidthOverride(17)
			//	.Visibility_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::BlenderInvalidVisibility)
			//	[
			//		SNew(SImage)
			//		.Image(FEditorStyle::GetBrush("SettingsEditor.WarningIcon"))
			//		.ColorAndOpacity(FLinearColor::Black)
			//	]
			//]
		]

		+SVerticalBox::Slot()
		.MaxHeight(17)
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.MaxWidth(200)
			[
				SNew(SBox)
				.HeightOverride(17)
				[
					SNew(STextBlock)
					.IsEnabled_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::HasDeveloperKey)
					.Text(FText::FromString("Path to Export Directory"))
					//.OnClicked_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::Select_Blender)
				]
			]
			+ SHorizontalBox::Slot()
				//.MaxWidth(400)
				.Padding(1)
			[
				SNew(SBox)
				.HeightOverride(17)
				[
					SNew(SEditableTextBox)
					.Text_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::GetBaseExportDirectoryDisplay)
					.OnTextChanged(this, &FCognitiveSettingsCustomization::OnExportPathChanged)
					//SNew(STextBlock)
					//
				]
			]
			+SHorizontalBox::Slot()
			.MaxWidth(17)
			[
				SNew(SBox)
				.HeightOverride(17)
				.WidthOverride(17)
				[
					SNew(SButton)
					//PickerWidget = SAssignNew(BrowseButton, SButton)
					.ButtonStyle(FEditorStyle::Get(), "HoverHintOnly")
					.ToolTipText(LOCTEXT("FolderButtonToolTipText", "Choose a directory from this computer"))
					//.OnClicked(FOnClicked::CreateSP(this, &FDirectoryPathStructCustomization::OnPickDirectory, PathProperty.ToSharedRef(), bRelativeToGameContentDir, bUseRelativePath, bLongPackageName))
					.OnClicked(this, &FCognitiveSettingsCustomization::SelectAndRefreshExportDirectory)
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

		+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(0.0f, 10.0f, 0.0f, 10.0f))
			[
				SNew(SSeparator)
			]
	];

	DynamicWorkflow.AddCustomRow(FText::FromString("Dynamic Object Upload Process"))
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.Padding(4.0f, 0.0f)
			[
				SNew(SVerticalBox)
				/*+ SVerticalBox::Slot()
				.HAlign(EHorizontalAlignment::HAlign_Center)
				[
					SNew(STextBlock)
					.Text(FText::FromString("Export"))
				]*/
				+ SVerticalBox::Slot()
				.MaxHeight(32)
				[
					SNew(SButton)
					.IsEnabled_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::HasFoundBlenderDynamicExportDirSelection)
					.Text(FText::FromString("Export Selected Dynamic Object Meshes"))
					.OnClicked_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::ExportSelectedDynamics)
				]
				+ SVerticalBox::Slot()
				.MaxHeight(32)
				[
					SNew(SButton)
					.IsEnabled_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::HasFoundBlenderAndDynamicExportDir)
					.Text(FText::FromString("Export All Dynamic Object Meshes"))
					.OnClicked_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::ExportAllDynamics)
				]
			]
		+SHorizontalBox::Slot()
		.Padding(FMargin(5.0f, 0.0f, 5.0f, 0.0f))
		.AutoWidth()
		.MaxWidth(2)
		[
			SNew(SSeparator)
			.Orientation(EOrientation::Orient_Vertical)
		]
		+ SHorizontalBox::Slot()
		.Padding(4.0f, 0.0f)
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SButton)
				.IsEnabled_Raw(FCognitiveEditorTools::GetInstance(),&FCognitiveEditorTools::HasSetDynamicExportDirectory)
				.Text(FText::FromString("Refresh Dynamic Directories"))
				.OnClicked(this, &FCognitiveSettingsCustomization::CopyDynamicSubDirectories)
			]
			+SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.IsEnabled_Raw(FCognitiveEditorTools::GetInstance(),&FCognitiveEditorTools::HasSetDynamicExportDirectory)
				.Text(FText::FromString("Meshes Exported:"))
			]
			+ SVerticalBox::Slot()
			.Padding(4.0f, 2.0f)
			.MaxHeight(2)
			[
				SNew(SSeparator)
				.Orientation(EOrientation::Orient_Horizontal)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBox)
				.HeightOverride(200)
				[
					SAssignNew(SubDirectoryListWidget,SFStringListWidget)
					//.Items(DynamicSubDirectories) //BIND
				]
			]
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.MaxWidth(2)
		.Padding(FMargin(5.0f, 0.0f, 5.0f, 0.0f))
		[
			SNew(SSeparator)
			.Orientation(EOrientation::Orient_Vertical)
		]
		+ SHorizontalBox::Slot()
		.Padding(4.0f, 0.0f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.MaxHeight(64)
			[
				SNew(SButton)
				.IsEnabled_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::HasSetDynamicExportDirectoryHasSceneId)
				.OnClicked_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::UploadDynamics)
				[
					SNew(STextBlock)
					.AutoWrapText(true)
					.Text_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::GetDynamicObjectUploadText)
				]
			]
		]
	];


	IDetailCategoryBuilder& DynamicManifestWorkflow = DetailBuilder.EditCategory(TEXT("Dynamic Objects"));

	DynamicManifestWorkflow.InitiallyCollapsed(true);

	DynamicManifestWorkflow.AddCustomRow(FText::FromString("Dynamic Object Summary"))
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			[
				SNew(SVerticalBox)
				+SVerticalBox::Slot()
				.AutoHeight()
				.Padding(FMargin(64, 24, 64, 24))
				[
					SNew(SBorder)
					//.Visibility_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::GetDuplicateDyanmicObjectVisibility)
					//.BorderBackgroundColor(FLinearColor::Red)
					.BorderImage(FEditorStyle::GetBrush("ToolPanel.LightGroupBorder"))
					.Padding(8.0f)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						[
							SNew(SImage)
							.Visibility_Raw(FCognitiveEditorTools::GetInstance(),&FCognitiveEditorTools::GetDuplicateDyanmicObjectVisibility)
							.Image(FEditorStyle::GetBrush("SettingsEditor.WarningIcon"))
						]
							// Notice
						+SHorizontalBox::Slot()
						.FillWidth(1.5f)
						.Padding(16.0f, 0.0f)
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.ColorAndOpacity(FLinearColor::Black)
							//.ShadowColorAndOpacity(FLinearColor::Black)
							//.ShadowOffset(FVector2D::UnitVector)
							.AutoWrapText(true)
							.Text(FText::FromString("Dynamic Objects must have a valid Mesh Name\nTo have data aggregated, Dynamic Objects must have a Unique Id"))
						]
						+SHorizontalBox::Slot()
						[
							SNew(SButton)
							.OnClicked(this,&FCognitiveSettingsCustomization::ValidateAndRefresh)
							[
								SNew(STextBlock)
								.Justification(ETextJustify::Center)
								.AutoWrapText(true)
								.Text(FText::FromString("Validate Mesh Names and Unique Ids"))
							]
						]
					]
				]
				+SVerticalBox::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Fill)
				[
					SNew(SBox)
					.HeightOverride(64)
					[
						SNew(SButton)
						.IsEnabled_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::HasDeveloperKey)
						.OnClicked_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::UploadDynamicsManifest)
						[
							SNew(STextBlock)
							.Justification(ETextJustify::Center)
							.Text(FText::FromString("Upload Dynamic Objects Ids to SceneExplorer for Aggregation"))
						]
					]
				]

				+SVerticalBox::Slot()
				[
					SNew(SBox)
					.HeightOverride(16)
				]

				//+ SVerticalBox::Slot()
				//.Padding(4.0f, 2.0f)
				//.MaxHeight(2)
				//[
				//	SNew(SSeparator)
				//	.Orientation(EOrientation::Orient_Horizontal)
				//]

				+SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SButton)
						.IsEnabled_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::HasDeveloperKey)
						.Text(FText::FromString("Refresh"))
						.OnClicked(this, &FCognitiveSettingsCustomization::RefreshDisplayDynamicObjectsCountInScene)
					]
					+SHorizontalBox::Slot()
					[
						SNew(STextBlock)
						.Justification(ETextJustify::Center)
						.Text_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::DisplayDynamicObjectsCountInScene)
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SBox)
					.HeightOverride(300)
					[
						SAssignNew(SceneDynamicObjectList,SDynamicObjectListWidget)
						//.CognitiveTools(this)
						.Items(GetSceneDynamics()) //BIND
					]
				]
			]
		];


	FCognitiveEditorTools::GetInstance()->ReadSceneDataFromFile();
	FCognitiveEditorTools::GetInstance()->RefreshDisplayDynamicObjectsCountInScene();
	FCognitiveEditorTools::GetInstance()->SearchForBlender();
	FCognitiveEditorTools::GetInstance()->CurrentSceneVersionRequest();

	CopyDynamicSubDirectories();

	FString EngineIni = FPaths::Combine(*(FPaths::ProjectDir()), TEXT("Config/DefaultEngine.ini"));
	FString EditorIni = FPaths::Combine(*(FPaths::ProjectDir()), TEXT("Config/DefaultEditor.ini"));

	GConfig->GetString(TEXT("Analytics"), TEXT("ApiKey"), FCognitiveEditorTools::GetInstance()->APIKey, EngineIni);
	GConfig->GetString(TEXT("Analytics"), TEXT("DeveloperKey"), FAnalyticsCognitiveVR::Get().DeveloperKey, EditorIni);
}

FReply FCognitiveSettingsCustomization::SelectAndRefreshExportDirectory()
{
	FCognitiveEditorTools::GetInstance()->SelectBaseExportDirectory();
	CopyDynamicSubDirectories();
	return FReply::Handled();
}

TSharedRef<ITableRow> FCognitiveSettingsCustomization::OnGenerateWorkspaceRow(TSharedPtr<cognitivevrapi::FEditorSceneData> InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	return
		SNew(SComboRow< TSharedPtr<cognitivevrapi::FEditorSceneData> >, OwnerTable)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1)
			.Padding(2.0f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(InItem->Name))
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1)
			.Padding(2.0f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(InItem->Id))
			]
			+ SHorizontalBox::Slot()
			.FillWidth(0.3)
			.Padding(2.0f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(FString::FromInt(InItem->VersionNumber)))
			]
			+ SHorizontalBox::Slot()
			.FillWidth(0.3)
			.Padding(2.0f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(FString::FromInt(InItem->VersionId)))
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1)
			.Padding(2.0f)
			[
				SNew(SButton)
				.Text(FText::FromString("Open in Browser..."))
				.OnClicked_Raw(FCognitiveEditorTools::GetInstance(),&FCognitiveEditorTools::OpenSceneInBrowser,InItem->Id)
			]
		];
}

TSharedRef<ITableRow> FCognitiveSettingsCustomization::OnGenerateDynamicRow(TSharedPtr<cognitivevrapi::FDynamicData> InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	return
		SNew(SComboRow< TSharedPtr<cognitivevrapi::FDynamicData> >, OwnerTable)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1)
			.Padding(2.0f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(InItem->Name))
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1)
			.Padding(2.0f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(InItem->MeshName))
			]
			+ SHorizontalBox::Slot()
			.FillWidth(0.3)
			.Padding(2.0f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(InItem->Id))
			]
		];
}

FReply FCognitiveSettingsCustomization::CopyDynamicSubDirectories()
{
	if (!FCognitiveEditorTools::GetInstance()->HasSetExportDirectory())
	{
		return FReply::Handled();
	}

	FCognitiveEditorTools::GetInstance()->RefreshDynamicSubDirectory();

	SubDirectoryListWidget->Items = FCognitiveEditorTools::GetInstance()->GetSubDirectoryNames();

	GLog->Log("this many dynamics " + FString::FromInt(FCognitiveEditorTools::GetInstance()->GetSubDirectoryNames().Num()));

	SubDirectoryListWidget->RefreshList();

	return FReply::Handled();
}

void FCognitiveSettingsCustomization::OnCommitedExportMinimumSize(float InNewValue, ETextCommit::Type CommitType)
{
	FCognitiveEditorTools::GetInstance()->MinimumSize = InNewValue;
}

void FCognitiveSettingsCustomization::OnChangedExportMinimumSize(float InNewValue)
{
	FCognitiveEditorTools::GetInstance()->MinimumSize = InNewValue;
}

void FCognitiveSettingsCustomization::OnCommitedExportMaximumSize(float InNewValue, ETextCommit::Type CommitType)
{
	FCognitiveEditorTools::GetInstance()->MaximumSize = InNewValue;
}

void FCognitiveSettingsCustomization::OnChangedExportMaximumSize(float InNewValue)
{
	FCognitiveEditorTools::GetInstance()->MaximumSize = InNewValue;
}

void FCognitiveSettingsCustomization::OnChangedExcludeMesh(const FText& InNewValue)
{
	FCognitiveEditorTools::GetInstance()->ExcludeMeshes = InNewValue.ToString();
}

void FCognitiveSettingsCustomization::OnCheckStateChangedStaticOnly(const bool& InNewValue)
{
	FCognitiveEditorTools::GetInstance()->StaticOnly = InNewValue;
}

void FCognitiveSettingsCustomization::OnChangedExportMinPolygon(int32 InNewValue)
{
	FCognitiveEditorTools::GetInstance()->MinPolygon = InNewValue;
}
void FCognitiveSettingsCustomization::OnCommitedExportMinPolygon(int32 InNewValue, ETextCommit::Type CommitType)
{
	FCognitiveEditorTools::GetInstance()->MinPolygon = InNewValue;
}

void FCognitiveSettingsCustomization::OnChangedExportMaxPolygon(int32 InNewValue)
{
	FCognitiveEditorTools::GetInstance()->MaxPolygon = InNewValue;
}
void FCognitiveSettingsCustomization::OnCommitedExportMaxPolygon(int32 InNewValue, ETextCommit::Type CommitType)
{
	FCognitiveEditorTools::GetInstance()->MaxPolygon = InNewValue;
}

void FCognitiveSettingsCustomization::OnChangedExportTextureRefactor(int32 InNewValue)
{
	FCognitiveEditorTools::GetInstance()->TextureRefactor = InNewValue;
}
void FCognitiveSettingsCustomization::OnCommitedExportTextureRefactor(int32 InNewValue, ETextCommit::Type CommitType)
{
	FCognitiveEditorTools::GetInstance()->TextureRefactor = InNewValue;
}

FReply FCognitiveSettingsCustomization::RefreshDisplayDynamicObjectsCountInScene()
{
	FCognitiveEditorTools::GetInstance()->RefreshDisplayDynamicObjectsCountInScene();
	SceneDynamicObjectList->RefreshList();
	return FReply::Handled();
}

void FCognitiveSettingsCustomization::OnBlenderPathChanged(const FText& Text)
{
	FCognitiveEditorTools::GetInstance()->BlenderPath = Text.ToString();
}

void FCognitiveSettingsCustomization::OnExportPathChanged(const FText& Text)
{
	FCognitiveEditorTools::GetInstance()->BaseExportDirectory = Text.ToString();
}

FReply FCognitiveSettingsCustomization::ValidateAndRefresh()
{
	FCognitiveEditorTools::GetInstance()->SetUniqueDynamicIds();
	FCognitiveEditorTools::GetInstance()->RefreshDisplayDynamicObjectsCountInScene();
	SceneDynamicObjectList->RefreshList();
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE