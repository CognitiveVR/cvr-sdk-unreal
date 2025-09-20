/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/

#include "MediaComponentDetails.h"
#include "C3DComponents/Media.h"
#include "CognitiveEditorTools.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonReader.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBox.h"
#include "Analytics.h"

#define LOCTEXT_NAMESPACE "Cognitive3DEditor"

TSharedRef<IDetailCustomization> IMediaComponentDetails::MakeInstance()
{
	return MakeShareable(new IMediaComponentDetails);
}

void IMediaComponentDetails::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
	// Get the selected objects
	TArray<TWeakObjectPtr<UObject>> SelectedObjects;
	DetailLayout.GetObjectsBeingCustomized(SelectedObjects);

	for (int32 ObjectIndex = 0; ObjectIndex < SelectedObjects.Num(); ++ObjectIndex)
	{
		TWeakObjectPtr<UObject> CurrentObject = SelectedObjects[ObjectIndex];
		if (CurrentObject.IsValid())
		{
			UMedia* CurrentMediaComponent = Cast<UMedia>(CurrentObject.Get());
			if (CurrentMediaComponent != nullptr)
			{
				SelectedMediaComponent = CurrentMediaComponent;
				break;
			}
		}
	}

	if (!SelectedMediaComponent.IsValid())
	{
		return;
	}

	// Get the property handles
	MediaSourceIdProperty = DetailLayout.GetProperty(GET_MEMBER_NAME_CHECKED(UMedia, SelectedMediaSourceId));
	MediaNameProperty = DetailLayout.GetProperty(GET_MEMBER_NAME_CHECKED(UMedia, MediaName));
	MediaIdProperty = DetailLayout.GetProperty(GET_MEMBER_NAME_CHECKED(UMedia, MediaId));
	MediaDescriptionProperty = DetailLayout.GetProperty(GET_MEMBER_NAME_CHECKED(UMedia, MediaDescription));

	// Hide the default property since we'll replace it with our custom widget
	DetailLayout.HideProperty(MediaSourceIdProperty);

	// Add custom category and widget
	IDetailCategoryBuilder& MediaCategory = DetailLayout.EditCategory("Media Source");

	// Add refresh button
	MediaCategory.AddCustomRow(LOCTEXT("RefreshMediaSources", "Refresh Media Sources"))
	.NameContent()
	[
		SNew(STextBlock)
		.Text(LOCTEXT("RefreshMediaSources", "Refresh Media Sources"))
		.Font(IDetailLayoutBuilder::GetDetailFont())
	]
	.ValueContent()
	.MaxDesiredWidth(200.f)
	.MinDesiredWidth(200.f)
	[
		SNew(SButton)
		.ContentPadding(1)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.Text(LOCTEXT("RefreshButton", "Refresh"))
		.ToolTipText(LOCTEXT("RefreshTooltip", "Refresh the list of available media sources"))
		.OnClicked_Lambda([this]() -> FReply
		{
			RequestMediaSources();
			return FReply::Handled();
		})
	];

	// Add dropdown for media source selection
	MediaCategory.AddCustomRow(LOCTEXT("SelectMediaSource", "Select Media Source"))
	.NameContent()
	[
		SNew(STextBlock)
		.Text(LOCTEXT("SelectMediaSource", "Select Media Source"))
		.Font(IDetailLayoutBuilder::GetDetailFont())
	]
	.ValueContent()
	.MaxDesiredWidth(400.f)
	.MinDesiredWidth(400.f)
	[
		SAssignNew(MediaSourceComboBox, SComboBox<TSharedPtr<FMediaSourceData>>)
		.OptionsSource(&MediaSources)
		.OnGenerateWidget(this, &IMediaComponentDetails::OnGenerateMediaSourceWidget)
		.OnSelectionChanged(this, &IMediaComponentDetails::OnMediaSourceSelectionChanged)
		.Content()
		[
			SNew(STextBlock)
			.Text(this, &IMediaComponentDetails::GetCurrentMediaSourceText)
		]
	];

	// Initialize by requesting media sources
	bIsRequestInProgress = false;
	RequestMediaSources();
}

void IMediaComponentDetails::RequestMediaSources()
{
	if (bIsRequestInProgress)
	{
		return;
	}

	bIsRequestInProgress = true;

	FString C3DSettingsPath = FCognitiveEditorTools::GetInstance()->GetSettingsFilePath();
	GConfig->LoadFile(C3DSettingsPath);

	FString Gateway = FAnalytics::Get().GetConfigValueFromIni(C3DSettingsPath, "/Script/Cognitive3D.Cognitive3DSettings", "Gateway", false);
	if (Gateway.Len() == 0)
	{
		Gateway = "data.cognitive3d.com";
	}

	// Construct the URL
	FString URL = "https://" + Gateway + "/v0/media";

	// Create HTTP request
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetVerb("GET");
	HttpRequest->SetURL(URL);

	// Set authentication header (APIKEY:DEVELOPER devkey)
	FString DeveloperKey = FCognitiveEditorTools::GetInstance()->DeveloperKey;
	if (!DeveloperKey.IsEmpty())
	{
		FString AuthHeader = FString::Printf(TEXT("APIKEY:DEVELOPER %s"), *DeveloperKey);
		HttpRequest->SetHeader(TEXT("Authorization"), AuthHeader);
	}

	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

	// Bind the response callback
	HttpRequest->OnProcessRequestComplete().BindSP(this, &IMediaComponentDetails::OnMediaSourcesRequestComplete);

	// Send the request
	HttpRequest->ProcessRequest();
}

void IMediaComponentDetails::OnMediaSourcesRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	bIsRequestInProgress = false;

	if (!bWasSuccessful || !Response.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to fetch media sources: Request unsuccessful"));
		return;
	}

	int32 ResponseCode = Response->GetResponseCode();
	if (ResponseCode < 200 || ResponseCode >= 300)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to fetch media sources: HTTP %d"), ResponseCode);
		return;
	}

	FString ResponseString = Response->GetContentAsString();

	// Parse JSON response
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseString);

	TArray<TSharedPtr<FJsonValue>> JsonArray;
	if (FJsonSerializer::Deserialize(Reader, JsonArray))
	{
		// Clear existing sources
		MediaSources.Empty();

		// Add "None" option
		MediaSources.Add(MakeShareable(new FMediaSourceData(TEXT(""), TEXT("None"), TEXT("No media source selected"), TEXT(""))));

		// Parse each media source
		for (const TSharedPtr<FJsonValue>& JsonValue : JsonArray)
		{
			TSharedPtr<FJsonObject> MediaObject = JsonValue->AsObject();
			if (MediaObject.IsValid())
			{
				FString Id = MediaObject->GetStringField(TEXT("uploadId"));
				FString Name = MediaObject->GetStringField(TEXT("name"));
				FString Description = MediaObject->GetStringField(TEXT("description"));
				FString FileName = MediaObject->GetStringField(TEXT("fileName"));

				MediaSources.Add(MakeShareable(new FMediaSourceData(Id, Name, Description, FileName)));
			}
		}

		// Refresh the combo box
		if (MediaSourceComboBox.IsValid())
		{
			MediaSourceComboBox->RefreshOptions();

			// Set the current selection based on the property value
			if (MediaSourceIdProperty.IsValid())
			{
				FString CurrentValue;
				if (MediaSourceIdProperty->GetValue(CurrentValue) == FPropertyAccess::Success)
				{
					for (const TSharedPtr<FMediaSourceData>& Source : MediaSources)
					{
						if (Source->Id == CurrentValue)
						{
							MediaSourceComboBox->SetSelectedItem(Source);
							break;
						}
					}
				}
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to parse media sources JSON response"));
	}
}

TSharedRef<SWidget> IMediaComponentDetails::OnGenerateMediaSourceWidget(TSharedPtr<FMediaSourceData> InOption)
{
	FString DisplayText = InOption->Name;
	if (!InOption->Description.IsEmpty() && !InOption->Id.IsEmpty())
	{
		DisplayText += FString::Printf(TEXT(" (%s)"), *InOption->Description);
	}

	return SNew(STextBlock)
		.Text(FText::FromString(DisplayText))
		.ToolTipText(FText::FromString(InOption->FileName));
}

void IMediaComponentDetails::OnMediaSourceSelectionChanged(TSharedPtr<FMediaSourceData> SelectedItem, ESelectInfo::Type SelectInfo)
{
	if (SelectedItem.IsValid())
	{
		// Update all the media properties
		if (MediaSourceIdProperty.IsValid())
		{
			MediaSourceIdProperty->SetValue(SelectedItem->Id);
		}

		if (MediaNameProperty.IsValid())
		{
			if (SelectedItem->Id.IsEmpty()) // "None" selection
			{
				MediaNameProperty->SetValue(FString());
			}
			else
			{
				MediaNameProperty->SetValue(SelectedItem->Name);
			}
		}

		if (MediaIdProperty.IsValid())
		{
			MediaIdProperty->SetValue(SelectedItem->Id);
		}

		if (MediaDescriptionProperty.IsValid())
		{
			if (SelectedItem->Id.IsEmpty()) // "None" selection
			{
				MediaDescriptionProperty->SetValue(FString());
			}
			else
			{
				MediaDescriptionProperty->SetValue(SelectedItem->Description);
			}
		}
	}
}

FText IMediaComponentDetails::GetCurrentMediaSourceText() const
{
	if (MediaSourceIdProperty.IsValid())
	{
		FString CurrentValue;
		if (MediaSourceIdProperty->GetValue(CurrentValue) == FPropertyAccess::Success)
		{
			for (const TSharedPtr<FMediaSourceData>& Source : MediaSources)
			{
				if (Source->Id == CurrentValue)
				{
					return FText::FromString(Source->Name);
				}
			}
		}
	}

	return LOCTEXT("NoSelection", "No selection");
}

#undef LOCTEXT_NAMESPACE