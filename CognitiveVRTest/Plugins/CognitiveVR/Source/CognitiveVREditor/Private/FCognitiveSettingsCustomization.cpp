
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

TArray<TSharedPtr<FEditorSceneData>> GetSceneData()
{
	return FCognitiveEditorTools::GetInstance()->GetSceneData();
}

TArray<TSharedPtr<FDynamicData>> FCognitiveSettingsCustomization::GetSceneDynamics()
{
	return FCognitiveEditorTools::GetInstance()->GetSceneDynamics();
}

void FCognitiveSettingsCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	// Create a commands category
	IDetailCategoryBuilder& LoginCategory = DetailBuilder.EditCategory(TEXT("Account"),FText::GetEmpty(),ECategoryPriority::Important);
	
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

	LoginCategory.AddCustomRow(FText::FromString("Application Key"))
	.ValueContent()
	.HAlign(HAlign_Fill)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.MaxWidth(96)
		[
			SNew(STextBlock)
			.Text(FText::FromString("Application Key"))
		]

		+ SHorizontalBox::Slot()
		.MaxWidth(128)
		[
			SNew(SEditableTextBox)
			.MinDesiredWidth(128)
			.Text_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::GetApplicationKey)
			.OnTextChanged_Raw(FCognitiveEditorTools::GetInstance(),&FCognitiveEditorTools::OnApplicationKeyChanged)
		]
	];

	LoginCategory.AddCustomRow(FText::FromString("Attribution Key"))
	.ValueContent()
	.HAlign(HAlign_Fill)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.MaxWidth(96)
		[
			SNew(STextBlock)
			.Text(FText::FromString("Attribution Key"))
		]

		+ SHorizontalBox::Slot()
		.MaxWidth(128)
		[
			SNew(SEditableTextBox)
			.MinDesiredWidth(128)
			.Text_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::GetAttributionKey)
			.OnTextChanged_Raw(FCognitiveEditorTools::GetInstance(),&FCognitiveEditorTools::OnAttributionKeyChanged)
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

	LoginCategory.AddCustomRow(FText::FromString("Blender Path"))
		.ValueContent()
		.HAlign(HAlign_Fill)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
		.MaxWidth(96)
		[
			SNew(STextBlock)
			.Text(FText::FromString("Blender Path"))
		]

	+ SHorizontalBox::Slot()
		//.MaxWidth(128)
		[
			SNew(SEditableTextBox)
			.MinDesiredWidth(128)
		.Text_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::GetBlenderPath)
		.OnTextChanged_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::OnBlenderPathChanged)
		]
	];

	LoginCategory.AddCustomRow(FText::FromString("Export Path"))
		.ValueContent()
		.HAlign(HAlign_Fill)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
		.MaxWidth(96)
		[
			SNew(STextBlock)
			.Text(FText::FromString("Export Path"))
		]

	+ SHorizontalBox::Slot()
		//.MaxWidth(128)
		[
			SNew(SEditableTextBox)
			.MinDesiredWidth(128)
		.Text_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::GetBaseExportDirectoryDisplay)
		.OnTextChanged_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::OnExportPathChanged)
		]
	];

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


	FCognitiveEditorTools::GetInstance()->ReadSceneDataFromFile();
	FCognitiveEditorTools::GetInstance()->RefreshDisplayDynamicObjectsCountInScene();
	FCognitiveEditorTools::GetInstance()->CurrentSceneVersionRequest();

	FString EngineIni = FPaths::Combine(*(FPaths::ProjectDir()), TEXT("Config/DefaultEngine.ini"));
	FString EditorIni = FPaths::Combine(*(FPaths::ProjectDir()), TEXT("Config/DefaultEditor.ini"));

	GConfig->GetString(TEXT("Analytics"), TEXT("AttributionKey"), FCognitiveEditorTools::GetInstance()->AttributionKey, EngineIni);
	GConfig->GetString(TEXT("Analytics"), TEXT("ApiKey"), FCognitiveEditorTools::GetInstance()->ApplicationKey, EngineIni);
	GConfig->GetString(TEXT("Analytics"), TEXT("DeveloperKey"), FAnalyticsCognitiveVR::Get().DeveloperKey, EditorIni);
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

void FCognitiveSettingsCustomization::OnBlenderPathChanged(const FText& Text)
{
	FCognitiveEditorTools::GetInstance()->BlenderPath = Text.ToString();
}

void FCognitiveSettingsCustomization::OnExportPathChanged(const FText& Text)
{
	FCognitiveEditorTools::GetInstance()->BaseExportDirectory = Text.ToString();
}

#undef LOCTEXT_NAMESPACE