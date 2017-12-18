
#include "CognitiveVREditorPrivatePCH.h"
#include "CognitiveTools.h"

#define LOCTEXT_NAMESPACE "BaseToolEditor"

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
		if (UObject* Instance = WeakObject.Get())
		{
			Class = Instance->GetClass();
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


	IDetailCategoryBuilder& LightingCategory = DetailBuilder.EditCategory(TEXT("Debugging"));
	LightingCategory.AddProperty(StaticOnlyProperty);
	LightingCategory.AddProperty(StaticOnlyProperty);
	LightingCategory.AddProperty(StaticOnlyProperty);

	/*SettingsCategory.AddProperty(MinPolygonProperty);
	SettingsCategory.AddProperty(MaxPolygonProperty);
	SettingsCategory.AddProperty(StaticOnlyProperty);
	SettingsCategory.AddProperty(MinSizeProperty);
	SettingsCategory.AddProperty(MaxSizeProperty);
	SettingsCategory.AddProperty(TextureResizeProperty);*/

	// Create a commands category
	IDetailCategoryBuilder& LoginCategory = DetailBuilder.EditCategory(TEXT("Account"),FText::GetEmpty(),ECategoryPriority::Important);

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
				.Text(FText::FromString("Email"))
			]

			+ SHorizontalBox::Slot()
			.MaxWidth(128)
			//.Padding(4)
			[
				SNew(SEditableTextBox)
				.MinDesiredWidth(128)
				.OnTextChanged(this,&FCognitiveTools::OnEmailChanged)
			]
		];

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
				.Text(FText::FromString("Password"))
			]

			+ SHorizontalBox::Slot()
			.MaxWidth(128)
			//.Padding(4)
			[
				SNew(SEditableTextBox)
				.IsPassword(true)
				.MinDesiredWidth(128)
				.OnTextChanged(this,&FCognitiveTools::OnPasswordChanged)
			]

			+ SHorizontalBox::Slot()
			.MaxWidth(128)
			[
				SNew(SButton)
				.IsEnabled(true)
				.Text(FText::FromString("Log In"))
				.OnClicked(this, &FCognitiveTools::LogIn)
			]
		];

	LoginCategory.AddCustomRow(FText::FromString("Commands"))
		.ValueContent()
		.MinDesiredWidth(256)
		[
			SNew(SButton)
			.Text(FText::FromString("DEBUG Print Session Id"))
			.OnClicked(this, &FCognitiveTools::PrintSessionId)
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
			//.Padding(4)
			[
				SNew(STextComboBox)
				.OptionsSource(&AllOrgNames)
				.IsEnabled(this, &FCognitiveTools::HasLoggedIn)
				//.OptionsSource(&tempOrgs)
				
				.OnSelectionChanged(this, &FCognitiveTools::OnOrganizationChanged)
				//.InitiallySelectedItem(GetOrganizationNameFromFile())
				//.InitiallySelectedItem(LowerBoundTypeSelectedItem)
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
			//.Padding(4)
			[
				SNew(STextComboBox)
				.IsEnabled(this,&FCognitiveTools::HasLoggedIn)
				.OptionsSource(&AllProductNames)
				//.ToolTip(SNew(SToolTip).Text(LOCTEXT("BaseColorFBXImportToolTip", "this is a tooltip")))
				.OnSelectionChanged(this, &FCognitiveTools::OnProductChanged)
				//.InitiallySelectedItem(GetProductNameFromFile())
			]
		];

	LoginCategory.AddCustomRow(FText::FromString("Commands"))
		.ValueContent()
		.MinDesiredWidth(256)
		[
			SNew(STextBlock)
			.Text(FText::FromString(FCognitiveTools::GetCustomerIdFromFile()))
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
				//.Style(FCoreStyle::Get(), "RadioButton")
				.IsEnabled(this, &FCognitiveTools::HasSelectedValidProduct)
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
				//.Style(FCoreStyle::Get(), "RadioButton")
				.IsEnabled(this, &FCognitiveTools::HasSelectedValidProduct)
				.IsChecked(this, &FCognitiveTools::HandleRadioButtonIsChecked, EReleaseType::Production)
				.OnCheckStateChanged(this, &FCognitiveTools::HandleRadioButtonCheckStateChanged, EReleaseType::Production)
				[
					SNew(STextBlock)
					.Text(FText::FromString("Production"))
				]
			]
		];

	LoginCategory.AddCustomRow(FText::FromString("Commands"))
		.ValueContent()
		.MinDesiredWidth(256)
		[
			SNew(SButton)
			.Text(FText::FromString("Save ID to file"))
			.IsEnabled(this,&FCognitiveTools::HasSelectedValidProduct)
			.OnClicked(this, &FCognitiveTools::SaveCustomerIdToFile)
		];

	//refresh doens't usually reload anything. unclear how unreal can reload ini files
	LoginCategory.AddCustomRow(FText::FromString("Commands"))
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.HAlign(EHorizontalAlignment::HAlign_Right)
			.MaxWidth(64)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.MaxHeight(24)
				[
					SNew(SButton)
					.IsEnabled(true)
					.Text(FText::FromString("Refresh"))
					.OnClicked(this, &FCognitiveTools::RefreshSceneData)
				]
			]
		];

	LoginCategory.AddCustomRow(FText::FromString("Commands"))
		//.ValueContent()
		//.MinDesiredWidth(896)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
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
					.FillWidth(1)
					[
						SNew(STextBlock)
						.Text(FText::FromString("Version Number"))
					]

					+ SHeaderRow::Column("version id")
					.FillWidth(1)
					[
						SNew(STextBlock)
						.Text(FText::FromString("Version Id"))
					]

					+ SHeaderRow::Column("Open in Browser")
					.FillWidth(0.2)
					[
						SNew(STextBlock)
						.Text(FText::FromString("Open"))
					]
				)
			]
		];

	LoginCategory.AddCustomRow(FText::FromString("Commands"))
		.ValueContent()
		.MinDesiredWidth(256)
		[
			SNew(SButton)
			.IsEnabled(true)
			.Text(FText::FromString("DEBUG Get Auth Token"))
			.OnClicked(this, &FCognitiveTools::DEBUG_RequestAuthToken)
		];

	LoginCategory.AddCustomRow(FText::FromString("Commands"))
		.ValueContent()
		.MinDesiredWidth(256)
		[
			SNew(SButton)
			.IsEnabled(true)
		.Text(FText::FromString("DEBUG REFRESH CURRENT SCENE"))
		.OnClicked(this, &FCognitiveTools::DebugRefreshCurrentScene)
		];

	// Create a commands category
	IDetailCategoryBuilder& Category = DetailBuilder.EditCategory(TEXT("Scene Commands"));

	FText p = GetBlenderPath();
	if (p.EqualTo(FText::FromString("")) && !HasSearchedForBlender)
	{
		HasSearchedForBlender = true;
		SearchForBlender();
	}

	Category.AddCustomRow(FText::FromString("Select Blender Horizontal"))
		.ValueContent()
		.HAlign(HAlign_Fill)
		[
			SNew(SHorizontalBox)
				//button
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Left)
				.Padding(FMargin(0.0f, 0.0f, 30.0f, 0.0f))
				[
					SNew(SButton)
					.IsEnabled(true)
					.Text(FText::FromString("Select Blender"))
					.OnClicked(this, &FCognitiveTools::Select_Blender)
				]

				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Right)
				.Padding(FMargin(4.0f, 0.0f, 0.0f, 0.0f))
				[
					SNew(STextBlock)
					.Text(this, &FCognitiveTools::GetBlenderPath)
				]
		];

	//select export meshes
	Category.AddCustomRow(FText::FromString("Commands"))
		.ValueContent()
		.MinDesiredWidth(256)
		[
			SNew(SButton)
			.IsEnabled(this, &FCognitiveTools::HasFoundBlender)
			.Text(FText::FromString("Select Export Meshes"))
			.OnClicked(this, &FCognitiveTools::Select_Export_Meshes)
		];

	//export selected scene
	Category.AddCustomRow(FText::FromString("Commands"))
		.ValueContent()
		.MinDesiredWidth(256)
		[
			SNew(SButton)
			.IsEnabled(this, &FCognitiveTools::HasFoundBlender)
			.Text(FText::FromString("Export Selected"))
			.OnClicked(this, &FCognitiveTools::Export_Selected)
		];

	//export whole scene
	Category.AddCustomRow(FText::FromString("Commands"))
		.ValueContent()
		.MinDesiredWidth(256)
		[
			SNew(SButton)
			.IsEnabled(this, &FCognitiveTools::HasFoundBlender)
			.Text(FText::FromString("Export All"))
			.OnClicked(this, &FCognitiveTools::Export_All)
		];

	Category.AddCustomRow(FText::FromString("Select Export Directory"))
		.ValueContent()
		.HAlign(HAlign_Fill)
		[
			SNew(SHorizontalBox)
			//button
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			.Padding(FMargin(0.0f, 0.0f, 30.0f, 0.0f))
			[
				SNew(SButton)
				.IsEnabled(true)
				.Text(FText::FromString("Select Export Directory"))
				.OnClicked(this, &FCognitiveTools::Select_Export_Directory)
			]

			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Right)
			.Padding(FMargin(4.0f, 0.0f, 0.0f, 0.0f))
			[
				SNew(STextBlock)
				.Text(this, &FCognitiveTools::GetExportDirectory)
			]
		];

	//List Materials
	Category.AddCustomRow(FText::FromString("Commands"))
		.ValueContent()
		.MinDesiredWidth(256)
		[
			SNew(SButton)
			.IsEnabled(this, &FCognitiveTools::HasFoundBlenderAndExportDir)
			.Text(FText::FromString("Export Transparent Textures"))
			.OnClicked(this, &FCognitiveTools::List_Materials)
		];

	//Reduce Meshes
	Category.AddCustomRow(FText::FromString("Commands"))
		.ValueContent()
		.MinDesiredWidth(256)
		[
			SNew(SButton)
			.IsEnabled(this, &FCognitiveTools::HasFoundBlenderAndExportDir)
			.Text(FText::FromString("Reduce Meshes"))
			.OnClicked(this, &FCognitiveTools::Reduce_Meshes)
		];

	//Reduce Textures
	Category.AddCustomRow(FText::FromString("Commands"))
		.ValueContent()
		.MinDesiredWidth(256)
		[
			SNew(SButton)
			.IsEnabled(this, &FCognitiveTools::HasFoundBlenderAndExportDir)
			.Text(FText::FromString("Reduce Textures"))
			.OnClicked(this, &FCognitiveTools::Reduce_Textures)
		];



	//upload scene
	Category.AddCustomRow(FText::FromString("Commands"))
		.ValueContent()
		.MinDesiredWidth(256)
		[
			SNew(SButton)
			.IsEnabled(this, &FCognitiveTools::HasFoundBlenderAndExportDir)
			.Text(FText::FromString("Upload Scene"))
			.OnClicked(this, &FCognitiveTools::UploadScene)
		];

	// Create a commands category
	IDetailCategoryBuilder& DynamicsCategory = DetailBuilder.EditCategory(TEXT("Dynamic Object Commands"));

	//export all dynamics
	DynamicsCategory.AddCustomRow(FText::FromString("Commands"))
		.ValueContent()
		.MinDesiredWidth(256)
		[
			SNew(SButton)
			.IsEnabled(this, &FCognitiveTools::HasFoundBlender)
			.Text(FText::FromString("Export All Dynamic Objects"))
			.OnClicked(this, &FCognitiveTools::ExportDynamics)
		];

	//export selected dynamics
	DynamicsCategory.AddCustomRow(FText::FromString("Commands"))
		.ValueContent()
		.MinDesiredWidth(256)
		[
			SNew(SButton)
			.IsEnabled(this, &FCognitiveTools::HasFoundBlender)
			.Text(FText::FromString("Export Selected Dynamic Objects"))
			.OnClicked(this, &FCognitiveTools::ExportSelectedDynamics)
		];

	//select dynamic export directory
	DynamicsCategory.AddCustomRow(FText::FromString("Select Dynamic Directory"))
		.ValueContent()
		.HAlign(HAlign_Fill)
		[
			SNew(SHorizontalBox)
			//button
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			.Padding(FMargin(0.0f, 0.0f, 30.0f, 0.0f))
			[
				SNew(SButton)
				.IsEnabled(true)
				.Text(FText::FromString("Select Dynamic Directory"))
				.OnClicked(this, &FCognitiveTools::SelectDynamicsDirectory)
			]

			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Right)
			.Padding(FMargin(4.0f, 0.0f, 0.0f, 0.0f))
			[
				SNew(STextBlock)
				.Text(this, &FCognitiveTools::GetDynamicExportDirectory)
			]
		];

	//upload dynamics
	DynamicsCategory.AddCustomRow(FText::FromString("Commands"))
		.ValueContent()
		.MinDesiredWidth(256)
		[
			SNew(SButton)
			.IsEnabled(this, &FCognitiveTools::HasSetDynamicExportDirectory)
			.Text(FText::FromString("Upload Dynamic Objects"))
			.OnClicked(this, &FCognitiveTools::UploadDynamics)
		];

	IDetailCategoryBuilder& DynamicsManifestCategory = DetailBuilder.EditCategory(TEXT("Dynamic Object Manifest"));

	//upload dynamics
	DynamicsManifestCategory.AddCustomRow(FText::FromString("Commands"))
		.ValueContent()
		.MinDesiredWidth(256)
		[
			SNew(SButton)
			.IsEnabled(true)
			.Text(FText::FromString("Set Unique Dynamic Ids"))
			.OnClicked(this, &FCognitiveTools::SetUniqueDynamicIds)
		];
	
	//upload dynamics manifest for aggregation
	DynamicsManifestCategory.AddCustomRow(FText::FromString("Commands"))
		.ValueContent()
		.MinDesiredWidth(256)
		[
			SNew(SButton)
			.IsEnabled(true)
			.Text(FText::FromString("Upload Dynamic Manifest"))
			.OnClicked(this, &FCognitiveTools::UploadDynamicsManifest)
		];

	//upload dynamics manifest for aggregation
	DynamicsManifestCategory.AddCustomRow(FText::FromString("Commands"))
		.ValueContent()
		.MinDesiredWidth(256)
		[
			SNew(SButton)
			.IsEnabled(true)
			.Text(FText::FromString("Get Dynamic Manifest"))
			.OnClicked(this, &FCognitiveTools::GetDynamicsManifest)
		];

	DynamicsManifestCategory.AddCustomRow(FText::FromString("Commands"))
		.ValueContent()
		.MinDesiredWidth(256)
		[
			SNew(SButton)
			.IsEnabled(true)
			.Text(FText::FromString("DEBUG SEND SCENE DATA"))
			.OnClicked(this, &FCognitiveTools::DebugSendSceneData)
		];

	FCognitiveTools::RefreshSceneData();
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
			.FillWidth(1)
			.Padding(2.0f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(FString::FromInt(InItem->VersionNumber)))
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1)
			.Padding(2.0f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(FString::FromInt(InItem->VersionId)))
			]
			+ SHorizontalBox::Slot()
			.FillWidth(0.2)
			.Padding(2.0f)
			[
				SNew(SButton)
				.Text(FText::FromString("Open"))
				.OnClicked(this,&FCognitiveTools::OpenSceneInBrowser,InItem->Id)
			]
		];
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

FString FCognitiveTools::GetCustomerIdFromFile() const
{
	FString customerid;
	GConfig->GetString(TEXT("Analytics"), TEXT("CognitiveVRApiKey"), customerid, GEngineIni);
	return customerid;
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

	//GConfig->UnloadFile(GEngineIni);
	//GConfig->LoadFile(GEngineIni);

	TArray<FString>scenstrings;
	GConfig->GetArray(TEXT("/Script/CognitiveVR.CognitiveVRSceneSettings"), TEXT("SceneData"), scenstrings,GEngineIni);
	
	GLog->Log("GEngineIni is " + GEngineIni);

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
			GLog->Log("failed to parse " + scenstrings[i]);
			continue;
		}

		FEditorSceneData* tempscene = new FEditorSceneData(Array[0], Array[1], FCString::Atoi(*Array[2]), FCString::Atoi(*Array[3]));
		SceneData.Add(MakeShareable(tempscene));
	}
	
	GLog->Log("FCognitiveToolsCustomization::RefreshSceneData found this many scenes: " + FString::FromInt(SceneData.Num()));

	return FReply::Handled();
}

void FCognitiveTools::SceneVersionRequest(FEditorSceneData data)
{
	if (FAnalyticsCognitiveVR::Get().EditorAuthToken.Len() == 0)
	{
		GLog->Log("FCognitiveTools::SceneVersionRequest no auth token. TODO get auth token and retry");
		return;
	}

	TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();

	//HttpRequest->SetVerb("POST");
	HttpRequest->SetURL(GetSceneVersion(data.Id));

	//FString body = "{\"email\":\"" + Email + "\",\"password\":\"" + Password + "\"}";

	/*TArray<uint8> bodybytes;

	FTCHARToUTF8 ConverterEnd1(*body);
	auto enddata1 = (const uint8*)ConverterEnd1.Get();
	bodybytes.Append(enddata1, ConverterEnd1.Length());*/

	GLog->Log("url "+GetSceneVersion(data.Id));
	GLog->Log("auth token " + FAnalyticsCognitiveVR::Get().EditorAuthToken);

	//HttpRequest->SetHeader("Content-Type", TEXT("application/json"));
	HttpRequest->SetHeader("X-HTTP-Method-Override", TEXT("GET"));
	HttpRequest->SetHeader("Authorization", TEXT("Bearer " + FAnalyticsCognitiveVR::Get().EditorAuthToken));
	//HttpRequest->SetHeader(TEXT("X-HTTP-Method-Override"), TEXT("POST"));

	//HttpRequest->SetContentAsString(body);

	HttpRequest->OnProcessRequestComplete().BindSP(this, &FCognitiveTools::SceneVersionResponse);
	HttpRequest->ProcessRequest();
}

void FCognitiveTools::SceneVersionResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	int32 responseCode = Response->GetResponseCode();

	GLog->Log("FCognitiveToolsCustomization::GetSceneVersionResponse Code: " + FString::FromInt(responseCode));

	GLog->Log(Response->GetContentAsString());

	if (responseCode >= 500)
	{
		//internal server error
		GLog->Log("FCognitiveToolsCustomization::SceneVersionResponse 500-ish internal server error");
		return;
	}
	if (responseCode >= 400)
	{
		if (responseCode == 401)
		{
			//not authorized
			GLog->Log("FCognitiveToolsCustomization::SceneVersionResponse not authorized!");
			return;
		}
		else
		{
			//maybe no scene?
			GLog->Log("FCognitiveToolsCustomization::SceneVersionResponse some error. maybe no scene?");
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
			GLog->Log("FCognitiveToolsCustomization::SceneVersionResponse couldn't find a latest version in SceneVersion data");
			return;
		}

		//check that there is scene data in ini
		TSharedPtr<FEditorSceneData> currentSceneData = GetCurrentSceneData();
		if (!currentSceneData.IsValid())
		{
			GLog->Log("FCognitiveToolsCustomization::SceneVersionResponse can't find current scene data in ini files");
			return;
		}

		//get array of scene data
		TArray<FString> iniscenedata;

		GConfig->GetArray(TEXT("/Script/CognitiveVR.CognitiveVRSceneSettings"), TEXT("SceneData"), iniscenedata, GEngineIni);


		/*FConfigSection* Section = GConfig->GetSectionPrivate(TEXT("/Script/CognitiveVR.CognitiveVRSettings"), false, true, GEngineIni);
		if (Section == NULL)
		{
			GLog->Log("FCognitiveToolsCustomization::SceneVersionResponse can't find ini section header");
			return;
		}
		for (FConfigSection::TIterator It(*Section); It; ++It)
		{
			if (It.Key() == TEXT("SceneData"))
			{
				GLog->Log("scene data");
				FString name;
				FString key;
				It.Value().GetValue().Split(TEXT(","), &name, &key);
				iniscenedata.Add(It.Value().GetValue());
			}
			else if (It.Key() == TEXT("+SceneData"))
			{
				GLog->Log("+scene data");
				FString name;
				FString key;
				It.Value().GetValue().Split(TEXT(","), &name, &key);
				iniscenedata.Add(It.Value().GetValue());
			}
			else
			{
				GLog->Log("found something that's not a scene key " + It.Key().ToString());
			}
		}*/

		//TArray<FString> sceneidstuff;
		//GConfig->GetArray(TEXT("/Script/CognitiveVR.CognitiveVRSettings"), TEXT("SceneData"), sceneidstuff, GEngineIni);
		//GLog->Log("get array returned " + FString::FromInt(sceneidstuff.Num()));

		
		GLog->Log("found this many scene datas in ini " + FString::FromInt(iniscenedata.Num()));
		GLog->Log("looking for scene " + currentSceneData->Name);

		//update current scene
		for (int i = 0; i < iniscenedata.Num(); i++)
		{
			GLog->Log("looking at data " + iniscenedata[i]);

			TArray<FString> entryarray;
			iniscenedata[i].ParseIntoArray(entryarray, TEXT(","), true);

			if (entryarray[0] == currentSceneData->Name)
			{
				iniscenedata[i] = entryarray[0] + "," + entryarray[1] + "," + FString::FromInt(versionNumber) + "," + FString::FromInt(versionId);
				GLog->Log("FCognitiveToolsCustomization::SceneVersionResponse overwriting scene data to append versionnumber and versionid");
				GLog->Log(iniscenedata[i]);
				//GConfig->Remove(TEXT("/Script/CognitiveVR.CognitiveVRSettings"), TEXT("SceneData"),)
				//GConfig->Remove(TEXT("SceneData"));
				break;
			}
			else
			{
				//GLog->Log("found scene " + entryarray[0]);
			}
		}
		//set array to config
		GConfig->RemoveKey(TEXT("/Script/CognitiveVR.CognitiveVRSceneSettings"), TEXT("SceneData"), GEngineIni);
		//GConfig->Remove(
		GConfig->SetArray(TEXT("/Script/CognitiveVR.CognitiveVRSceneSettings"), TEXT("SceneData"), iniscenedata, GEngineIni);
		GConfig->Flush(false, GEngineIni);
	}
	else
	{
		GLog->Log("FCognitiveToolsCustomization::SceneVersionResponse failed to parse json response");
	}
}

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

bool FCognitiveTools::HasLoggedIn() const
{
	return FAnalyticsCognitiveVR::Get().EditorSessionToken.Len() > 0;
}

bool FCognitiveTools::HasSelectedValidProduct() const
{
	return SelectedProduct.customerId.Len() > 0;
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

//TODO load releasetype and selected customer+product from ini file

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
	
	GLog->Log("write customer id to ini: " + CustomerId);
	GConfig->SetString(TEXT("Analytics"), TEXT("ProviderModuleName"), TEXT("CognitiveVR"), GEngineIni);
	GConfig->SetString(TEXT("Analytics"), TEXT("CognitiveVRApiKey"), *CustomerId, GEngineIni);

	SaveProductNameToFile(SelectedProduct.name);

	GConfig->Flush(false, GEngineIni);

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

	GLog->Log("organization selection changed " + newOrgName);

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

FReply FCognitiveTools::PrintSessionId()
{
	FString editorSessionId = FAnalyticsCognitiveVR::Get().EditorSessionId;
	GLog->Log(editorSessionId);
	return FReply::Handled();
}

FReply FCognitiveTools::LogIn()
{
	//how to send request and listen for response?

	TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();

	HttpRequest->SetVerb("POST");
	HttpRequest->SetURL("https://api.cognitivevr.io/sessions");

	FString body = "{\"email\":\"" + Email + "\",\"password\":\"" + Password + "\"}";
	
	/*TArray<uint8> bodybytes;

	FTCHARToUTF8 ConverterEnd1(*body);
	auto enddata1 = (const uint8*)ConverterEnd1.Get();
	bodybytes.Append(enddata1, ConverterEnd1.Length());*/

	

	HttpRequest->SetHeader("Content-Type", TEXT("application/json"));
	//HttpRequest->SetHeader(TEXT("X-HTTP-Method-Override"), TEXT("POST"));

	HttpRequest->SetContentAsString(body);

	HttpRequest->OnProcessRequestComplete().BindSP(this, &FCognitiveTools::LogInResponse);

	GLog->Log("send login request!");
	GLog->Log(FString::FromInt(HttpRequest->GetContentLength()));

	if (Email.Len() == 0)
	{
		GLog->Log("email length is 0");
		return FReply::Handled();
	}
	if (Password.Len() == 0)
	{
		GLog->Log("password length is 0");
		return FReply::Handled();
	}

	GLog->Log("email and password length > 0");

	HttpRequest->ProcessRequest();
	return FReply::Handled();
}

void FCognitiveTools::LogInResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (Response.IsValid())
	{
		GLog->Log("Login Response "+Response->GetContentAsString());
		GLog->Log("Login error code"+FString::FromInt(Response->GetResponseCode()));
		if (Response->GetResponseCode() == 201)
		{
			FAnalyticsCognitiveVR::Get().EditorSessionToken = Response->GetHeader("Set-Cookie");
			//request auth token
			//AuthTokenRequest();


			TArray<FString> Array;
			FString MyString(Response->GetHeader("Set-Cookie"));
			MyString.ParseIntoArray(Array, TEXT(";"), true);

			FAnalyticsCognitiveVR::Get().EditorSessionId = Array[0].RightChop(18);
			GLog->Log("token " + FAnalyticsCognitiveVR::Get().EditorSessionToken);
			GLog->Log("id " + FAnalyticsCognitiveVR::Get().EditorSessionId);
			
			//parse login response to userdata
			//read organization names from that
			//OrganizationNames =
			
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

				GLog->Log("found this many organizations: "+FString::FromInt(OrganizationInfos.Num()));
			}
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

void FCognitiveTools::AuthTokenRequest()
{
	TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();

	TSharedPtr<FEditorSceneData> currentscenedata = GetCurrentSceneData();
	if (!currentscenedata.IsValid())
	{
		GLog->Log("FCogntiveToolsCustomization::AuthTokenRequest cannot find current scene data");
		return;
	}
	if (FAnalyticsCognitiveVR::Get().EditorSessionToken.Len() == 0)
	{
		GLog->Log("FCogntiveTools::AuthTokenRequest session token is invalid. log in");
		return;
	}

	GLog->Log("FCognitiveToolsCustomization::AuthTokenRequest send auth token request");
	GLog->Log("url "+PostAuthToken(currentscenedata->Id));
	GLog->Log("cookie " + FAnalyticsCognitiveVR::Get().EditorSessionToken);

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

	GLog->Log("FCognitiveToolsCustomization::AuthTokenResponse response code " + FString::FromInt(Response->GetResponseCode()));
	GLog->Log("FCognitiveToolsCustomization::AuthTokenResponse " + Response->GetContentAsString());

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


TSharedPtr<FEditorSceneData> FCognitiveTools::GetCurrentSceneData()
{
	UWorld* myworld = GWorld->GetWorld();

	FString currentSceneName = myworld->GetMapName();
	currentSceneName.RemoveFromStart(myworld->StreamingLevelsPrefix);
	return GetSceneData(currentSceneName);
}

TSharedPtr<FEditorSceneData> FCognitiveTools::GetSceneData(FString scenename)
{
	for (int i = 0; i < SceneData.Num(); i++)
	{
		if (!SceneData[i].IsValid()) { continue; }
		if (SceneData[i]->Name == scenename)
		{
			return SceneData[i];
		}
	}
	GLog->Log("FCognitiveToolsCustomization::GetSceneData couldn't find SceneData for scene " + scenename);
	return NULL;
}

FReply FCognitiveTools::DebugSendSceneData()
{
	SaveSceneData("FirstPersonExampleMap1234", "1234-asdf-5678-hjkl");
	return FReply::Handled();
}

void FCognitiveTools::SaveSceneData(FString sceneName, FString sceneKey)
{
	FString keyValue = sceneName + "," + sceneKey;
	UE_LOG(LogTemp, Warning, TEXT("Upload complete! Add this into the SceneData array in Project Settings:      %s"),*keyValue);


	TArray<FString> scenePairs = TArray<FString>();

	GConfig->GetArray(TEXT("/Script/CognitiveVR.CognitiveVRSceneSettings"), TEXT("SceneData"), scenePairs, GEngineIni);

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

	GConfig->SetArray(TEXT("/Script/CognitiveVR.CognitiveVRSceneSettings"), TEXT("SceneData"), scenePairs, GEngineIni);

	GConfig->Flush(false, GEngineIni);
	//GConfig->UnloadFile(GEngineIni);
	//GConfig->LoadFile(GEngineIni);
}

#undef LOCTEXT_NAMESPACE