
#include "CognitiveVREditorPrivatePCH.h"
#include "CognitiveTools.h"

#define LOCTEXT_NAMESPACE "BaseToolEditor"

static TArray<TSharedPtr<FDynamicData>> SceneDynamics;
static TArray<TSharedPtr<FDynamicData>> SceneExplorerDynamics;
static TArray<TSharedPtr<FString>> SubDirectoryNames;

//deals with all the interface stuff in the editor preferences
//includes any details needed to make the ui work

void FCognitiveTools::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TSet<UClass*> Classes;

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
	}


	IDetailCategoryBuilder& SettingsCategory = DetailBuilder.EditCategory(TEXT("Export Settings"));

	MinPolygonProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UCognitiveVRSettings, MinPolygons));
	MaxPolygonProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UCognitiveVRSettings, MaxPolygons));
	StaticOnlyProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UCognitiveVRSettings, staticOnly));
	MinSizeProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UCognitiveVRSettings, MinimumSize));
	MaxSizeProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UCognitiveVRSettings, MaximumSize));
	TextureResizeProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UCognitiveVRSettings, TextureResizeFactor));
	ExcludeMeshProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UCognitiveVRSettings, ExcludeMeshes));

	// Create a commands category
	IDetailCategoryBuilder& LoginCategory = DetailBuilder.EditCategory(TEXT("Account"),FText::GetEmpty(),ECategoryPriority::Important);

	LoginCategory.AddCustomRow(FText::FromString("Commands"))
		.ValueContent()
		.HAlign(HAlign_Fill)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.MaxWidth(96)
			[
				SNew(STextBlock)
				.Text(FText::FromString("Email"))
			]

			+ SHorizontalBox::Slot()
			.MaxWidth(128)
			[
				SNew(SEditableTextBox)
				.MinDesiredWidth(128)
				.Visibility(this,&FCognitiveTools::LoginTextboxUsable)
				.OnTextChanged(this,&FCognitiveTools::OnEmailChanged)
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
				.Text(FText::FromString("Password"))
			]

			+ SHorizontalBox::Slot()
			.MaxWidth(128)
			[
				SNew(SEditableTextBox)
				.IsPassword(true)
				.MinDesiredWidth(128)
				.Visibility(this, &FCognitiveTools::LoginTextboxUsable)
				.OnTextChanged(this,&FCognitiveTools::OnPasswordChanged)
			]

			+ SHorizontalBox::Slot() //log in
			.MaxWidth(128)
				.Padding(4, 0, 0, 0)
			[
				SNew(SButton)
				.Visibility(this, &FCognitiveTools::GetLoginButtonState)
				.IsEnabled(this, &FCognitiveTools::HasValidLogInFields)
				.Text(FText::FromString("Log In"))
				.OnClicked(this, &FCognitiveTools::LogIn)
			]
			
			+ SHorizontalBox::Slot() //log out
			.MaxWidth(128)
			.Padding(4,0,0,0)
			[
				SNew(SButton)
				.Visibility(this, &FCognitiveTools::GetLogoutButtonState)
				.IsEnabled(true)
				.Text(FText::FromString("Log Out"))
				.OnClicked(this, &FCognitiveTools::LogOut)
			]
		];

	//simple spacer
	LoginCategory.AddCustomRow(FText::FromString("Commands"))
	.ValueContent()
	.HAlign(HAlign_Fill)
	[
		SNew(SHorizontalBox)
		.Visibility(EVisibility::Hidden)
	];

	//ORGANIZATION DROPDOWN
	LoginCategory.AddCustomRow(FText::FromString("Commands"))
	.ValueContent()
	.HAlign(HAlign_Fill)
	[
		SNew(SHorizontalBox)
		//button
		+ SHorizontalBox::Slot()
		.MaxWidth(96)
		[
			SNew(STextBlock)
			.Text(FText::FromString("Organization"))
		]

		+ SHorizontalBox::Slot()
		.MaxWidth(128)
		[
			SNew(STextComboBox)
			.OptionsSource(&AllOrgNames)
			.IsEnabled(this, &FCognitiveTools::HasLoggedIn)				
			.OnSelectionChanged(this, &FCognitiveTools::OnOrganizationChanged)
		]
	];

	//PRODUCT DROPDOWN
	LoginCategory.AddCustomRow(FText::FromString("Commands"))
	.ValueContent()
	.HAlign(HAlign_Fill)
	[
		SNew(SHorizontalBox)
		//button
		+ SHorizontalBox::Slot()
		.MaxWidth(96)
		[
			SNew(STextBlock)
			.Text(FText::FromString("Product"))
		]

		+ SHorizontalBox::Slot()
		.MaxWidth(128)
		[
			SNew(STextComboBox)
			.IsEnabled(this,&FCognitiveTools::HasLoggedIn)
			.OptionsSource(&AllProductNames)
			.OnSelectionChanged(this, &FCognitiveTools::OnProductChanged)
		]
	];

	LoginCategory.AddCustomRow(FText::FromString("Commands"))
	.ValueContent()
	.MinDesiredWidth(256)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.MaxWidth(128)
		[
			SNew(SCheckBox)
			.Style(FCoreStyle::Get(), "RadioButton")
			.IsEnabled(this, &FCognitiveTools::HasLoadedOrSelectedValidProduct)
			.IsChecked(this, &FCognitiveTools::HandleRadioButtonIsChecked, EReleaseType::Test)
			.OnCheckStateChanged(this, &FCognitiveTools::HandleRadioButtonCheckStateChanged, EReleaseType::Test)
			[
				SNew(STextBlock)
				.Text(FText::FromString("Test"))
			]
		]
	
		+ SHorizontalBox::Slot()
		.MaxWidth(128)
		[
			SNew(SCheckBox)
			.Style(FCoreStyle::Get(), "RadioButton")
			.IsEnabled(this, &FCognitiveTools::HasLoadedOrSelectedValidProduct)
			.IsChecked(this, &FCognitiveTools::HandleRadioButtonIsChecked, EReleaseType::Production)
			.OnCheckStateChanged(this, &FCognitiveTools::HandleRadioButtonCheckStateChanged, EReleaseType::Production)
			[
				SNew(STextBlock)
				.Text(FText::FromString("Production"))
			]
		]
	];

	//simple spacer
	LoginCategory.AddCustomRow(FText::FromString("Commands"))
	.ValueContent()
	.HAlign(HAlign_Fill)
	[
		SNew(SHorizontalBox)
		.Visibility(EVisibility::Hidden)
	];

	LoginCategory.AddCustomRow(FText::FromString("Commands"))
	.ValueContent()
	.MinDesiredWidth(256)
	[
		SNew(STextBlock)
		.Text(this,&FCognitiveTools::GetCustomerId)
	];

	LoginCategory.AddCustomRow(FText::FromString("Commands"))
	.ValueContent()
	.MinDesiredWidth(256)
	[
		SNew(SButton)
		.Text(FText::FromString("Save"))
		.IsEnabled(this,&FCognitiveTools::HasSelectedValidProduct)
		.OnClicked(this, &FCognitiveTools::SaveCustomerIdToFile)
	];

	LoginCategory.AddCustomRow(FText::FromString("Commands"))
	.ValueContent()
	.MinDesiredWidth(256)
	[
		SNew(SButton)
		.Text(FText::FromString("Open on Dashboard..."))
		.IsEnabled(this,&FCognitiveTools::EnableOpenProductOnDashboard)
		.OnClicked(this, &FCognitiveTools::OpenProductOnDashboard)
	];

	LoginCategory.AddCustomRow(FText::FromString("Commands"))
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.HAlign(EHorizontalAlignment::HAlign_Center)
		[
			SNew(STextBlock)
			.Visibility(this, &FCognitiveTools::ConfigFileChangedVisibility)
			.ColorAndOpacity(FLinearColor::Yellow)
			.Text(FText::FromString("Config files changed. Data displayed below may be incorrect until you restart Unreal Editor!"))
		]
	];

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
			.IsEnabled(this, &FCognitiveTools::HasEditorAuthToken)
			.Text(FText::FromString("Get Latest Scene Version Data"))
			.ToolTip(SNew(SToolTip).Text(LOCTEXT("get scene data tip", "This will get the latest scene version data from Scene Explorer and saves it to your config files. Must restart Unreal Editor to see the changes in your config files here")))
			.OnClicked(this, &FCognitiveTools::DebugRefreshCurrentScene)
		]
	];

	ExportedSceneData.AddCustomRow(FText::FromString("Commands"))
	[
			SNew(SBox)
			.MaxDesiredHeight(200)
			[
			SNew(SListView<TSharedPtr<FEditorSceneData>>)
			.ItemHeight(24)
			.ListItemsSource(&SceneData)
			.OnGenerateRow(this, &FCognitiveTools::OnGenerateWorkspaceRow)
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

	IDetailCategoryBuilder& SceneWorkflow = DetailBuilder.EditCategory(TEXT("Scene Upload Workflow"));
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
					.IsChecked(this, &FCognitiveTools::HasFoundBlenderCheckbox)
				]
				+SHorizontalBox::Slot()
				[
					SNew(SButton)
					.IsEnabled(this, &FCognitiveTools::HasLoggedIn)
					.Text(FText::FromString("Select Blender.exe"))
					.OnClicked(this, &FCognitiveTools::Select_Blender)
				]
			]
			+ SVerticalBox::Slot()
			[
				SNew(SProperty, ExcludeMeshProperty)
				.IsEnabled(this, &FCognitiveTools::HasFoundBlender)
				.ShouldDisplayName(true)
			]
			+SVerticalBox::Slot()
			[
				SNew(SProperty,StaticOnlyProperty)
				.IsEnabled(this, &FCognitiveTools::HasFoundBlender)
				.ShouldDisplayName(true)
			]
			+ SVerticalBox::Slot()
			[
				SNew(SProperty, MinSizeProperty)
				.IsEnabled(this, &FCognitiveTools::HasFoundBlender)
				.ShouldDisplayName(true)
			]
			+ SVerticalBox::Slot()
			[
				SNew(SProperty, MaxSizeProperty)
				.IsEnabled(this, &FCognitiveTools::HasFoundBlender)
				.ShouldDisplayName(true)
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SButton)
				.IsEnabled(this, &FCognitiveTools::HasFoundBlender)
				.Text(FText::FromString("Select Export Meshes"))
				.OnClicked(this, &FCognitiveTools::Select_Export_Meshes)
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
				.IsEnabled(this, &FCognitiveTools::HasFoundBlenderAndHasSelection)
				.Text(FText::FromString("Export Selected Scene Actors"))
				.OnClicked(this, &FCognitiveTools::Export_Selected)
			]
			+ SVerticalBox::Slot()
			.MaxHeight(32)
			[
				SNew(SButton)
				.IsEnabled(this, &FCognitiveTools::HasFoundBlender)
				.Text(FText::FromString("Export All Scene Actors"))
				.OnClicked(this, &FCognitiveTools::Export_All)
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
					.IsEnabled(this, &FCognitiveTools::HasFoundBlender)
					.Text(FText::FromString("Select Export Directory"))
					.OnClicked(this, &FCognitiveTools::Select_Export_Directory)
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(this, &FCognitiveTools::GetExportDirectory)
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBox)
				[
					SNew(SButton)
					.IsEnabled(this, &FCognitiveTools::HasSetExportDirectory)
					.Text(FText::FromString("Export Transparent Textures"))
					.OnClicked(this, &FCognitiveTools::List_Materials)
				]
			]


			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SProperty, MinPolygonProperty)
				.IsEnabled(this, &FCognitiveTools::LoginAndCustonerIdAndBlenderExportDir)
				.ShouldDisplayName(true)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SProperty, MaxPolygonProperty)
				.IsEnabled(this, &FCognitiveTools::LoginAndCustonerIdAndBlenderExportDir)
				.ShouldDisplayName(true)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBox)
				[
					SNew(SButton)
					.IsEnabled(this, &FCognitiveTools::LoginAndCustonerIdAndBlenderExportDir)
					.Text(FText::FromString("Reduce Mesh Topology"))
					.OnClicked(this, &FCognitiveTools::Reduce_Meshes)
				]
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SProperty, TextureResizeProperty)
				.IsEnabled(this, &FCognitiveTools::LoginAndCustonerIdAndBlenderExportDir)
				.ShouldDisplayName(true)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBox)
				[
					SNew(SButton)
					.IsEnabled(this, &FCognitiveTools::LoginAndCustonerIdAndBlenderExportDir)
					.Text(FText::FromString("Convert Textures"))
					.OnClicked(this, &FCognitiveTools::Reduce_Textures)
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
				.IsEnabled(this, &FCognitiveTools::CanUploadSceneFiles)
				.Text(this, &FCognitiveTools::UploadSceneNameFiles)
				.ToolTip(SNew(SToolTip).Text(LOCTEXT("export tip", "Make sure you have settings.json and no .bmp files in your export directory")))
				.OnClicked(this, &FCognitiveTools::UploadScene)
			]
			+ SVerticalBox::Slot()
			.MaxHeight(32)
			.HAlign(EHorizontalAlignment::HAlign_Center)
			.VAlign(EVerticalAlignment::VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString("Press F9 to take a screenshot"))
			]
			+ SVerticalBox::Slot()
			.MaxHeight(32)
			[
				SNew(SButton)
				.IsEnabled(this, &FCognitiveTools::CurrentSceneHasSceneId)
				.Text(FText::FromString("Upload Screenshot"))
				.OnClicked(this, &FCognitiveTools::SelectUploadScreenshot)
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
				.Text(FText::FromString("5. Scene Explorer"))
			]
			+ SVerticalBox::Slot()
			.MaxHeight(32)
			[
				SNew(SButton)
				.IsEnabled(this, &FCognitiveTools::CurrentSceneHasSceneId)
				.Text(this, &FCognitiveTools::OpenSceneNameInBrowser)
				.OnClicked(this, &FCognitiveTools::OpenCurrentSceneInBrowser)
			]
		]
		
	];


	IDetailCategoryBuilder& DynamicWorkflow = DetailBuilder.EditCategory(TEXT("Dynamic Upload Workflow"));

	DynamicWorkflow.InitiallyCollapsed(true);

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
					.IsEnabled(this, &FCognitiveTools::HasFoundBlender) //TODO make this disabled if no selection
					.Text(FText::FromString("Export Selected Dynamic Objects"))
					.OnClicked(this, &FCognitiveTools::ExportSelectedDynamics)
				]
				+ SVerticalBox::Slot()
				.MaxHeight(32)
				[
					SNew(SButton)
					.IsEnabled(this, &FCognitiveTools::HasFoundBlender)
					.Text(FText::FromString("Export All Dynamic Object Meshes"))
					.OnClicked(this, &FCognitiveTools::ExportDynamics)
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
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SButton)
					.IsEnabled(this,&FCognitiveTools::HasLoggedIn)
					.Text(FText::FromString("Select Dynamic Mesh Directory"))
					.OnClicked(this, &FCognitiveTools::SelectDynamicsDirectory)
				]
				+SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SButton)
					.IsEnabled(this,& FCognitiveTools::HasSetDynamicExportDirectory)
					.Text(FText::FromString("Refresh"))
					.OnClicked(this, &FCognitiveTools::RefreshDynamicSubDirectory)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(this, &FCognitiveTools::GetDynamicExportDirectory)
				]
				+ SVerticalBox::Slot()
				[
					SNew(SBox)
					.HeightOverride(200)
					[
						SAssignNew(SubDirectoryListWidget,SFStringListWidget)
						.Items(GetSubDirectoryNames())
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
					.IsEnabled(this, &FCognitiveTools::HasSetDynamicExportDirectory)
					.Text(this, &FCognitiveTools::GetDynamicObjectUploadText)
					.OnClicked(this, &FCognitiveTools::UploadDynamics)
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
						.CognitiveTools(this)
						.Items(SceneDynamics)
					]
				]
				+SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SButton)
					.IsEnabled(this, &FCognitiveTools::HasLoggedIn)
					.Text(FText::FromString("Refresh"))
					.OnClicked(this, &FCognitiveTools::RefreshDisplayDynamicObjectsCountInScene)
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(this,&FCognitiveTools::DisplayDynamicObjectsCountInScene)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.ColorAndOpacity(FLinearColor::Yellow)
					.Visibility(this,&FCognitiveTools::GetDuplicateDyanmicObjectVisibility)
					.Text(FText::FromString("Scene contains some duplicate Dynamic Object Ids"))
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SButton)
					.IsEnabled(this, &FCognitiveTools::HasLoggedIn)
					.Text(FText::FromString("Set Unique Dynamic Ids"))
					.OnClicked(this, &FCognitiveTools::SetUniqueDynamicIds)
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
					.Text(this, &FCognitiveTools::DisplayDynamicObjectsCountOnWeb)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					[
						SNew(SButton)
						.IsEnabled(this,&FCognitiveTools::HasEditorAuthToken)
						.ToolTip(SNew(SToolTip).Text(this,&FCognitiveTools::GetDynamicsOnSceneExplorerTooltip))
						.Text(FText::FromString("Get Dynamics List from SceneExplorer"))
						.OnClicked(this, &FCognitiveTools::GetDynamicsManifest)
					]
					+ SHorizontalBox::Slot()
					[
						SNew(SButton)
						.IsEnabled(this, &FCognitiveTools::HasEditorAuthToken)
						.ToolTip(SNew(SToolTip).Text(this, &FCognitiveTools::SendDynamicsToSceneExplorerTooltip))
						.Text(FText::FromString("Send Dynamics List to SceneExplorer"))
						.OnClicked(this, &FCognitiveTools::UploadDynamicsManifest)
					]
				]
			]
		];


	FCognitiveTools::RefreshSceneData();
	FCognitiveTools::RefreshDisplayDynamicObjectsCountInScene();
	FCognitiveTools::SearchForBlender();

	SelectedProduct.customerId = GetCustomerIdFromFileWithoutRelease();
}

TSharedRef<ITableRow> FCognitiveTools::OnGenerateWorkspaceRow(TSharedPtr<FEditorSceneData> InItem, const TSharedRef<STableViewBase>& OwnerTable)
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
				.OnClicked(this,&FCognitiveTools::OpenSceneInBrowser,InItem->Id)
			]
		];
}

TSharedRef<ITableRow> FCognitiveTools::OnGenerateDynamicRow(TSharedPtr<FDynamicData> InItem, const TSharedRef<STableViewBase>& OwnerTable)
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
				.Text(FText::FromString(FString::FromInt(InItem->Id)))
			]
		];
}

bool FCognitiveTools::HasValidLogInFields() const
{
	return Email.Len() > 0 && Password.Len() > 0;
}

FText FCognitiveTools::UploadSceneNameFiles() const
{
	auto currentscenedata = GetCurrentSceneData();
	if (!currentscenedata.IsValid())
	{
		UWorld* myworld = GWorld->GetWorld();

		FString currentSceneName = myworld->GetMapName();
		currentSceneName.RemoveFromStart(myworld->StreamingLevelsPrefix);

		return FText::FromString("Upload Files for " + currentSceneName);
	}
	FString outstring = "Upload Files for " + currentscenedata->Name;

	return FText::FromString(outstring);
}

FText FCognitiveTools::OpenSceneNameInBrowser() const
{
	auto currentscenedata = GetCurrentSceneData();
	if (!currentscenedata.IsValid())
	{
		UWorld* myworld = GWorld->GetWorld();

		FString currentSceneName = myworld->GetMapName();
		currentSceneName.RemoveFromStart(myworld->StreamingLevelsPrefix);

		return FText::FromString("Upload Files for " + currentSceneName);
	}
	FString outstring = "Open " + currentscenedata->Name + " in Browser...";

	return FText::FromString(outstring);
}

EVisibility FCognitiveTools::LoginTextboxUsable() const
{
	if (HasLoggedIn())
		return EVisibility::HitTestInvisible;
	return EVisibility::Visible;
}

FText FCognitiveTools::GetDynamicObjectUploadText() const
{
	auto data = GetCurrentSceneData();
	if (!data.IsValid())
	{
		return FText::FromString("scene data is invalid!");
	}

	return FText::FromString("Upload "+FString::FromInt(SubDirectoryNames.Num())+" Dynamic Object Meshes to " + data->Name + " version " + FString::FromInt(data->VersionNumber));
}

FText FCognitiveTools::GetDynamicsFromManifest() const
{
	return FText::FromString("DYNAMICS");
}

FReply FCognitiveTools::LogOut()
{
	Email = "";
	Password = "";

	FAnalyticsCognitiveVR::Get().EditorAuthToken = "";
	FAnalyticsCognitiveVR::Get().EditorSessionId = "";
	FAnalyticsCognitiveVR::Get().EditorSessionToken = "";

	return FReply::Handled();
}

FReply FCognitiveTools::RefreshDynamicSubDirectory()
{
	FindAllSubDirectoryNames();
	SubDirectoryListWidget->RefreshList();
	return FReply::Handled();
}

FReply FCognitiveTools::DebugRefreshCurrentScene()
{
	TSharedPtr<FEditorSceneData> scenedata = GetCurrentSceneData();

	if (scenedata.IsValid())
	{
		SceneVersionRequest(*scenedata);
	}

	return FReply::Handled();
}

EVisibility FCognitiveTools::GetLogoutButtonState() const
{
	if (HasLoggedIn())
		return EVisibility::Visible;
	return EVisibility::Collapsed;
}

EVisibility FCognitiveTools::GetLoginButtonState() const
{
	if (HasLoggedIn())
		return EVisibility::Collapsed;
	return EVisibility::Visible;
}

FString FCognitiveTools::GetCustomerIdFromFile() const
{
	FString customerid;
	GConfig->GetString(TEXT("Analytics"), TEXT("CognitiveVRApiKey"), customerid, GEngineIni);
	return customerid;
}

FString FCognitiveTools::GetCustomerIdFromFileWithoutRelease() const
{
	FString customerid;
	GConfig->GetString(TEXT("Analytics"), TEXT("CognitiveVRApiKey"), customerid, GEngineIni);
	customerid.RemoveFromEnd("-test");
	customerid.RemoveFromEnd("-prod");
	return customerid;
}

bool FCognitiveTools::HasSavedCustomerId() const
{
	FString customerid = GetCustomerIdFromFile();
	if (customerid.Len() == 0) { return false; }
	if (customerid.EndsWith("-test")) { return true; }
	if (customerid.EndsWith("-prod")) { return true; }
	return false;
}

void FCognitiveTools::SaveOrganizationNameToFile(FString organization)
{
	//GConfig->SetString(TEXT("Analytics"), TEXT("CognitiveOrganization"), *organization, GEngineIni);
	//GConfig->Flush(false, GEngineIni);
}

TSharedPtr< FString > FCognitiveTools::GetOrganizationNameFromFile()
{
	FString organization;
	GConfig->GetString(TEXT("Analytics"), TEXT("CognitiveOrganization"), organization, GEngineIni);
	return MakeShareable(new FString(organization));
}

void FCognitiveTools::SaveProductNameToFile(FString product)
{
	//GConfig->SetString(TEXT("Analytics"), TEXT("CognitiveProduct"), *product, GEngineIni);
	//GConfig->Flush(false, GEngineIni);
}

TSharedPtr< FString > FCognitiveTools::GetProductNameFromFile()
{
	FString product;
	GConfig->GetString(TEXT("Analytics"), TEXT("CognitiveProduct"), product, GEngineIni);
	return MakeShareable(new FString(product));
}

FReply FCognitiveTools::OpenSceneInBrowser(FString sceneid)
{
	FString url = SceneExplorerOpen(sceneid);

	FPlatformProcess::LaunchURL(*url, nullptr, nullptr);

	return FReply::Handled();
}

FReply FCognitiveTools::OpenCurrentSceneInBrowser()
{
	TSharedPtr<FEditorSceneData> scenedata = GetCurrentSceneData();

	if (!scenedata.IsValid())
	{
		return FReply::Handled();
	}

	FString url = SceneExplorerOpen(scenedata->Id);

	return FReply::Handled();
}

// Callback for checking a radio button.
void FCognitiveTools::HandleRadioButtonCheckStateChanged(ECheckBoxState NewRadioState, EReleaseType RadioThatChanged)
{
	if (NewRadioState == ECheckBoxState::Checked)
	{
		RadioChoice = RadioThatChanged;
	}

	if (HasSelectedValidProduct())
	{
		SaveCustomerIdToFile();
	}
}

FReply FCognitiveTools::RefreshSceneData()
{
	SceneData.Empty();

	TArray<FString>scenstrings;
	FString TestSyncFile = FPaths::Combine(*(FPaths::GameDir()), TEXT("Config/DefaultEngine.ini"));
	GConfig->GetArray(TEXT("/Script/CognitiveVR.CognitiveVRSceneSettings"), TEXT("SceneData"), scenstrings,TestSyncFile);

	for (int i = 0; i < scenstrings.Num(); i++)
	{
		TArray<FString> Array;
		scenstrings[i].ParseIntoArray(Array, TEXT(","), true);

		if (Array.Num() == 2) //scenename,sceneid
		{
			//old scene data. append versionnumber and versionid
			Array.Add("1");
			Array.Add("0");
		}

		if (Array.Num() != 4)
		{
			GLog->Log("FCognitiveTools::RefreshSceneData failed to parse " + scenstrings[i]);
			continue;
		}

		FEditorSceneData* tempscene = new FEditorSceneData(Array[0], Array[1], FCString::Atoi(*Array[2]), FCString::Atoi(*Array[3]));
		SceneData.Add(MakeShareable(tempscene));
	}
	
	GLog->Log("FCognitiveTools::RefreshSceneData found this many scenes: " + FString::FromInt(SceneData.Num()));
	//ConfigFileHasChanged = true;

	return FReply::Handled();
}

void FCognitiveTools::SceneVersionRequest(FEditorSceneData data)
{
	if (FAnalyticsCognitiveVR::Get().EditorAuthToken.Len() == 0)
	{
		GLog->Log("FCognitiveTools::SceneVersionRequest no auth token. TODO get auth token and retry");
		//auto httprequest = RequestAuthTokenCallback();
		//httprequest->OnProcessRequestComplete().BindSP(this, &FCognitiveTools::OnSceneVersionGetAuthToken);
		return;
	}

	TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();

	HttpRequest->SetURL(GetSceneVersion(data.Id));

	GLog->Log("FCognitiveTools::SceneVersionRequest send scene version request");

	HttpRequest->SetHeader("X-HTTP-Method-Override", TEXT("GET"));
	HttpRequest->SetHeader("Authorization", TEXT("Bearer " + FAnalyticsCognitiveVR::Get().EditorAuthToken));

	HttpRequest->OnProcessRequestComplete().BindSP(this, &FCognitiveTools::SceneVersionResponse);
	HttpRequest->ProcessRequest();
}

void FCognitiveTools::SceneVersionResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	int32 responseCode = Response->GetResponseCode();

	//GLog->Log("FCognitiveTools::SceneVersionResponse response: " + Response->GetContentAsString());

	if (responseCode >= 500)
	{
		//internal server error
		GLog->Log("FCognitiveTools::SceneVersionResponse 500-ish internal server error");
		return;
	}
	if (responseCode >= 400)
	{
		if (responseCode == 401)
		{
			//not authorized
			GLog->Log("FCognitiveTools::SceneVersionResponse not authorized!");
			//DEBUG_RequestAuthToken();
			//auto httprequest = RequestAuthTokenCallback();
			//if (httprequest)
			//{
				//httprequest->OnProcessRequestComplete().BindSP(this, &FCognitiveTools::OnSceneVersionGetAuthToken);
			//}
			return;
		}
		else
		{
			//maybe no scene?
			GLog->Log("FCognitiveTools::SceneVersionResponse some error. Maybe no scene?");
			return;
		}
	}

	//parse response content to json

	TSharedPtr<FJsonObject> JsonSceneSettings;

	TSharedRef<TJsonReader<>>Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	if (FJsonSerializer::Deserialize(Reader, JsonSceneSettings))
	{
		//get the latest version of the scene
		int32 versionNumber = 0;
		int32 versionId = 0;
		TArray<TSharedPtr<FJsonValue>> versions = JsonSceneSettings->GetArrayField("versions");
		for (int i = 0; i < versions.Num(); i++) {

			int32 tempversion = versions[i]->AsObject()->GetNumberField("versionnumber");
			if (tempversion > versionNumber)
			{
				versionNumber = tempversion;
				versionId = versions[i]->AsObject()->GetNumberField("id");
			}
		}
		if (versionNumber + versionId == 0)
		{
			GLog->Log("FCognitiveTools::SceneVersionResponse couldn't find a latest version in SceneVersion data");
			return;
		}

		//check that there is scene data in ini
		TSharedPtr<FEditorSceneData> currentSceneData = GetCurrentSceneData();
		if (!currentSceneData.IsValid())
		{
			GLog->Log("FCognitiveTools::SceneVersionResponse can't find current scene data in ini files");
			return;
		}

		//get array of scene data
		TArray<FString> iniscenedata;

		FString TestSyncFile = FPaths::Combine(*(FPaths::GameDir()), TEXT("Config/DefaultEngine.ini"));
		GConfig->GetArray(TEXT("/Script/CognitiveVR.CognitiveVRSceneSettings"), TEXT("SceneData"), iniscenedata, TestSyncFile);

		//GLog->Log("found this many scene datas in ini " + FString::FromInt(iniscenedata.Num()));
		//GLog->Log("looking for scene " + currentSceneData->Name);

		//update current scene
		for (int i = 0; i < iniscenedata.Num(); i++)
		{
			//GLog->Log("looking at data " + iniscenedata[i]);

			TArray<FString> entryarray;
			iniscenedata[i].ParseIntoArray(entryarray, TEXT(","), true);

			if (entryarray[0] == currentSceneData->Name)
			{
				iniscenedata[i] = entryarray[0] + "," + entryarray[1] + "," + FString::FromInt(versionNumber) + "," + FString::FromInt(versionId);
				//GLog->Log("FCognitiveToolsCustomization::SceneVersionResponse overwriting scene data to append versionnumber and versionid");
				//GLog->Log(iniscenedata[i]);
				break;
			}
			else
			{
				//GLog->Log("found scene " + entryarray[0]);
			}
		}

		GLog->Log("FCognitiveTools::SceneVersionResponse successful. Write scene data to config file");

		//set array to config
		GConfig->RemoveKey(TEXT("/Script/CognitiveVR.CognitiveVRSceneSettings"), TEXT("SceneData"), TestSyncFile);
		//GConfig->Remove(
		GConfig->SetArray(TEXT("/Script/CognitiveVR.CognitiveVRSceneSettings"), TEXT("SceneData"), iniscenedata, TestSyncFile);
		GConfig->Flush(false, GEngineIni);
		ConfigFileHasChanged = true;
	}
	else
	{
		GLog->Log("FCognitiveToolsCustomization::SceneVersionResponse failed to parse json response");
	}
}

/*void FCognitiveTools::OnSceneVersionGetAuthToken(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		auto scene = GetCurrentSceneData();
		if (scene.IsValid())
		{
			SceneVersionRequest(*scene);
		}
	}
}*/

TArray<TSharedPtr<FEditorSceneData>> FCognitiveTools::GetSceneData() const
{
	return SceneData;
}

// Callback for determining whether a radio button is checked.
ECheckBoxState FCognitiveTools::HandleRadioButtonIsChecked(EReleaseType ButtonId) const
{
	return (RadioChoice == ButtonId)
		? ECheckBoxState::Checked
		: ECheckBoxState::Unchecked;
}

ECheckBoxState FCognitiveTools::HasFoundBlenderCheckbox() const
{
	return (HasFoundBlender())
		? ECheckBoxState::Checked
		: ECheckBoxState::Unchecked;
}

bool FCognitiveTools::HasLoggedIn() const
{
	return FAnalyticsCognitiveVR::Get().EditorSessionToken.Len() > 0;
}

bool FCognitiveTools::HasSelectedValidProduct() const
{
	if (!HasLoggedIn()) { return false; }
	return SelectedProduct.customerId.Len() > 0;
}

bool FCognitiveTools::HasLoadedOrSelectedValidProduct() const
{
	if (!HasLoggedIn()) { return false; }
	if (SelectedProduct.customerId.Len() > 0) { return true; }
	if (HasSavedCustomerId()) { return true; }
	return false;
}

bool FCognitiveTools::CustomerIdDoesntMatchFile() const
{
	FString customerid = GetCustomerIdFromFile();
	if (customerid != SelectedProduct.customerId)
	{
		return true;
	}
	return false;
}

FCognitiveTools::EReleaseType FCognitiveTools::GetReleaseTypeFromFile()
{
	FString customerid = FCognitiveTools::GetCustomerIdFromFile();

	if (customerid.Len() > 0)
	{
		if (customerid.EndsWith("-prod"))
		{
			return EReleaseType::Production;
		}
	}
	return EReleaseType::Test;
}

FReply FCognitiveTools::SaveCustomerIdToFile()
{
	FString CustomerId = SelectedProduct.customerId;

	if (RadioChoice == EReleaseType::Test)
	{
		CustomerId.Append("-test");
	}
	else
	{
		CustomerId.Append("-prod");
	}
	
	FString TestSyncFile = FPaths::Combine(*(FPaths::GameDir()), TEXT("Config/DefaultEngine.ini"));
	GLog->Log("FCognitiveTools::SaveCustomerIdToFile save: " + CustomerId);

	GConfig->SetString(TEXT("Analytics"), TEXT("ProviderModuleName"), TEXT("CognitiveVR"), TestSyncFile);
	GConfig->SetString(TEXT("Analytics"), TEXT("CognitiveVRApiKey"), *CustomerId, TestSyncFile);

	SaveProductNameToFile(SelectedProduct.name);

	GConfig->Flush(false, GEngineIni);

	ConfigFileHasChanged = true;

	return FReply::Handled();
}

void FCognitiveTools::OnEmailChanged(const FText& Text)
{
	Email = Text.ToString();
}

void FCognitiveTools::OnPasswordChanged(const FText& Text)
{
	Password = Text.ToString();
}

FReply FCognitiveTools::OpenProductOnDashboard()
{
	FString url = DashboardNewProduct(GetCustomerId().ToString());

	if (RadioChoice == EReleaseType::Test)
	{
		url.Append("-test");
	}
	else
	{
		url.Append("-prod");
	}

	FPlatformProcess::LaunchURL(*url, nullptr, nullptr);

	return FReply::Handled();
}

bool FCognitiveTools::EnableOpenProductOnDashboard() const
{
	return HasSelectedValidProduct();
}

void FCognitiveTools::OnProductChanged(TSharedPtr<FString> Selection, ESelectInfo::Type SelectInfo)
{
	if (!Selection.IsValid()) { return; }

	FString newProductName = *Selection;
	for (int i = 0; i < ProductInfos.Num(); i++)
	{
		if (newProductName == ProductInfos[i].name)
		{
			SelectedProduct = ProductInfos[i];
			return;
		}
	}
}

void FCognitiveTools::OnOrganizationChanged(TSharedPtr<FString> Selection, ESelectInfo::Type SelectInfo)
{
	if (!Selection.IsValid()) { return; }

	FString newOrgName = *Selection;

	//GLog->Log("organization selection changed " + newOrgName);

	FOrganizationData selectedOrg;

	for (int i = 0; i < OrganizationInfos.Num(); i++)
	{
		if (OrganizationInfos[i].name == newOrgName)
		{
			selectedOrg = OrganizationInfos[i];
			break;
		}
	}

	SaveOrganizationNameToFile(selectedOrg.name);

	ProductInfos.Empty();

	TArray<TSharedPtr<FJsonValue>> mainArray = JsonUserData->GetArrayField("products");
	for (int RowNum = 0; RowNum != mainArray.Num(); RowNum++) {
		TSharedPtr<FJsonObject> tempRow = mainArray[RowNum]->AsObject();
		if (tempRow->GetStringField("orgId") != selectedOrg.id)
		{
			continue;
		}
		FProductData tempProduct;
		tempProduct.id = tempRow->GetStringField("id");
		tempProduct.name = tempRow->GetStringField("name");
		tempProduct.orgId = tempRow->GetStringField("orgId");
		tempProduct.customerId = tempRow->GetStringField("customerId");
		ProductInfos.Add(tempProduct);

		AllProductNames.Add(MakeShareable(new FString(tempProduct.name)));
	}
}

TArray<TSharedPtr<FString>> FCognitiveTools::GetOrganizationNames()
{
	return AllOrgNames;
}

FReply FCognitiveTools::DEBUGPrintSessionId()
{
	FString editorSessionId = FAnalyticsCognitiveVR::Get().EditorSessionId;
	GLog->Log("FCognitiveTools::DEBUGPrintSessionId: " + editorSessionId);
	return FReply::Handled();
}

FReply FCognitiveTools::LogIn()
{
	//how to send request and listen for response?

	TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();

	HttpRequest->SetVerb("POST");
	HttpRequest->SetURL("https://api.cognitivevr.io/sessions");

	FString body = "{\"email\":\"" + Email + "\",\"password\":\"" + Password + "\"}";

	HttpRequest->SetHeader("Content-Type", TEXT("application/json"));

	HttpRequest->SetContentAsString(body);

	HttpRequest->OnProcessRequestComplete().BindSP(this, &FCognitiveTools::OnLogInResponse);

	if (Email.Len() == 0)
	{
		GLog->Log("Email length is 0");
		return FReply::Handled();
	}
	if (Password.Len() == 0)
	{
		GLog->Log("Password length is 0");
		return FReply::Handled();
	}

	GLog->Log("FCognitiveTools::LogIn Send login request!");

	HttpRequest->ProcessRequest();
	return FReply::Handled();
}

void FCognitiveTools::OnLogInResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (Response.IsValid())
	{
		//GLog->Log("Login Response "+Response->GetContentAsString());
		//GLog->Log("Login error code"+FString::FromInt(Response->GetResponseCode()));
		if (Response->GetResponseCode() == 201)
		{
			FAnalyticsCognitiveVR::Get().EditorSessionToken = Response->GetHeader("Set-Cookie");
			//request auth token

			TArray<FString> Array;
			FString MyString(Response->GetHeader("Set-Cookie"));
			MyString.ParseIntoArray(Array, TEXT(";"), true);

			FAnalyticsCognitiveVR::Get().EditorSessionId = Array[0].RightChop(18);
			//GLog->Log("token " + FAnalyticsCognitiveVR::Get().EditorSessionToken);
			//GLog->Log("id " + FAnalyticsCognitiveVR::Get().EditorSessionId);
			
			//parse login response to userdata
			//read organization names from that
			
			TSharedRef<TJsonReader<>>Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
			if (FJsonSerializer::Deserialize(Reader, JsonUserData))
			{
				TArray<TSharedPtr<FJsonValue>> mainArray = JsonUserData->GetArrayField("organizations");
				for (int RowNum = 0; RowNum != mainArray.Num(); RowNum++) {
					FOrganizationData tempOrg;
					TSharedPtr<FJsonObject> tempRow = mainArray[RowNum]->AsObject();
					tempOrg.id = tempRow->GetStringField("id");
					tempOrg.name = tempRow->GetStringField("name");
					tempOrg.prefix = tempRow->GetStringField("prefix");
					OrganizationInfos.Add(tempOrg);

					AllOrgNames.Add(MakeShareable(new FString(tempOrg.name)));
				}

				//GLog->Log("found this many organizations: "+FString::FromInt(OrganizationInfos.Num()));

				AuthTokenRequest();
			}
			GLog->Log("Log In Successful");
		}
		else if (Response->GetResponseCode() >= 500)
		{
			GLog->Log("FCognitiveTools::OnLogInResponse internal server error");
		}
		else
		{
			GLog->Log("Email or Password is incorrect");
		}
	}
	else
	{
		GLog->Log("Login Response is null");
	}
}

FReply FCognitiveTools::DEBUG_RequestAuthToken()
{
	AuthTokenRequest();
	return FReply::Handled();
}

/*TSharedRef<IHttpRequest> FCognitiveTools::RequestAuthTokenCallback()
{
	TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();

	TSharedPtr<FEditorSceneData> currentscenedata = GetCurrentSceneData();
	if (!currentscenedata.IsValid())
	{
		GLog->Log("FCogntiveTools::AuthTokenRequest cannot find current scene data");
		return HttpRequest;
	}
	if (FAnalyticsCognitiveVR::Get().EditorSessionToken.Len() == 0)
	{
		GLog->Log("FCogntiveTools::AuthTokenRequest session token is invalid. Please Log in");
		return HttpRequest;
	}

	//GLog->Log("FCognitiveTools::AuthTokenRequest send auth token request");
	//GLog->Log("url "+PostAuthToken(currentscenedata->Id));
	//GLog->Log("cookie " + FAnalyticsCognitiveVR::Get().EditorSessionToken);

	HttpRequest->SetVerb("POST");
	HttpRequest->SetHeader("Cookie", FAnalyticsCognitiveVR::Get().EditorSessionToken);
	HttpRequest->SetURL(PostAuthToken(currentscenedata->Id));
	HttpRequest->OnProcessRequestComplete().BindSP(this, &FCognitiveTools::AuthTokenResponse);
	HttpRequest->ProcessRequest();
	return HttpRequest;
}*/

void FCognitiveTools::AuthTokenRequest()
{
	TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();

	TSharedPtr<FEditorSceneData> currentscenedata = GetCurrentSceneData();
	if (!currentscenedata.IsValid())
	{
		GLog->Log("FCogntiveTools::AuthTokenRequest cannot find current scene data");
		return;
	}
	if (FAnalyticsCognitiveVR::Get().EditorSessionToken.Len() == 0)
	{
		GLog->Log("FCogntiveTools::AuthTokenRequest session token is invalid. Please Log in");
		return;
	}

	//GLog->Log("FCognitiveTools::AuthTokenRequest send auth token request");
	//GLog->Log("url "+PostAuthToken(currentscenedata->Id));
	//GLog->Log("cookie " + FAnalyticsCognitiveVR::Get().EditorSessionToken);

	HttpRequest->SetVerb("POST");
	HttpRequest->SetHeader("Cookie", FAnalyticsCognitiveVR::Get().EditorSessionToken);
	HttpRequest->SetURL(PostAuthToken(currentscenedata->Id));
	HttpRequest->OnProcessRequestComplete().BindSP(this, &FCognitiveTools::AuthTokenResponse);
	HttpRequest->ProcessRequest();
}

void FCognitiveTools::AuthTokenResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!Response.IsValid())
	{
		GLog->Log("FCognitiveToolsCustomization::AuthTokenResponse response isn't valid");
		return;
	}

	//GLog->Log("FCognitiveToolsCustomization::AuthTokenResponse response code " + FString::FromInt(Response->GetResponseCode()));
	//GLog->Log("FCognitiveToolsCustomization::AuthTokenResponse " + Response->GetContentAsString());

	if (bWasSuccessful)
	{
		TSharedPtr<FJsonObject> JsonAuthToken;
		TSharedRef<TJsonReader<>>Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
		if (FJsonSerializer::Deserialize(Reader, JsonAuthToken))
		{
			FString token = JsonAuthToken->GetStringField("token");
			FAnalyticsCognitiveVR::Get().EditorAuthToken = token;
		}
	}
}

FReply FCognitiveTools::ReexportDynamicMeshesCmd()
{
	ReexportDynamicMeshes(ExportDynamicsDirectory);
	return FReply::Handled();
}

FReply FCognitiveTools::ExportDynamicTextures()
{
	ConvertDynamicTextures();
	return FReply::Handled();
}

TSharedPtr<FEditorSceneData> FCognitiveTools::GetCurrentSceneData() const
{
	UWorld* myworld = GWorld->GetWorld();

	FString currentSceneName = myworld->GetMapName();
	currentSceneName.RemoveFromStart(myworld->StreamingLevelsPrefix);
	return GetSceneData(currentSceneName);
}

FString lastSceneName;
TSharedPtr<FEditorSceneData> FCognitiveTools::GetSceneData(FString scenename) const
{
	for (int i = 0; i < SceneData.Num(); i++)
	{
		if (!SceneData[i].IsValid()) { continue; }
		if (SceneData[i]->Name == scenename)
		{
			return SceneData[i];
		}
	}
	if (lastSceneName != scenename)
	{
		GLog->Log("FCognitiveToolsCustomization::GetSceneData couldn't find SceneData for scene " + scenename);
		lastSceneName = scenename;
	}
	return NULL;
}

FReply FCognitiveTools::DEBUGSendSceneData()
{
	SaveSceneData("FirstPersonExampleMap1234", "1234-asdf-5678-hjkl");
	return FReply::Handled();
}

void FCognitiveTools::SaveSceneData(FString sceneName, FString sceneKey)
{
	FString keyValue = sceneName + "," + sceneKey;

	TArray<FString> scenePairs = TArray<FString>();

	FString TestSyncFile = FPaths::Combine(*(FPaths::GameDir()), TEXT("Config/DefaultEngine.ini"));
	GConfig->GetArray(TEXT("/Script/CognitiveVR.CognitiveVRSceneSettings"), TEXT("SceneData"), scenePairs, TestSyncFile);

	bool didSetKey = false;
	for (int32 i = 0; i < scenePairs.Num(); i++)
	{
		FString name;
		FString key;
		scenePairs[i].Split(TEXT(","), &name, &key);
		if (*name == sceneName)
		{
			scenePairs[i] = keyValue;
			didSetKey = true;
			GLog->Log("FCognitiveToolsCustomization::SaveSceneData - found and replace key for scene " + name + " new value " + keyValue);
			break;
		}
	}
	if (!didSetKey)
	{
		scenePairs.Add(keyValue);
		GLog->Log("FCognitiveToolsCustomization::SaveSceneData - added new scene value and key for " + sceneName);
	}

	//remove scene names that don't have keys!
	for (int32 i = scenePairs.Num()-1; i >= 0; i--)
	{
		FString name;
		FString key;
		if (!scenePairs[i].Split(TEXT(","), &name, &key))
		{
			scenePairs.RemoveAt(i);
		}
	}

	GConfig->SetArray(TEXT("/Script/CognitiveVR.CognitiveVRSceneSettings"), TEXT("SceneData"), scenePairs, TestSyncFile);

	GConfig->Flush(false, GEngineIni);
}

#undef LOCTEXT_NAMESPACE