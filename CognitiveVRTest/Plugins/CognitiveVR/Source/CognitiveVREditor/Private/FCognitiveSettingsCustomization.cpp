
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
					SNew(STextBlock)
					//.MinDesiredWidth(256)
					.Text(FText::FromString("Name"))
				]

				+ SHeaderRow::Column("id")
				.FillWidth(1)
				[
					SNew(STextBlock)
					//.MinDesiredWidth(512)
					.Text(FText::FromString("Id"))
				]

				+ SHeaderRow::Column("version number")
				.FillWidth(0.3)
				[
					SNew(STextBlock)
					.Text(FText::FromString("Version Number"))
				]

				+ SHeaderRow::Column("version id")
				.FillWidth(0.3)
				[
					SNew(STextBlock)
					.Text(FText::FromString("Version Id"))
				]

				+ SHeaderRow::Column("open")
				[
					SNew(STextBlock)
					.Text(FText::FromString("Scene Explorer"))
				]
			)
		]
	];

	/*IDetailCategoryBuilder& SceneWorkflow = DetailBuilder.EditCategory(TEXT("Scene Upload Workflow"));
	SceneWorkflow.InitiallyCollapsed(true);

	SceneWorkflow.AddCustomRow(FText::FromString("Commands"))
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.Padding(4.0f, 0.0f)
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.Padding(0,0,0,4)
			.HAlign(EHorizontalAlignment::HAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString("1. Export Settings"))
			]
			+SVerticalBox::Slot()
			.AutoHeight()
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
				[
					SNew(SButton)
					.IsEnabled_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::HasDeveloperKey)
					.Text(FText::FromString("Select Blender.exe"))
					.OnClicked_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::Select_Blender)
				]
			]
			+ SVerticalBox::Slot()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				[
					SNew(STextBlock)
					.Text(FText::FromString("Exclude Meshes"))
				]
				+SHorizontalBox::Slot()
				[
					SNew(SEditableTextBox)
					.MinDesiredWidth(128)
					.Text_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::GetExcludeMeshes)
					.OnTextChanged(this, &FCognitiveSettingsCustomization::OnChangedExcludeMesh)
				]
			]
			+SVerticalBox::Slot()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				[
					SNew(STextBlock)
					.Text(FText::FromString("Static Only"))
				]
				+ SHorizontalBox::Slot()
				[
					SNew(SCheckBox)
					.IsChecked_Raw(FCognitiveEditorTools::GetInstance(),&FCognitiveEditorTools::GetStaticOnlyCheckboxState)
					.OnCheckStateChanged_Raw(FCognitiveEditorTools::GetInstance(),&FCognitiveEditorTools::OnStaticOnlyCheckboxChanged)
				]
			]
			+ SVerticalBox::Slot()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				[
					SNew(STextBlock)
					.Text(FText::FromString("Minimum Size"))
				]
				+ SHorizontalBox::Slot()
				[
					SNew(SSpinBox<float>)
					.MinValue(0)
					.MaxValue(500000)
					.MinSliderValue(0)
					.MaxSliderValue(500000)
					.Value_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::GetMinimumSize)
					.OnValueChanged(this, &FCognitiveSettingsCustomization::OnChangedExportMinimumSize)
					.OnValueCommitted(this, &FCognitiveSettingsCustomization::OnCommitedExportMinimumSize)
				]
			]
			+ SVerticalBox::Slot()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				[
					SNew(STextBlock)
					.Text(FText::FromString("Maximum Size"))
				]
				+ SHorizontalBox::Slot()
				[
					SNew(SSpinBox<float>)
					.MinValue(0)
					.MaxValue(500000)
					.MinSliderValue(0)
					.MaxSliderValue(500000)
					.Value_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::GetMaximumSize)
					.OnValueChanged(this, &FCognitiveSettingsCustomization::OnChangedExportMaximumSize)
					.OnValueCommitted(this, &FCognitiveSettingsCustomization::OnCommitedExportMaximumSize)
				]
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SButton)
				.IsEnabled_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::HasFoundBlender)
				.Text(FText::FromString("Select Export Meshes"))
				.OnClicked_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::Select_Export_Meshes)
			]
		]

		+ SHorizontalBox::Slot()
		.Padding(4.0f, 0.0f)
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()

			.HAlign(EHorizontalAlignment::HAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString("2. Export"))
			]

			+ SVerticalBox::Slot()
			.Padding(0, 0, 0, 4)
			.HAlign(EHorizontalAlignment::HAlign_Center)
			[
				SNew(STextBlock)
				.ColorAndOpacity(FLinearColor::Yellow)
				.Text(FText::FromString("Important - Export as \"*.obj\"!"))
			]

			+ SVerticalBox::Slot()
			.MaxHeight(32)
			[
				SNew(SButton)
				.IsEnabled_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::HasFoundBlenderAndHasSelection)
				.Text(FText::FromString("Export Selected Scene Actors"))
				.OnClicked_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::Export_Selected)
			]
			+ SVerticalBox::Slot()
			.MaxHeight(32)
			[
				SNew(SButton)
				.IsEnabled_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::HasFoundBlender)
				.Text(FText::FromString("Export All Scene Actors"))
				.OnClicked_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::Export_All)
			]
		]
		+ SHorizontalBox::Slot()
		.Padding(4.0f, 0.0f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 4)
			.HAlign(EHorizontalAlignment::HAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString("3. Optimize Files"))
			]

			+SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBox)
				[
					SNew(SButton)
					.IsEnabled_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::HasFoundBlender)
					.Text(FText::FromString("Select Export Directory"))
					.OnClicked_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::SelectBaseExportDirectory)
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::GetBaseExportDirectoryDisplay)
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBox)
				[
					SNew(SButton)
					.IsEnabled_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::HasSetExportDirectory)
					.Text(FText::FromString("Export Transparent Textures"))
					.OnClicked_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::List_Materials)
				]
			]


			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				[
					SNew(STextBlock)
					.Text(FText::FromString("Min Polygons"))
				]
				+ SHorizontalBox::Slot()
				[
					SNew(SSpinBox<int32>)
					.MinValue(1)
					.MaxValue(10248576)
					.MinSliderValue(1)
					.MaxSliderValue(10248576)
					.Value_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::GetMinPolygon)
					.OnValueChanged(this, &FCognitiveSettingsCustomization::OnChangedExportMinPolygon)
					.OnValueCommitted(this, &FCognitiveSettingsCustomization::OnCommitedExportMinPolygon)
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				[
					SNew(STextBlock)
					.Text(FText::FromString("Max Polygons"))
				]
				+ SHorizontalBox::Slot()
				[
					SNew(SSpinBox<int32>)
					.MinValue(1)
					.MaxValue(10248576)
					.MinSliderValue(1)
					.MaxSliderValue(10248576)
					.Value_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::GetMaxPolygon)
					.OnValueChanged(this, &FCognitiveSettingsCustomization::OnChangedExportMaxPolygon)
					.OnValueCommitted(this, &FCognitiveSettingsCustomization::OnCommitedExportMaxPolygon)
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBox)
				[
					SNew(SButton)
					.IsEnabled_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::LoginAndCustonerIdAndBlenderExportDir)
					.Text(FText::FromString("Reduce Mesh Topology"))
					.OnClicked_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::Reduce_Meshes)
				]
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				[
					SNew(STextBlock)
					.Text(FText::FromString("Texture Scale"))
				]
				+ SHorizontalBox::Slot()
				[
					SNew(SSpinBox<int32>)
					.MinValue(1)
					.MaxValue(16)
					.MinSliderValue(1)
					.MaxSliderValue(16)
					.Value_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::GetTextureRefactor)
					.OnValueChanged(this, &FCognitiveSettingsCustomization::OnChangedExportTextureRefactor)
					.OnValueCommitted(this, &FCognitiveSettingsCustomization::OnCommitedExportTextureRefactor)
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBox)
				[
					SNew(SButton)
					.IsEnabled_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::LoginAndCustonerIdAndBlenderExportDir)
					.Text(FText::FromString("Convert Textures"))
					.OnClicked_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::Reduce_Textures)
				]
			]
		]
		+ SHorizontalBox::Slot()
		.Padding(4.0f, 0.0f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.Padding(0, 0, 0, 4)
			.HAlign(EHorizontalAlignment::HAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString("4. Upload"))
			]
			+ SVerticalBox::Slot()
			.MaxHeight(32)
			[
				SNew(SButton)
				.IsEnabled_Raw(FCognitiveEditorTools::GetInstance(),&FCognitiveEditorTools::HasSetExportDirectory)
				.Text(FText::FromString("Take Screenshot"))
				.OnClicked_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::TakeScreenshot)
			]

			+ SVerticalBox::Slot()
			.MaxHeight(32)
			[
				SNew(SButton)
				.IsEnabled_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::CanUploadSceneFiles)
				.Text_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::UploadSceneNameFiles)
				.ToolTip(SNew(SToolTip).Text(LOCTEXT("export tip", "Make sure you have settings.json and no .bmp files in your export directory")))
				.OnClicked_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::UploadScene)
			]
			//+ SVerticalBox::Slot()
			//.MaxHeight(32)
			//[
			//	SNew(SButton)
			//	.IsEnabled(this, &FCognitiveTools::CurrentSceneHasSceneId)
			//	.Text(FText::FromString("Upload Screenshot"))
			//	.OnClicked(this, &FCognitiveTools::SelectUploadScreenshot)
			//]
		]
		+ SHorizontalBox::Slot()
		.Padding(4.0f, 0.0f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.Padding(0, 0, 0, 4)
			.HAlign(EHorizontalAlignment::HAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString("5. Scene Explorer"))
			]
			+ SVerticalBox::Slot()
			.MaxHeight(32)
			[
				SNew(SButton)
				.IsEnabled_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::CurrentSceneHasSceneId)
				.Text_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::OpenSceneNameInBrowser)
				.OnClicked_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::OpenCurrentSceneInBrowser)
			]
		]
		
	];*/


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
			[
				SNew(SButton)
				.IsEnabled_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::HasDeveloperKey)
				.Text(FText::FromString("Select Blender.exe"))
				.OnClicked_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::Select_Blender)
			]
		]

		+SVerticalBox::Slot()
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			[
				SNew(SButton)
				.IsEnabled_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::HasFoundBlender)
				.Text(FText::FromString("Select Export Directory"))
				.OnClicked_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::SelectBaseExportDirectory)
			]
			+ SHorizontalBox::Slot()
			[
				SNew(STextBlock)
				.Text_Raw(FCognitiveEditorTools::GetInstance(),&FCognitiveEditorTools::GetBaseExportDirectoryDisplay)
			]
		]
	];

	DynamicWorkflow.AddCustomRow(FText::FromString("Commands"))
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.Padding(4.0f, 0.0f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.HAlign(EHorizontalAlignment::HAlign_Center)
				[
					SNew(STextBlock)
					.Text(FText::FromString("1. Export"))
				]
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
			+ SHorizontalBox::Slot()
			.Padding(4.0f, 0.0f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 0, 0, 4)
				.HAlign(EHorizontalAlignment::HAlign_Center)
				[
					SNew(STextBlock)
					.Text(FText::FromString("2. Organize Meshes"))
				]
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
					.Text(FText::FromString("Refresh Sub Directories"))
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
			.Padding(4.0f, 0.0f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.Padding(0, 0, 0, 4)
				.HAlign(EHorizontalAlignment::HAlign_Center)
				[
					SNew(STextBlock)
					.Text(FText::FromString("3. Upload"))
				]
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
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.ColorAndOpacity(FLinearColor::Yellow)
					.Visibility_Raw(FCognitiveEditorTools::GetInstance(),&FCognitiveEditorTools::GetDuplicateDyanmicObjectVisibility)
					.Text(FText::FromString("Scene contains some duplicate Dynamic Object Ids"))
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SButton)
					.IsEnabled_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::HasDeveloperKey)
					.Visibility_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::GetDuplicateDyanmicObjectVisibility)
					.Text(FText::FromString("Set Unique Dynamic Ids"))
					.OnClicked_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::SetUniqueDynamicIds)
				]
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

	FString EngineIni = FPaths::Combine(*(FPaths::GameDir()), TEXT("Config/DefaultEngine.ini"));
	FString EditorIni = FPaths::Combine(*(FPaths::GameDir()), TEXT("Config/DefaultEditor.ini"));

	GConfig->GetString(TEXT("Analytics"), TEXT("ApiKey"), FCognitiveEditorTools::GetInstance()->APIKey, EngineIni);
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