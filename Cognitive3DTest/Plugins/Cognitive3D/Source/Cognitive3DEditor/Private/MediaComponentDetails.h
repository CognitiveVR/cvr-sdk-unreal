/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "DetailCategoryBuilder.h"
#include "PropertyHandle.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Http.h"
#include "UObject/WeakObjectPtr.h"

class UMedia;

struct FMediaSourceData
{
	FString Id;
	FString Name;
	FString Description;
	FString FileName;

	FMediaSourceData(FString InId, const FString& InName, const FString& InDescription, const FString& InFileName)
		: Id(InId), Name(InName), Description(InDescription), FileName(InFileName) {}
};

class IMediaComponentDetails : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

private:
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailLayout) override;

	// HTTP request handling
	void RequestMediaSources();
	void OnMediaSourcesRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	// Dropdown handling
	TSharedRef<SWidget> OnGenerateMediaSourceWidget(TSharedPtr<FMediaSourceData> InOption);
	void OnMediaSourceSelectionChanged(TSharedPtr<FMediaSourceData> SelectedItem, ESelectInfo::Type SelectInfo);
	FText GetCurrentMediaSourceText() const;

	// Data
	TWeakObjectPtr<UMedia> SelectedMediaComponent;
	TSharedPtr<IPropertyHandle> MediaSourceIdProperty;
	TSharedPtr<IPropertyHandle> MediaNameProperty;
	TSharedPtr<IPropertyHandle> MediaIdProperty;
	TSharedPtr<IPropertyHandle> MediaDescriptionProperty;
	TArray<TSharedPtr<FMediaSourceData>> MediaSources;
	TSharedPtr<SComboBox<TSharedPtr<FMediaSourceData>>> MediaSourceComboBox;
	bool bIsRequestInProgress;
};