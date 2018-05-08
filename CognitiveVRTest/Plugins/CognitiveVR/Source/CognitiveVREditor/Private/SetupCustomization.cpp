#include "CognitiveVREditorPrivatePCH.h"
#include "SetupCustomization.h"

#define LOCTEXT_NAMESPACE "BaseToolEditor"

TSharedRef<IDetailCustomization> FSetupCustomization::MakeInstance()
{
	GLog->Log("make instance of customization");
	return MakeShareable(new FSetupCustomization());
}

EVisibility FSetupCustomization::ShouldDisplay(int32 page)
{
	return page == CurrentPage ? EVisibility::Visible : EVisibility::Collapsed;
}

void FSetupCustomization::CustomizeDetails(IDetailLayoutBuilder & DetailBuilder)
{

	GLog->Log("FSetupCustomization::CustomizeDetails");

	//DetailLayoutPtr = &DetailBuilder;


	IDetailCategoryBuilder& SettingsCategory = DetailBuilder.EditCategory(TEXT("Export Settings"));

	/*MinPolygonProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UCognitiveVRSettings, MinPolygons));
	MaxPolygonProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UCognitiveVRSettings, MaxPolygons));
	StaticOnlyProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UCognitiveVRSettings, staticOnly));
	MinSizeProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UCognitiveVRSettings, MinimumSize));
	MaxSizeProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UCognitiveVRSettings, MaximumSize));
	TextureResizeProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UCognitiveVRSettings, TextureResizeFactor));
	ExcludeMeshProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UCognitiveVRSettings, ExcludeMeshes));*/

	// Create a commands category
	IDetailCategoryBuilder& LoginCategory = DetailBuilder.EditCategory(TEXT("Account"),FText::GetEmpty(),ECategoryPriority::Important);
	
	//SAssignNew()
	
	LoginCategory.AddCustomRow(FText::FromString("Commands")).Visibility(ShouldDisplay(0))
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

		//+ SHorizontalBox::Slot()
		//.MaxWidth(128)
		//[
		//	SNew(SEditableTextBox)
		//	.MinDesiredWidth(128)
		//	.Text(this, &FCognitiveTools::GetDeveloperKey)
		//	.OnTextChanged(this,&FCognitiveTools::OnDeveloperKeyChanged)
		//]
	];

	LoginCategory.AddCustomRow(FText::FromString("Commands")).Visibility(ShouldDisplay(0))
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.HAlign(EHorizontalAlignment::HAlign_Center)
		[
			SNew(STextBlock)
			//.Visibility(this, &FCognitiveTools::ConfigFileChangedVisibility)
			.ColorAndOpacity(FLinearColor::Yellow)
			.Text(FText::FromString("Config files changed. Data displayed below may be incorrect until you restart Unreal Editor!"))
		]
	];

	LoginCategory.AddCustomRow(FText::FromString("Commands")).Visibility(ShouldDisplay(0))
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		//.AutoWidth()
		.HAlign(EHorizontalAlignment::HAlign_Center)
		[
			SNew(SButton)
			//.HAlign(EHorizontalAlignment::HAlign_Center)
			//.IsEnabled(this, &FCognitiveTools::HasDeveloperKey)
			.Text(FText::FromString("Get Latest Scene Version Data"))
			.ToolTip(SNew(SToolTip).Text(LOCTEXT("get scene data tip", "This will get the latest scene version data from Scene Explorer and saves it to your config files. Must restart Unreal Editor to see the changes in your config files here")))
			.OnClicked(this, &FSetupCustomization::ToolsDebugButton)
			
		]
	];

	LoginCategory.AddCustomRow(FText::FromString("CommandsA"))
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		[
			SNew(SButton).Visibility(EVisibility::Hidden)
		]
		+ SVerticalBox::Slot()
			[
				SNew(SButton).Visibility(EVisibility::Hidden)
			]
		+ SVerticalBox::Slot()
			[
				SNew(SButton).Visibility(EVisibility::Hidden)
			]
		+ SVerticalBox::Slot()
			[
				SNew(SButton).Visibility(EVisibility::Hidden)
			]
		+ SVerticalBox::Slot()
			[
				SNew(SButton).Visibility(EVisibility::Hidden)
			]
		+ SVerticalBox::Slot()
			[
				SNew(SButton).Visibility(EVisibility::Hidden)
			]
		+ SVerticalBox::Slot()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				//.AutoWidth()
				.HAlign(EHorizontalAlignment::HAlign_Center)
				[
					SNew(SButton)
					//.HAlign(EHorizontalAlignment::HAlign_Center)
					//.IsEnabled(this, &FCognitiveTools::HasDeveloperKey)
					.Text(FText::FromString("one"))
					.ToolTip(SNew(SToolTip).Text(LOCTEXT("one", "This will get the latest scene version data from Scene Explorer and saves it to your config files. Must restart Unreal Editor to see the changes in your config files here")))
					.OnClicked(this, &FSetupCustomization::ToolsDebugButton)
				]
			]
	];
	
	LoginCategory.AddCustomRow(FText::FromString("Commands8"))
		.Visibility(ShouldDisplay(0))
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		//.AutoWidth()
		.HAlign(EHorizontalAlignment::HAlign_Center)
		[
			SNew(SButton)
			//.HAlign(EHorizontalAlignment::HAlign_Center)
			//.IsEnabled(this, &FCognitiveTools::HasDeveloperKey)
			.Text(FText::FromString("one"))
			.ToolTip(SNew(SToolTip).Text(LOCTEXT("one", "This will get the latest scene version data from Scene Explorer and saves it to your config files. Must restart Unreal Editor to see the changes in your config files here")))
			.OnClicked(this, &FSetupCustomization::ToolsDebugButton)
		]
		+ SHorizontalBox::Slot()
		//.AutoWidth()
		.HAlign(EHorizontalAlignment::HAlign_Center)
		[
			SNew(SButton)
			//.HAlign(EHorizontalAlignment::HAlign_Center)
			//.IsEnabled(this, &FCognitiveTools::HasDeveloperKey)
			.Text(FText::FromString("two"))
			.ToolTip(SNew(SToolTip).Text(LOCTEXT("two", "This will get the latest scene version data from Scene Explorer and saves it to your config files. Must restart Unreal Editor to see the changes in your config files here")))
			.OnClicked(this, &FSetupCustomization::ToolsDebugButton)
		]
		+ SHorizontalBox::Slot()
		//.AutoWidth()
		.HAlign(EHorizontalAlignment::HAlign_Center)
		[
			SNew(SButton)
			//.HAlign(EHorizontalAlignment::HAlign_Center)
			//.IsEnabled(this, &FCognitiveTools::HasDeveloperKey)
			.Text(FText::FromString("three"))
			.ToolTip(SNew(SToolTip).Text(LOCTEXT("three", "This will get the latest scene version data from Scene Explorer and saves it to your config files. Must restart Unreal Editor to see the changes in your config files here")))
			.OnClicked(this, &FSetupCustomization::ToolsDebugButton)
		]
	];

	//uint32 NumChildren;
	/*
	StructPropertyHandle->GetNumChildren(NumChildren);

	for (uint32 ChildIndex = 0; ChildIndex < NumChildren; ++ChildIndex)
	{
		const TSharedRef< IPropertyHandle > ChildHandle = StructPropertyHandle->GetChildHandle(ChildIndex).ToSharedRef();

		if (ChildHandle->GetProperty()->GetName() == TEXT("SomeUProperty"))
		{
			SomeUPropertyHandle = ChildHandle;
		}
	}

	check(SomeUPropertyHandle.IsValid());



	HeaderRow.NameContent()
		//[
			//StructPropertyHandle->CreatePropertyNameWidget(TEXT("New property header name"), false)
		//]
		.ValueContent()
		.MinDesiredWidth(500)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("Extra info", "Some new representation"))
		.Font(IDetailLayoutBuilder::GetDetailFont())
		];*/
}

FReply FSetupCustomization::ToolsDebugButton()
{
	FCognitiveTools::GetInstance()->DebugButton();

	GLog->Log("pressed button");
	
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE