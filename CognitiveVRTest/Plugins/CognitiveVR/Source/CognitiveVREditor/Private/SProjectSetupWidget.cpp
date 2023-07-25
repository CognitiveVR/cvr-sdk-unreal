
#include "SProjectSetupWidget.h"

#define LOCTEXT_NAMESPACE "BaseToolEditor"

void SProjectSetupWidget::CheckForExpiredDeveloperKey(FString developerKey)
{
	auto Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindRaw(this, &SProjectSetupWidget::OnDeveloperKeyResponseReceived);
	FString gateway = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "Gateway", false);
	FString url = "https://" + gateway + "/v0/apiKeys/verify";
	Request->SetURL(url);
	Request->SetVerb("GET");
	Request->SetHeader(TEXT("Authorization"), "APIKEY:DEVELOPER " + developerKey);
	Request->ProcessRequest();
}

void SProjectSetupWidget::OnDeveloperKeyResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (Response.IsValid() == false)
	{
		SGenericDialogWidget::OpenDialog(FText::FromString("Your developer key has expired"), SNew(STextBlock).Text(FText::FromString("Please log in to the dashboard, select your project, and generate a new developer key.\n\nNote:\nDeveloper keys allow you to upload and modify Scenes, and the keys expire after 90 days.\nApplication keys authorize your app to send data to our server, and they never expire.")));
		GLog->Log("Developer Key Response is invalid. Developer key may be invalid or expired. Check your internet connection");
		return;
	}

	int32 responseCode = Response->GetResponseCode();
	if (responseCode == 200)
	{
		GLog->Log("Developer Key Response Code is 200");
		CurrentPageEnum = EPage::OrganizationDetails;
	}
	else
	{
		SGenericDialogWidget::OpenDialog(FText::FromString("Your developer key has expired"), SNew(STextBlock).Text(FText::FromString("Please log in to the dashboard, select your project, and generate a new developer key.\n\nNote:\nDeveloper keys allow you to upload and modify Scenes, and the keys expire after 90 days.\nApplication keys authorize your app to send data to our server, and they never expire.")));
		GLog->Log("Developer Key Response Code is not 200. Developer key may be invalid or expired");
	}
}

void SProjectSetupWidget::FetchApplicationKey(FString developerKey)
{
	auto HttpRequest = FHttpModule::Get().CreateRequest();

	FString Gateway = FString("data.cognitive3d.com");

	FString url = FString("https://" + Gateway + "/v0/applicationKey");
	HttpRequest->SetURL(url);
	HttpRequest->SetHeader("X-HTTP-Method-Override", TEXT("GET"));
	FString AuthValue = "APIKEY:DEVELOPER " + FAnalyticsCognitiveVR::Get().DeveloperKey;
	HttpRequest->SetHeader("Authorization", AuthValue);
	HttpRequest->SetHeader("Content-Type", "application/json");
	HttpRequest->OnProcessRequestComplete().BindRaw(this, &SProjectSetupWidget::GetApplicationKeyResponse);
	HttpRequest->ProcessRequest();
}

void SProjectSetupWidget::GetApplicationKeyResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	GLog->Log("SProjectSetupWidget::GetApplicationKeyResponse");

	FSuppressableWarningDialog::FSetupInfo Info(LOCTEXT("UpdateApplicationKeyBody", "Do you want to use the Application Key available from the Dashboard?"), LOCTEXT("UpdateApplicationKeyTitle", "Found Application Key"), "Cognitive3dApplicationKey");
	Info.ConfirmText = LOCTEXT("Yes", "Yes");
	Info.CancelText = LOCTEXT("No", "No");
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

void SProjectSetupWidget::FetchOrganizationDetails(FString developerKey)
{
	auto HttpRequest = FHttpModule::Get().CreateRequest();

	FString Gateway = FString("data.cognitive3d.com");

	FString url = FString("https://" + Gateway + "/v0/subscriptions");
	HttpRequest->SetURL(url);
	HttpRequest->SetHeader("X-HTTP-Method-Override", TEXT("GET"));
	FString AuthValue = "APIKEY:DEVELOPER " + FAnalyticsCognitiveVR::Get().DeveloperKey;
	HttpRequest->SetHeader("Authorization", AuthValue);
	HttpRequest->SetHeader("Content-Type", "application/json");
	HttpRequest->OnProcessRequestComplete().BindRaw(this, &SProjectSetupWidget::GetOrganizationDetailsResponse);
	HttpRequest->ProcessRequest();
}

void SProjectSetupWidget::GetOrganizationDetailsResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (Response->GetResponseCode() != 200) { return; }

	auto content = Response->GetContentAsString();
	FOrganizationDataResponse responseObject;
	FJsonObjectConverter::JsonObjectStringToUStruct(content, &responseObject, 0, 0);
	OrgNameTextBlock->SetText(FText::FromString("Organization Name: " + responseObject.organizationName));

	if (responseObject.subscriptions.Num() == 0)
	{
		GLog->Log("no subscriptions");
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


void SProjectSetupWidget::Construct(const FArguments& Args)
{
	DisplayAPIKey = FCognitiveEditorTools::GetInstance()->GetApplicationKey().ToString();
	DisplayDeveloperKey = FCognitiveEditorTools::GetInstance()->GetDeveloperKey().ToString();

	float padding = 10;

	ChildSlot
		[
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
						.Text(this, &SProjectSetupWidget::GetHeaderTitle)
					]
				]
			]

			+ SOverlay::Slot()
			[
				SNew(SVerticalBox)

#pragma region "intro screen"
			+ SVerticalBox::Slot()
			//.VAlign(VAlign_Center)
			.AutoHeight()
			.Padding(0,0,0, padding)
			[
				SNew(SRichTextBlock)
				.Justification(ETextJustify::Center)
				.Visibility(this, &SProjectSetupWidget::IsIntroVisible)
				.DecoratorStyleSet(&FEditorStyle::Get())
				.Text(FText::FromString("Welcome to the <RichTextBlock.BoldHighlight>Cognitive3D Scene Setup</>"))
			]
			+ SVerticalBox::Slot()
			//.VAlign(VAlign_Center)
			.AutoHeight()
			.Padding(0, 0, 0, padding)
			[
				SNew(STextBlock)
				.Visibility(this, &SProjectSetupWidget::IsIntroVisible)
				.Justification(ETextJustify::Center)
				.AutoWrapText(true)
				.Text(FText::FromString("This will guide you through the initial setup of your scene and will have produciton ready analytics at the end of this setup."))
			]
			+SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, padding)
			[
				SNew(SBox)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.WidthOverride(270)
				.HeightOverride(150)
				.Visibility(this, &SProjectSetupWidget::IsIntroVisible)
				[
					SNew(SButton)
					.OnClicked_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::OpenURL, FString("https://docs.cognitive3d.com/unreal/get-started/"))
					[
						SNew(SImage)
						.Image(this, &SProjectSetupWidget::GetVideoImage)
					]
				]
			]

#pragma endregion

#pragma region "key screen"

			+ SVerticalBox::Slot()
			//.VAlign(VAlign_Center)
			.AutoHeight()
				.Padding(0, 0, 0, padding)
			[
				SNew(SRichTextBlock)
				.Visibility(this, &SProjectSetupWidget::IsDevKeyVisible)
				.AutoWrapText(true)
				.Justification(ETextJustify::Center)
				.DecoratorStyleSet(&FEditorStyle::Get())
				.Text(FText::FromString("Please add your <RichTextBlock.BoldHighlight>Cognitive3D Project API Keys</> below to continue\nThese are available on the Project Dashboard"))
			]
			+ SVerticalBox::Slot()
			.MaxHeight(32)
				.AutoHeight()
				.Padding(0, 0, 0, padding)
			[
				SNew(SHorizontalBox)
				.Visibility(this, &SProjectSetupWidget::IsDevKeyVisible)
				+ SHorizontalBox::Slot()
				[
					SNew(SBox)
					.Visibility(this, &SProjectSetupWidget::IsDevKeyVisible)
					.WidthOverride(128)
					.HeightOverride(32)
					[
						SNew(STextBlock)
						.Visibility(this, &SProjectSetupWidget::IsDevKeyVisible)
						.Text(FText::FromString("Developer Key"))
					]
				]
				+ SHorizontalBox::Slot()
				[
					SNew(SBox)
					.Visibility(this, &SProjectSetupWidget::IsDevKeyVisible)
					.WidthOverride(128)
					.HeightOverride(32)
					[
						SNew(SEditableTextBox)
						.Visibility(this, &SProjectSetupWidget::IsDevKeyVisible)
						.Text(this, &SProjectSetupWidget::GetDisplayDeveloperKey)
						.OnTextChanged(this,&SProjectSetupWidget::OnDeveloperKeyChanged)
						//.OnTextCommitted()
					]
				]
			]

#pragma endregion

#pragma region Organization Details

			+ SVerticalBox::Slot()
			.MaxHeight(8)
			.AutoHeight()
			.Padding(0, 0, 0, padding)
			[
				SNew(STextBlock)
				.Visibility(this, &SProjectSetupWidget::IsOrganizationDetailsVisible)
			]
			
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, padding)
			[
				SAssignNew(OrgNameTextBlock, STextBlock)
				.Visibility(this, &SProjectSetupWidget::IsOrganizationDetailsVisible)
				.Justification(ETextJustify::Center)
				.Text(FText::FromString("Unknown"))
				//.Text(InArgs._LabelText)
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, padding)
			[
				SAssignNew(OrgTrialTextBlock, STextBlock)
				.Visibility(this, &SProjectSetupWidget::IsOrganizationDetailsVisible)
				.Justification(ETextJustify::Center)
				.Text(FText::FromString("Unknown"))
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, padding)
			[
				SAssignNew(OrgExpiryTextBlock, STextBlock)
				.Visibility(this, &SProjectSetupWidget::IsOrganizationDetailsVisible)
				.Justification(ETextJustify::Center)
				.Text(FText::FromString("Unknown"))
			]

			+ SVerticalBox::Slot()
			.MaxHeight(32)
			.AutoHeight()
			.Padding(0, 0, 0, padding)
			[
				SNew(SHorizontalBox)
				.Visibility(this, &SProjectSetupWidget::IsOrganizationDetailsVisible)
				+ SHorizontalBox::Slot()
				[
					SNew(SBox)
					.Visibility(this, &SProjectSetupWidget::IsOrganizationDetailsVisible)
					.WidthOverride(128)
					.HeightOverride(32)
					[
						SNew(STextBlock)
						.Visibility(this, &SProjectSetupWidget::IsOrganizationDetailsVisible)
						.Text(FText::FromString("Application Key"))
					]
				]

				+ SHorizontalBox::Slot()
				[
					SNew(SBox)
					.Visibility(this, &SProjectSetupWidget::IsOrganizationDetailsVisible)
					.WidthOverride(128)
					.HeightOverride(32)
					[
						SNew(SEditableTextBox)
						.Visibility(this, &SProjectSetupWidget::IsOrganizationDetailsVisible)
						.Text(this, &SProjectSetupWidget::GetDisplayAPIKey)
						.OnTextChanged(this,&SProjectSetupWidget::OnAPIKeyChanged)
					]
				]
			]


#pragma region "blender"
			+SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, padding)
			[
				SNew(SBox)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.WidthOverride(256)
				.HeightOverride(78)
				.Visibility(this, &SProjectSetupWidget::IsExportPathVisible)
				[
					SNew(SImage)
					.Visibility(this, &SProjectSetupWidget::IsExportPathVisible)
					.Image(this, &SProjectSetupWidget::GetBlenderLogo)
				]
			]


			+ SVerticalBox::Slot()
			.AutoHeight()
				.Padding(0, 0, 0, padding)
			[
				SNew(STextBlock)
				.Visibility(this, &SProjectSetupWidget::IsExportPathVisible)
				.Justification(ETextJustify::Center)
				.AutoWrapText(true)
				.Text(FText::FromString("When uploading your level to the dashboard, we use Blender to automatically prepare the scene.\nThis includes converting exported images to .pngs\nand reducing the polygon count of large meshes.\n\nWe also need a temporary Export Directory to save Unreal files to while we process them."))
			]

			+ SVerticalBox::Slot()
			.VAlign(VAlign_Center)
			.MaxHeight(30)
				.AutoHeight()
				.Padding(0, 0, 0, padding)
			[
				SNew(STextBlock)
				.Visibility(this, &SProjectSetupWidget::IsExportPathVisible)
				.Justification(ETextJustify::Center)
				.Text(FText::FromString("Blender is free and open source."))
			]

			+ SVerticalBox::Slot()
			.VAlign(VAlign_Center)
			.MaxHeight(30)
			.AutoHeight()
			.Padding(0, 0, 0, padding)
			[
				SNew(SHorizontalBox)
				.Visibility(this, &SProjectSetupWidget::IsExportPathVisible)
				+SHorizontalBox::Slot()
				.HAlign(HAlign_Center)
				[
					SNew(SButton)
					.HAlign(HAlign_Center)
					.Visibility(this,&SProjectSetupWidget::IsExportPathVisible)
					.Text(FText::FromString("www.blender.org"))
					.OnClicked_Raw(FCognitiveEditorTools::GetInstance(),&FCognitiveEditorTools::OpenURL,FString("https://www.blender.org"))
				]
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
				.Padding(0, 0, 0, padding)
			[
				SNew(SSeparator)
				.Visibility(this, &SProjectSetupWidget::IsExportPathVisible)
			]

			//path to blender
			+ SVerticalBox::Slot()
			.MaxHeight(32)
			.AutoHeight()
			.Padding(0, 0, 0, padding)
			[
				SNew(SHorizontalBox)
				.Visibility(this, &SProjectSetupWidget::IsExportPathVisible)
				+SHorizontalBox::Slot()
				.MaxWidth(200)
				[
					SNew(SBox)
					.Visibility(this, &SProjectSetupWidget::IsExportPathVisible)
					.HeightOverride(32)
					[
						SNew(STextBlock)
						.IsEnabled_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::HasDeveloperKey)
						.Text(FText::FromString("Path to Blender.exe"))
					]
				]
				+ SHorizontalBox::Slot()
				.FillWidth(3)
				.Padding(1)
				[
					SNew(SBox)
					.Visibility(this, &SProjectSetupWidget::IsExportPathVisible)
					.HeightOverride(32)
					.MaxDesiredHeight(32)
					[
						SNew(SEditableTextBox)
						.Text_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::GetBlenderPath)
						.OnTextChanged(this, &SProjectSetupWidget::OnBlenderPathChanged)
					]
				]
				+SHorizontalBox::Slot()
				.MaxWidth(17)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(FMargin(4.0f, 0.0f, 0.0f, 0.0f))
					.VAlign(VAlign_Center)
					[
						SNew(SBox)
						.Visibility(this, &SProjectSetupWidget::IsExportPathVisible)
						.HeightOverride(17)
						.WidthOverride(17)
						[
							SNew(SButton)
							.Visibility(this, &SProjectSetupWidget::IsExportPathVisible)
							//PickerWidget = SAssignNew(BrowseButton, SButton)
							.ButtonStyle(FEditorStyle::Get(), "HoverHintOnly")
							.ToolTipText(LOCTEXT("FolderButtonToolTipText", "Choose a directory from this computer"))
							.OnClicked_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::Select_Blender)
							.ContentPadding(2.0f)
							.ForegroundColor(FSlateColor::UseForeground())
							.IsFocusable(false)
							[
								SNew(SImage)
								.Visibility(this, &SProjectSetupWidget::IsExportPathVisible)
								.Image(FEditorStyle::GetBrush("PropertyWindow.Button_Ellipsis"))
								.ColorAndOpacity(FSlateColor::UseForeground())
							]
						]
					]
				]
			]

			//path to export directory
			+ SVerticalBox::Slot()
			.MaxHeight(32)
				.AutoHeight()
				.Padding(0, 0, 0, padding)
			[
				SNew(SHorizontalBox)
				.Visibility(this, &SProjectSetupWidget::IsExportPathVisible)
				+SHorizontalBox::Slot()
				.MaxWidth(200)
				[
					SNew(SBox)
					.Visibility(this, &SProjectSetupWidget::IsExportPathVisible)
					.HeightOverride(32)
					[
						SNew(STextBlock)
						.Visibility(this, &SProjectSetupWidget::IsExportPathVisible)
						.IsEnabled_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::HasDeveloperKey)
						.Text(FText::FromString("Path to Export Directory"))
					]
				]
				+ SHorizontalBox::Slot()
				.FillWidth(3)
				.Padding(1)
				[
					SNew(SBox)
					.Visibility(this, &SProjectSetupWidget::IsExportPathVisible)
					.HeightOverride(32)
					.MaxDesiredHeight(32)
					[
						SNew(SEditableTextBox)
						.Visibility(this, &SProjectSetupWidget::IsExportPathVisible)
						.Text_Raw(FCognitiveEditorTools::GetInstance(), &FCognitiveEditorTools::GetBaseExportDirectoryDisplay)
						.OnTextChanged(this, &SProjectSetupWidget::OnExportPathChanged)
						//SNew(STextBlock)
						//
					]
				]
				+SHorizontalBox::Slot()
				.MaxWidth(17)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(FMargin(4.0f, 0.0f, 0.0f, 0.0f))
					.VAlign(VAlign_Center)
					[
						SNew(SBox)
						.Visibility(this, &SProjectSetupWidget::IsExportPathVisible)
						.HeightOverride(17)
						.WidthOverride(17)
						[
							SNew(SButton)
							.Visibility(this, &SProjectSetupWidget::IsExportPathVisible)
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
			]
			
#pragma endregion

#pragma region complete

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, padding)
			[
				SNew(STextBlock)
				.Visibility(this, &SProjectSetupWidget::IsCompleteVisible)
				.Justification(ETextJustify::Center)
				.AutoWrapText(true)
				.Text(FText::FromString("The project settings are complete. Next you'll be guided to upload a scene to give context to the data you record."))
			]

			+ SVerticalBox::Slot()
			.VAlign(VAlign_Center)
			.MaxHeight(30)
			.AutoHeight()
			.Padding(0, 0, 0, padding)
			[
				SNew(SHorizontalBox)
				.Visibility(this, &SProjectSetupWidget::IsCompleteVisible)
				+SHorizontalBox::Slot()
				.HAlign(HAlign_Center)
				[
					SNew(SButton)
					.HAlign(HAlign_Center)
					.Visibility(this,&SProjectSetupWidget::IsCompleteVisible)
					.Text(FText::FromString("Open Scene Setup Window"))
					.OnClicked(this, &SProjectSetupWidget::OpenSceneSetupWindow)
					//.OnClicked_Raw(FCognitiveEditorTools::GetInstance(),&FCognitiveEditorTools::OpenURL,FString("https://www.blender.org"))
				]
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, padding)
			[
				SNew(STextBlock)
				.Visibility(this, &SProjectSetupWidget::IsCompleteVisible)
				.Justification(ETextJustify::Center)
				.AutoWrapText(true)
				.Text(FText::FromString("Alternatively, you can use Dynamic Object components to identify key actors in your experience."))
			]

			+ SVerticalBox::Slot()
			.VAlign(VAlign_Center)
			.MaxHeight(30)
			.AutoHeight()
			.Padding(0, 0, 0, padding)
			[
				SNew(SHorizontalBox)
				.Visibility(this, &SProjectSetupWidget::IsCompleteVisible)
				+SHorizontalBox::Slot()
				.HAlign(HAlign_Center)
				[
					SNew(SButton)
					.HAlign(HAlign_Center)
					.Visibility(this,&SProjectSetupWidget::IsCompleteVisible)
					.Text(FText::FromString("open dynamic object window"))
					.OnClicked(this, &SProjectSetupWidget::NextPage)
					//.OnClicked_Raw(FCognitiveEditorTools::GetInstance(),&FCognitiveEditorTools::OpenURL,FString("https://www.blender.org"))
				]
			]

#pragma endregion

			#pragma region dynamic object intro

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, padding)
			[
				SNew(STextBlock)
				.Visibility(this, &SProjectSetupWidget::IsDynamicObjectsVisible)
				.Justification(ETextJustify::Center)
				.AutoWrapText(true)
				.Text(FText::FromString("Dynamic Objects record engagements with various objects in your experience. This includes the positions of moving objects and if/how a user gazes on an object. These can be used with Objectives to quickly evalute your users' performance.\n\nSome examples include Billboards, Vehicles or Tools.\n\nThe next screen is an overview of all the Dynamic Objects in your scene and what Dynamic Objects already exist on the dashboard.\n\nFor now, simply add Dynamic Object components to your key Actors"))
			]

			+ SVerticalBox::Slot()
			.VAlign(VAlign_Center)
			.MaxHeight(30)
			.AutoHeight()
			.Padding(0, 0, 0, padding)
			[
				SNew(SHorizontalBox)
				.Visibility(this, &SProjectSetupWidget::IsDynamicObjectsVisible)
				+SHorizontalBox::Slot()
				.HAlign(HAlign_Center)
				[
					SNew(SButton)
					.HAlign(HAlign_Center)
					.Visibility(this,&SProjectSetupWidget::IsDynamicObjectsVisible)
					.Text(FText::FromString("Open Dynamic Object Window"))
					.OnClicked(this, &SProjectSetupWidget::OpenDynamicObjectWindow)
					//.OnClicked_Raw(FCognitiveEditorTools::GetInstance(),&FCognitiveEditorTools::OpenURL,FString("https://www.blender.org"))
				]
			]
			
#pragma endregion

			]

#pragma region "footer"

			+ SOverlay::Slot()
			.VAlign(VAlign_Bottom)
			.HAlign(HAlign_Right)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				[
					SNew(SBox)
					.WidthOverride(128)
					.HeightOverride(32)
					[
						SNew(SButton)
						.Text(FText::FromString("Back"))
						.Visibility(this,&SProjectSetupWidget::BackButtonVisibility)
						.OnClicked(this, &SProjectSetupWidget::LastPage)
					]
				]
				+ SHorizontalBox::Slot()
				[
					SNew(SBox)
					.WidthOverride(128)
					.HeightOverride(32)
					[
						SNew(SButton)
						.Text(this, &SProjectSetupWidget::NextButtonText)
						.IsEnabled(this,&SProjectSetupWidget::NextButtonEnabled)
						.Visibility(this, &SProjectSetupWidget::NextButtonVisibility)
						.OnClicked(this, &SProjectSetupWidget::NextPage)
					]
				]
			]
			//DEBUG NEXT/BACK BUTTONS
			//+ SOverlay::Slot()
			//.VAlign(VAlign_Bottom)
			//.HAlign(HAlign_Left)
			//[
			//	SNew(SHorizontalBox)
			//	+ SHorizontalBox::Slot()
			//	[
			//		SNew(SBox)
			//		.WidthOverride(128)
			//		.HeightOverride(32)
			//		[
			//			SNew(SButton)
			//			.Text(FText::FromString("Debug Back"))
			//			.OnClicked(this, &SProjectSetupWidget::DebugPreviousPage)
			//		]
			//	]
			//	+ SHorizontalBox::Slot()
			//	[
			//		SNew(SBox)
			//		.WidthOverride(128)
			//		.HeightOverride(32)
			//		[
			//			SNew(SButton)
			//			.Text(FText::FromString("Debug Next"))
			//			.OnClicked(this, &SProjectSetupWidget::DebugNextPage)
			//		]
			//	]
			//]

#pragma endregion
		];

		FString texturepath = IPluginManager::Get().FindPlugin(TEXT("CognitiveVR"))->GetBaseDir() / TEXT("Resources") / TEXT("blender_logo_socket_small.png");
		FName BrushName = FName(*texturepath);
		BlenderLogoTexture = new FSlateDynamicImageBrush(BrushName, FVector2D(256, 78));

		texturepath = IPluginManager::Get().FindPlugin(TEXT("CognitiveVR"))->GetBaseDir() / TEXT("Resources") / TEXT("getting_started_video.png");
		BrushName = FName(*texturepath);
		VideoImage = new FSlateDynamicImageBrush(BrushName, FVector2D(270, 150));
}

const FSlateBrush* SProjectSetupWidget::GetBlenderLogo() const
{
	return BlenderLogoTexture;
}

const FSlateBrush* SProjectSetupWidget::GetVideoImage() const
{
	return VideoImage;
}

FText SProjectSetupWidget::GetDisplayAPIKey() const
{
	return FText::FromString(DisplayAPIKey);
}

FText SProjectSetupWidget::GetDisplayDeveloperKey() const
{
	return FText::FromString(DisplayDeveloperKey);
}

EVisibility SProjectSetupWidget::IsIntroVisible() const
{
	return CurrentPageEnum == EPage::Intro ? EVisibility::Visible : EVisibility::Collapsed;
}
EVisibility SProjectSetupWidget::IsDevKeyVisible() const
{
	return CurrentPageEnum == EPage::DeveloperKey ? EVisibility::Visible : EVisibility::Collapsed;
}
EVisibility SProjectSetupWidget::IsOrganizationDetailsVisible() const
{
	return CurrentPageEnum == EPage::OrganizationDetails ? EVisibility::Visible : EVisibility::Collapsed;
}
EVisibility SProjectSetupWidget::IsExportPathVisible() const
{
	return CurrentPageEnum == EPage::ExportPath ? EVisibility::Visible : EVisibility::Collapsed;
}
EVisibility SProjectSetupWidget::IsCompleteVisible() const
{
	return CurrentPageEnum == EPage::Complete ? EVisibility::Visible : EVisibility::Collapsed;
}
EVisibility SProjectSetupWidget::IsDynamicObjectsVisible() const
{
	return CurrentPageEnum == EPage::DynamicObjects ? EVisibility::Visible : EVisibility::Collapsed;
}

void SProjectSetupWidget::OnAPIKeyChanged(const FText& Text)
{
	DisplayAPIKey = Text.ToString();
}

void SProjectSetupWidget::OnDeveloperKeyChanged(const FText& Text)
{
	DisplayDeveloperKey = Text.ToString();
}

FReply SProjectSetupWidget::DebugNextPage()
{
	CurrentPageEnum = (EPage)(((uint8)CurrentPageEnum) + 1);
	return FReply::Handled();
}
FReply SProjectSetupWidget::DebugPreviousPage()
{
	CurrentPageEnum = (EPage)(((uint8)CurrentPageEnum) - 1);
	return FReply::Handled();
}

FReply SProjectSetupWidget::NextPage()
{
	if (CurrentPageEnum == EPage::DeveloperKey)
	{
		CheckForExpiredDeveloperKey(DisplayDeveloperKey);

		FCognitiveEditorTools::GetInstance()->SaveDeveloperKeyToFile(DisplayDeveloperKey);
		FCognitiveEditorTools::GetInstance()->CurrentSceneVersionRequest();
		FetchApplicationKey(DisplayDeveloperKey);
		FetchOrganizationDetails(DisplayDeveloperKey);
		GLog->Log("SProjectSetupWidget::NextPage");
		return FReply::Handled();
	}
	if (CurrentPageEnum == EPage::ExportPath)
	{
		GLog->Log("set dynamic and scene export directories. create if needed");
		FCognitiveEditorTools::GetInstance()->CreateExportFolderStructure();
	}

	CurrentPageEnum = (EPage)(((uint8)CurrentPageEnum) + 1);

	return FReply::Handled();
}

FReply SProjectSetupWidget::LastPage()
{
	if (CurrentPageEnum == EPage::Intro) { return FReply::Handled(); }
	CurrentPageEnum = (EPage)(((uint8)CurrentPageEnum) - 1);
	return FReply::Handled();
}

EVisibility SProjectSetupWidget::NextButtonVisibility() const
{
	if (CurrentPageEnum == EPage::Complete || CurrentPageEnum == EPage::DynamicObjects)
	{
		return EVisibility::Hidden;
	}
	return EVisibility::Visible;
}
FText SProjectSetupWidget::NextButtonText() const
{
	if (CurrentPageEnum == EPage::DeveloperKey)
	{
		return FText::FromString("Validate");
	}
	return FText::FromString("Next");
}

bool SProjectSetupWidget::NextButtonEnabled() const
{
	if (CurrentPageEnum == EPage::DeveloperKey)
	{
		if (DisplayDeveloperKey.Len() > 0)
			return true;
		return false;
	}

	if (CurrentPageEnum == EPage::ExportPath)
	{
		if (FCognitiveEditorTools::GetInstance()->HasFoundBlender() && FCognitiveEditorTools::GetInstance()->BaseExportDirectory.Len() > 0)
		{
			return true;
		}
		return false;
	}

	return true;
}

EVisibility SProjectSetupWidget::BackButtonVisibility() const
{
	if (CurrentPageEnum == EPage::Intro)
	{
		return EVisibility::Hidden;
	}
	return EVisibility::Visible;
}

FText SProjectSetupWidget::GetHeaderTitle() const
{
	switch (CurrentPageEnum)
	{
	case SProjectSetupWidget::EPage::Intro:
		return FText::FromString("<RichTextBlock.BoldHighlight>STEP 1 - WELCOME</>");
		break;
	case SProjectSetupWidget::EPage::DeveloperKey:
		return FText::FromString("<RichTextBlock.BoldHighlight>STEP 2 - AUTHENTICATION</>");
		break;
	case SProjectSetupWidget::EPage::OrganizationDetails:
		return FText::FromString("<RichTextBlock.BoldHighlight>STEP 3 - ORGANIZATION DETAILS</>");
		break;
	case SProjectSetupWidget::EPage::ExportPath:
		return FText::FromString("<RichTextBlock.BoldHighlight>STEP 4 - EXPORT</>");
		break;
	case SProjectSetupWidget::EPage::Complete:
		return FText::FromString("<RichTextBlock.BoldHighlight>STEP 5 - NEXT STEPS</>");
		break;
	case SProjectSetupWidget::EPage::DynamicObjects:
		return FText::FromString("<RichTextBlock.BoldHighlight>STEP 6 - DYNAMIC OBJECTS</>");
		break;
	default:
		return FText::FromString("<RichTextBlock.BoldHighlight>UNKNOWN</>");
		break;
	}
}

void SProjectSetupWidget::OnBlenderPathChanged(const FText& Text)
{
	FCognitiveEditorTools::GetInstance()->BlenderPath = Text.ToString();
}

void SProjectSetupWidget::OnExportPathChanged(const FText& Text)
{
	FCognitiveEditorTools::GetInstance()->BaseExportDirectory = Text.ToString();
}

FReply SProjectSetupWidget::OpenSceneSetupWindow()
{
	FCognitiveVREditorModule::SpawnCognitiveSceneSetupTab();
	return FReply::Handled();
}

FReply SProjectSetupWidget::OpenDynamicObjectWindow()
{
	FCognitiveVREditorModule::SpawnCognitiveDynamicTab();
	return FReply::Handled();
}
