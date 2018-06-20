
#include "CognitiveVREditorPrivatePCH.h"
#include "CognitiveEditorTools.h"
#include "FCognitiveSettingsCustomization.h"

#define LOCTEXT_NAMESPACE "BaseToolEditor"

//deals with all the interface stuff in the editor preferences
//includes any details needed to make the ui work

//TSharedRef<FCognitiveTools> ToolsInstance;
FCognitiveSettingsCustomization* CognitiveSettingsCustomizationInstance;

TSharedRef<IDetailCustomization> FCognitiveSettingsCustomization::MakeInstance()
{
	CognitiveSettingsCustomizationInstance = new FCognitiveSettingsCustomization;
	return MakeShareable(CognitiveSettingsCustomizationInstance);
}

TArray<TSharedPtr<FEditorSceneData>> GetSceneData()
{
	return FCognitiveEditorTools::GetInstance()->GetSceneData();
}

TArray<TSharedPtr<FDynamicData>> FCognitiveSettingsCustomization::GetSceneDynamics()
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
	

	
	LoginCategory.AddCustomRow(FText::FromString("Commands"))
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

	LoginCategory.AddCustomRow(FText::FromString("Commands"))
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

	LoginCategory.AddCustomRow(FText::FromString("Commands"))
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

	ExportedSceneData.AddCustomRow(FText::FromString("Commands"))
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

	ExportedSceneData.AddCustomRow(FText::FromString("Commands"))
	[
			SNew(SBox)
			.MaxDesiredHeight(200)
			[
			SNew(SListView<TSharedPtr<FEditorSceneData>>)
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

	IDetailCategoryBuilder& DynamicWorkflow = DetailBuilder.EditCategory(TEXT("Dynamic Upload Workflow"));

	DynamicWorkflow.InitiallyCollapsed(true);

	DynamicWorkflow.AddCustomRow(FText::FromString("Commands"))
	[
		SNew(SVerticalBox)
		+SVerticalBox::Slot()
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SCheckBox)
				.IsEnabled(false)
				.IsChecked_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::HasFoundBlenderCheckbox)
			]
			+SHorizontalBox::Slot()
			.MaxWidth(256)
			[
				SNew(SButton)
				.IsEnabled_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::HasDeveloperKey)
				.Text(FText::FromString("Select Blender.exe"))
				.OnClicked_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::Select_Blender)
			]
			+ SHorizontalBox::Slot()
			[
				SNew(STextBlock)
				.Text_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::GetBlenderPath)
			]
		]

		+SVerticalBox::Slot()
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.MaxWidth(256)
			[
				SNew(SButton)
				.IsEnabled_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::HasFoundBlender)
				.Text(FText::FromString("Select Export Directory"))
				.OnClicked(this, &FCognitiveSettingsCustomization::SelectAndRefreshExportDirectory)
			]
			+ SHorizontalBox::Slot()
			[
				SNew(STextBlock)
				.Text_Raw(FCognitiveEditorTools::GetInstance(),&FCognitiveEditorTools::GetBaseExportDirectoryDisplay)
			]
		]

		+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(0.0f, 10.0f, 0.0f, 10.0f))
			[
				SNew(SSeparator)
			]
	];

	DynamicWorkflow.AddCustomRow(FText::FromString("Commands"))
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
					.Text(FText::FromString("Export Selected Dynamic Objects"))
					.OnClicked_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::ExportSelectedDynamics)
				]
				+ SVerticalBox::Slot()
				.MaxHeight(32)
				[
					SNew(SButton)
					.IsEnabled_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::HasFoundBlenderAndDynamicExportDir)
					.Text(FText::FromString("Export All Dynamic Object Meshes"))
					.OnClicked_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::ExportDynamics)
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
				/*+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 0, 0, 4)
				.HAlign(EHorizontalAlignment::HAlign_Center)
				[
					SNew(STextBlock)
					.Text(FText::FromString("Organize Meshes"))
				]*/
				/*+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SButton)
					.IsEnabled_Raw(FCognitiveEditorTools::GetInstance(),&FCognitiveEditorTools::HasDeveloperKey)
					.Text(FText::FromString("Select Dynamic Mesh Directory"))
					.OnClicked_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::SelectDynamicsDirectory)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::GetDynamicsExportDirectoryDisplay)
				]*/
				+SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SButton)
					.IsEnabled_Raw(FCognitiveEditorTools::GetInstance(),&FCognitiveEditorTools::HasSetDynamicExportDirectory)
					.Text(FText::FromString("Refresh Dynamic Directories"))
					.OnClicked(this, &FCognitiveSettingsCustomization::CopyDynamicSubDirectories)
				]
				+ SVerticalBox::Slot()
				[
					SNew(SBox)
					.HeightOverride(100)
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
				/*+ SVerticalBox::Slot()
				.Padding(0, 0, 0, 4)
				.HAlign(EHorizontalAlignment::HAlign_Center)
				[
					SNew(STextBlock)
					.Text(FText::FromString("Upload"))
				]*/
				+ SVerticalBox::Slot()
				.MaxHeight(32)
				[
					SNew(SButton)
					.IsEnabled_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::HasSetDynamicExportDirectoryHasSceneId)
					.Text_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::GetDynamicObjectUploadText)
					.OnClicked_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::UploadDynamics)
				]
			]
		];


	IDetailCategoryBuilder& DynamicManifestWorkflow = DetailBuilder.EditCategory(TEXT("Dynamic Objects"));

	DynamicManifestWorkflow.InitiallyCollapsed(true);

	DynamicManifestWorkflow.AddCustomRow(FText::FromString("Commands"))
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			[
				SNew(SVerticalBox)
				+SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot()
					[
						SNew(SButton)
						.IsEnabled_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::HasDeveloperKey)
						.Text(FText::FromString("Refresh"))
						.OnClicked(this, &FCognitiveSettingsCustomization::RefreshDisplayDynamicObjectsCountInScene)
					]
					+SHorizontalBox::Slot()
					[
						SNew(STextBlock)
						.Text_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::DisplayDynamicObjectsCountInScene)
					]
				]

				+SVerticalBox::Slot()
				.AutoHeight()
				.Padding(FMargin(64, 24, 64, 24))
				[
					SNew(SBorder)
					.Visibility_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::GetDuplicateDyanmicObjectVisibility)
					.BorderBackgroundColor(FLinearColor::Red)
					.BorderImage(FEditorStyle::GetBrush("ToolPanel.LightGroupBorder"))
					.Padding(8.0f)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						[
							SNew(SImage)
							.Image(FEditorStyle::GetBrush("SettingsEditor.WarningIcon"))
						]
							// Notice
						+SHorizontalBox::Slot()
						.FillWidth(1.0f)
						.Padding(16.0f, 0.0f)
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.ColorAndOpacity(FLinearColor::White)
							.ShadowColorAndOpacity(FLinearColor::Black)
							.ShadowOffset(FVector2D::UnitVector)
							.Text(FText::FromString("Scene contains duplicate Dynamic Object Ids"))
						]
						+SHorizontalBox::Slot()
						[
							SNew(SButton)
							.OnClicked_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::SetUniqueDynamicIds)
							.Text(FText::FromString("Set Unique Dynamic Ids"))
						]
					]
				]

				/*+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(FMargin(24, 10, 24, 0))
				[
					SNew(STextBlock)
					.ColorAndOpacity(FLinearColor::Yellow)
					.Justification(ETextJustify::Center)
					.Visibility_Raw(FCognitiveEditorTools::GetInstance(),&FCognitiveEditorTools::GetDuplicateDyanmicObjectVisibility)
					.Text(FText::FromString("Scene contains some duplicate Dynamic Object Ids"))
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(FMargin(64,0,64,64))
				[
					SNew(SBox)
					.Visibility_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::GetDuplicateDyanmicObjectVisibility)
					.HeightOverride(32)
					[
						SNew(SButton)
						.IsEnabled_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::HasDeveloperKey)
						.Visibility_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::GetDuplicateDyanmicObjectVisibility)
						.Text(FText::FromString("Set Unique Dynamic Ids"))
						.OnClicked_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::SetUniqueDynamicIds)
						//TODO replace with something really eye-catching, like SPlatformSetupMessage
					]
				]*/
				+SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SButton)
					.IsEnabled_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::HasDeveloperKey)
					.ToolTip(SNew(SToolTip).Text_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::SendDynamicsToSceneExplorerTooltip))
					.Text(FText::FromString("Sync with Scene Explorer"))
					.OnClicked_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::UploadDynamicsManifest)
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 0, 0, 4)
				.HAlign(EHorizontalAlignment::HAlign_Center)
				[
					SNew(STextBlock)
					.Text(FText::FromString("Dynamic Objects In Scene"))
				]
				+ SVerticalBox::Slot()
				.FillHeight(1)
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
			/*+ SHorizontalBox::Slot()
			.Padding(4.0f, 0.0f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 0, 0, 4)
				.HAlign(EHorizontalAlignment::HAlign_Center)
				[
					SNew(STextBlock)
					.Text(FText::FromString("Dynamic Objects on SceneExplorer"))
				]
				+SVerticalBox::Slot()
				.FillHeight(1)
				[
					SNew(SBox)
					.HeightOverride(300)
					[
						SAssignNew(WebDynamicList,SDynamicObjectWebListWidget)
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::DisplayDynamicObjectsCountOnWeb)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					[
						SNew(SButton)
						.IsEnabled_Raw(FCognitiveEditorTools::GetInstance(),&FCognitiveEditorTools::HasDeveloperKey)
						.ToolTip(SNew(SToolTip).Text_Raw(FCognitiveEditorTools::GetInstance(),&FCognitiveEditorTools::GetDynamicsOnSceneExplorerTooltip))
						.Text(FText::FromString("Get Dynamics List from SceneExplorer"))
						.OnClicked_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::GetDynamicsManifest)
					]
					+ SHorizontalBox::Slot()
					[
						SNew(SButton)
						.IsEnabled_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::HasDeveloperKey)
						.ToolTip(SNew(SToolTip).Text_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::SendDynamicsToSceneExplorerTooltip))
						.Text(FText::FromString("Send Dynamics List to SceneExplorer"))
						.OnClicked_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::UploadDynamicsManifest)
					]
				]
			]*/
		];


	FCognitiveEditorTools::GetInstance()->ReadSceneDataFromFile();
	FCognitiveEditorTools::GetInstance()->RefreshDisplayDynamicObjectsCountInScene();
	FCognitiveEditorTools::GetInstance()->SearchForBlender();
	FCognitiveEditorTools::GetInstance()->CurrentSceneVersionRequest();

	CopyDynamicSubDirectories();

	FString EngineIni = FPaths::Combine(*(FPaths::GameDir()), TEXT("Config/DefaultEngine.ini"));
	FString EditorIni = FPaths::Combine(*(FPaths::GameDir()), TEXT("Config/DefaultEditor.ini"));

	GConfig->GetString(TEXT("Analytics"), TEXT("ApiKey"), FCognitiveEditorTools::GetInstance()->APIKey, EngineIni);
	GConfig->GetString(TEXT("Analytics"), TEXT("DeveloperKey"), FAnalyticsCognitiveVR::Get().DeveloperKey, EditorIni);
}

FReply FCognitiveSettingsCustomization::SelectAndRefreshExportDirectory()
{
	FCognitiveEditorTools::GetInstance()->SelectBaseExportDirectory();
	CopyDynamicSubDirectories();
	return FReply::Handled();
}

TSharedRef<ITableRow> FCognitiveSettingsCustomization::OnGenerateWorkspaceRow(TSharedPtr<FEditorSceneData> InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	return
		SNew(SComboRow< TSharedPtr<FEditorSceneData> >, OwnerTable)
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

TSharedRef<ITableRow> FCognitiveSettingsCustomization::OnGenerateDynamicRow(TSharedPtr<FDynamicData> InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	return
		SNew(SComboRow< TSharedPtr<FDynamicData> >, OwnerTable)
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

#undef LOCTEXT_NAMESPACE