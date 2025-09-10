/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/

#include "CognitiveSettingsCustomization.h"
#include "CognitiveEditorData.h"

#define LOCTEXT_NAMESPACE "BaseToolEditor"

//deals with all the interface stuff in the editor preferences
//includes any details needed to make the ui work

//TSharedRef<FCognitiveTools> ToolsInstance;
ICognitiveSettingsCustomization* ICognitiveSettingsCustomization::CognitiveSettingsCustomizationInstance;

TSharedRef<IDetailCustomization> ICognitiveSettingsCustomization::MakeInstance()
{
	CognitiveSettingsCustomizationInstance = new ICognitiveSettingsCustomization;
	return MakeShareable(CognitiveSettingsCustomizationInstance);
}

TArray<TSharedPtr<FEditorSceneData>> GetSceneData()
{
	return FCognitiveEditorTools::GetInstance()->GetSceneData();
}

TArray<TSharedPtr<FDynamicData>> ICognitiveSettingsCustomization::GetSceneDynamics()
{
	return FCognitiveEditorTools::GetInstance()->GetSceneDynamics();
}

void ICognitiveSettingsCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
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
	+SHorizontalBox::Slot()
		.MaxWidth(17)
		[
			SNew(SBox)
			.HeightOverride(17)
			.WidthOverride(17)
			[
				SNew(SButton)
				//PickerWidget = SAssignNew(BrowseButton, SButton)
				.ButtonStyle(FCognitiveEditorTools::GetSlateStyle(), "HoverHintOnly")
				.ToolTipText(LOCTEXT("FolderButtonToolTipText", "Choose a directory from this computer"))
				.OnClicked_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::SelectBaseExportDirectory)
				.ContentPadding(2.0f)
				.ForegroundColor(FSlateColor::UseForeground())
				.IsFocusable(false)
				[
					SNew(SImage)
					.Image(FCognitiveEditorTools::GetBrush(FName("PropertyWindow.Button_Ellipsis")))
					.ColorAndOpacity(FSlateColor::UseForeground())
				]
			]
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
			.OnGenerateRow(this, &ICognitiveSettingsCustomization::OnGenerateWorkspaceRow)
			.HeaderRow(
				SNew(SHeaderRow)
				+ SHeaderRow::Column("name")
				.FillWidth(1.0f)
				.DefaultLabel(FText::FromString("Name"))

				+ SHeaderRow::Column("path")
				.FillWidth(2.0f)
				.DefaultLabel(FText::FromString("Path"))

				+ SHeaderRow::Column("id")
				.FillWidth(1.0f)
				.DefaultLabel(FText::FromString("Id"))

				+ SHeaderRow::Column("version number")
				.FillWidth(0.5f)
				.DefaultLabel(FText::FromString("Version Number"))

				+ SHeaderRow::Column("version id")
				.FillWidth(0.5f)
				.DefaultLabel(FText::FromString("Version Id"))

				+ SHeaderRow::Column("open")
				.FillWidth(0.8f)
				.DefaultLabel(FText::FromString("Scene Explorer"))
			)
		]
	];

	//gets data to autofill Account section
	FCognitiveEditorTools::GetInstance()->ReadSceneDataFromFile();
	FCognitiveEditorTools::GetInstance()->RefreshDisplayDynamicObjectsCountInScene();
	FCognitiveEditorTools::GetInstance()->CurrentSceneVersionRequest();

	FString C3DSettingsPath = FCognitiveEditorTools::GetInstance()->GetSettingsFilePath();
	FString C3DKeysPath = FCognitiveEditorTools::GetInstance()->GetKeysFilePath();
	// Explicitly load the custom config file into GConfig.
	GConfig->LoadFile(C3DSettingsPath);
	GConfig->LoadFile(C3DKeysPath);
	GConfig->GetString(TEXT("Analytics"), TEXT("AttributionKey"), FCognitiveEditorTools::GetInstance()->AttributionKey, C3DSettingsPath);
	GConfig->GetString(TEXT("Analytics"), TEXT("ApiKey"), FCognitiveEditorTools::GetInstance()->ApplicationKey, C3DSettingsPath);
	GConfig->GetString(TEXT("Analytics"), TEXT("DeveloperKey"), FCognitiveEditorTools::GetInstance()->DeveloperKey, C3DKeysPath);

	GConfig->Flush(false, C3DSettingsPath);
	GConfig->Flush(false, C3DKeysPath);

	if (FCognitiveEditorTools::GetInstance()->DeveloperKey.IsEmpty() || FCognitiveEditorTools::GetInstance()->ApplicationKey.IsEmpty()
		|| FCognitiveEditorTools::GetInstance()->BaseExportDirectory.IsEmpty())
	{
		FString EngineIni = FPaths::Combine(*(FPaths::ProjectDir()), TEXT("Config/DefaultEngine.ini"));
		FString EditorIni = FPaths::Combine(*(FPaths::ProjectDir()), TEXT("Config/DefaultEditor.ini"));
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 2
		const FString NormalizedEngineIni = GConfig->NormalizeConfigIniPath(EngineIni);
		const FString NormalizedEditorIni = GConfig->NormalizeConfigIniPath(EditorIni);

		GConfig->GetString(TEXT("Analytics"), TEXT("ApiKey"), FCognitiveEditorTools::GetInstance()->ApplicationKey, NormalizedEngineIni);
		GConfig->GetString(TEXT("Analytics"), TEXT("AttributionKey"), FCognitiveEditorTools::GetInstance()->AttributionKey, NormalizedEngineIni);
		GConfig->GetString(TEXT("Analytics"), TEXT("DeveloperKey"), FCognitiveEditorTools::GetInstance()->DeveloperKey, NormalizedEditorIni);
		GConfig->GetString(TEXT("Analytics"), TEXT("ExportPath"), FCognitiveEditorTools::GetInstance()->BaseExportDirectory, NormalizedEditorIni);

		//
		GConfig->GetString(TEXT("Analytics"), TEXT("ApiKey"), FCognitiveEditorTools::GetInstance()->ApplicationKey, GGameIni);
		GConfig->GetString(TEXT("Analytics"), TEXT("AttributionKey"), FCognitiveEditorTools::GetInstance()->AttributionKey, GGameIni);
		GConfig->GetString(TEXT("Analytics"), TEXT("DeveloperKey"), FCognitiveEditorTools::GetInstance()->DeveloperKey, GGameIni);
		GConfig->GetString(TEXT("Analytics"), TEXT("ExportPath"), FCognitiveEditorTools::GetInstance()->BaseExportDirectory, GGameIni);

		GConfig->Flush(false, GGameIni);

		GConfig->SetString(TEXT("Analytics"), TEXT("ApiKey"), *FCognitiveEditorTools::GetInstance()->ApplicationKey, C3DSettingsPath);
		GConfig->SetString(TEXT("Analytics"), TEXT("AttributionKey"), *FCognitiveEditorTools::GetInstance()->AttributionKey, C3DSettingsPath);
		GConfig->SetString(TEXT("Analytics"), TEXT("DeveloperKey"), *FCognitiveEditorTools::GetInstance()->DeveloperKey, C3DKeysPath);
		GConfig->SetString(TEXT("Analytics"), TEXT("ExportPath"), *FCognitiveEditorTools::GetInstance()->BaseExportDirectory, C3DSettingsPath);
		GConfig->Flush(false, C3DSettingsPath);
		GConfig->Flush(false, C3DKeysPath);
#else
		GConfig->GetString(TEXT("Analytics"), TEXT("ApiKey"), FCognitiveEditorTools::GetInstance()->ApplicationKey, EngineIni);
		GConfig->GetString(TEXT("Analytics"), TEXT("AttributionKey"), FCognitiveEditorTools::GetInstance()->AttributionKey, EngineIni);
		GConfig->GetString(TEXT("Analytics"), TEXT("DeveloperKey"), FCognitiveEditorTools::GetInstance()->DeveloperKey, EditorIni);
		GConfig->GetString(TEXT("Analytics"), TEXT("ExportPath"), FCognitiveEditorTools::GetInstance()->BaseExportDirectory, EditorIni);

		GConfig->SetString(TEXT("Analytics"), TEXT("ApiKey"), *FCognitiveEditorTools::GetInstance()->ApplicationKey, C3DSettingsPath);
		GConfig->SetString(TEXT("Analytics"), TEXT("AttributionKey"), *FCognitiveEditorTools::GetInstance()->AttributionKey, C3DSettingsPath);
		GConfig->SetString(TEXT("Analytics"), TEXT("DeveloperKey"), *FCognitiveEditorTools::GetInstance()->DeveloperKey, C3DKeysPath);
		GConfig->SetString(TEXT("Analytics"), TEXT("ExportPath"), *FCognitiveEditorTools::GetInstance()->BaseExportDirectory, C3DSettingsPath);
		GConfig->Flush(false, C3DSettingsPath);
		GConfig->Flush(false, C3DKeysPath);
#endif // ENGINE_MAJOR_VERSION == 4
	}

	//section to check third party SDKs active in the runtime build.cs file
	IDetailCategoryBuilder& ThirdPartyData = DetailBuilder.EditCategory(TEXT("Active third party SDKs"));

	//populate ThirdPartySDKData array using this function call to use later in the list
	FCognitiveEditorTools::GetInstance()->ReadThirdPartySDKData();

	ThirdPartyData.AddCustomRow(FText::FromString("Currently active third party SDKs"))
	[
		SNew(SBox)
		.HAlign(EHorizontalAlignment::HAlign_Center)
		[
			SNew(SListView<TSharedPtr<FString>>)
			.ItemHeight(24)
			.ListItemsSource(&FCognitiveEditorTools::GetInstance()->ThirdPartySDKData)
			.OnGenerateRow(this, &ICognitiveSettingsCustomization::OnGenerateSDKListRow)
		]
	];
}

TSharedRef<ITableRow> ICognitiveSettingsCustomization::OnGenerateWorkspaceRow(TSharedPtr<FEditorSceneData> InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	class SSceneSettingsTableRow : public SMultiColumnTableRow<TSharedPtr<FEditorSceneData>>
	{
	public:
		SLATE_BEGIN_ARGS(SSceneSettingsTableRow) {}
			SLATE_ARGUMENT(TSharedPtr<FEditorSceneData>, SceneData)
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView)
		{
			SceneData = InArgs._SceneData;
			SMultiColumnTableRow<TSharedPtr<FEditorSceneData>>::Construct(
				FSuperRowType::FArguments()
				.Padding(1.0f),
				InOwnerTableView
			);
		}

		virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override
		{
			if (ColumnName == "name")
			{
				return SNew(STextBlock)
					.Text(FText::FromString(SceneData->Name))
					.Margin(FMargin(4, 0));
			}
			else if (ColumnName == "path")
			{
				return SNew(STextBlock)
					.Text(FText::FromString(SceneData->Path))
					.Margin(FMargin(4, 0));
			}
			else if (ColumnName == "id")
			{
				return SNew(STextBlock)
					.Text(FText::FromString(SceneData->Id))
					.Margin(FMargin(4, 0));
			}
			else if (ColumnName == "version number")
			{
				return SNew(STextBlock)
					.Text(FText::FromString(FString::FromInt(SceneData->VersionNumber)))
					.Margin(FMargin(4, 0));
			}
			else if (ColumnName == "version id")
			{
				return SNew(STextBlock)
					.Text(FText::FromString(FString::FromInt(SceneData->VersionId)))
					.Margin(FMargin(4, 0));
			}
			else if (ColumnName == "open")
			{
				return SNew(SButton)
					.Text(FText::FromString("Open in Browser..."))
					.OnClicked_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::OpenSceneInBrowser, SceneData->Id)
					.HAlign(HAlign_Center);
			}

			return SNullWidget::NullWidget;
		}

	private:
		TSharedPtr<FEditorSceneData> SceneData;
	};

	return SNew(SSceneSettingsTableRow, OwnerTable)
		.SceneData(InItem);
}

TSharedRef<ITableRow> ICognitiveSettingsCustomization::OnGenerateSDKListRow(TSharedPtr<FString> InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	return
		SNew(SComboRow< TSharedPtr<FString> >, OwnerTable)
		[
			SNew(SBox)
			.HAlign(EHorizontalAlignment::HAlign_Center)
			.VAlign(EVerticalAlignment::VAlign_Center)
		[
			SNew(STextBlock)
			.Text(FText::FromString(*InItem))
			.Font(FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Bold.ttf"), 12))
			.Justification(ETextJustify::Center)
		]
	
		];
}

void ICognitiveSettingsCustomization::OnCommitedExportMinimumSize(float InNewValue, ETextCommit::Type CommitType)
{
	FCognitiveEditorTools::GetInstance()->MinimumSize = InNewValue;
}

void ICognitiveSettingsCustomization::OnChangedExportMinimumSize(float InNewValue)
{
	FCognitiveEditorTools::GetInstance()->MinimumSize = InNewValue;
}

void ICognitiveSettingsCustomization::OnCommitedExportMaximumSize(float InNewValue, ETextCommit::Type CommitType)
{
	FCognitiveEditorTools::GetInstance()->MaximumSize = InNewValue;
}

void ICognitiveSettingsCustomization::OnChangedExportMaximumSize(float InNewValue)
{
	FCognitiveEditorTools::GetInstance()->MaximumSize = InNewValue;
}

void ICognitiveSettingsCustomization::OnChangedExcludeMesh(const FText& InNewValue)
{
	FCognitiveEditorTools::GetInstance()->ExcludeMeshes = InNewValue.ToString();
}

void ICognitiveSettingsCustomization::OnCheckStateChangedStaticOnly(const bool& InNewValue)
{
	FCognitiveEditorTools::GetInstance()->StaticOnly = InNewValue;
}

void ICognitiveSettingsCustomization::OnChangedExportMinPolygon(int32 InNewValue)
{
	FCognitiveEditorTools::GetInstance()->MinPolygon = InNewValue;
}
void ICognitiveSettingsCustomization::OnCommitedExportMinPolygon(int32 InNewValue, ETextCommit::Type CommitType)
{
	FCognitiveEditorTools::GetInstance()->MinPolygon = InNewValue;
}

void ICognitiveSettingsCustomization::OnChangedExportMaxPolygon(int32 InNewValue)
{
	FCognitiveEditorTools::GetInstance()->MaxPolygon = InNewValue;
}
void ICognitiveSettingsCustomization::OnCommitedExportMaxPolygon(int32 InNewValue, ETextCommit::Type CommitType)
{
	FCognitiveEditorTools::GetInstance()->MaxPolygon = InNewValue;
}

void ICognitiveSettingsCustomization::OnChangedExportTextureRefactor(int32 InNewValue)
{
	FCognitiveEditorTools::GetInstance()->TextureRefactor = InNewValue;
}
void ICognitiveSettingsCustomization::OnCommitedExportTextureRefactor(int32 InNewValue, ETextCommit::Type CommitType)
{
	FCognitiveEditorTools::GetInstance()->TextureRefactor = InNewValue;
}

void ICognitiveSettingsCustomization::OnExportPathChanged(const FText& Text)
{
	FCognitiveEditorTools::GetInstance()->BaseExportDirectory = Text.ToString();
}

#undef LOCTEXT_NAMESPACE