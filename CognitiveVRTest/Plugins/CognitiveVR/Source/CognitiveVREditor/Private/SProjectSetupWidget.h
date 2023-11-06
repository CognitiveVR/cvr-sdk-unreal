#pragma once

#include "CognitiveEditorTools.h"
#include "CognitiveEditorData.h"
#include "CognitiveVRSettings.h"
#include "IDetailCustomization.h"
#include "PropertyEditing.h"
#include "PropertyCustomizationHelpers.h"
#include "Json.h"
#include "SCheckBox.h"
#include "STableRow.h"
#include "STextComboBox.h"
#include "SListView.h"
#include "SThrobber.h"
#include "Runtime/SlateCore/Public/Layout/Visibility.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "CognitiveVRActor.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include "C3DCommonEditorTypes.h"
#include "Misc/DateTime.h"
#include "CognitiveVREditorModule.h"

class FCognitiveTools;
class FCognitiveVREditorModule;

//screens:
//intro
//developer key
//application key result
//export path
//complete

class SProjectSetupWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SProjectSetupWidget){}
	SLATE_ATTRIBUTE(FString, OrganizationName)
		SLATE_ATTRIBUTE(FString, OrganizationSubscriptionType)
		SLATE_ATTRIBUTE(FString, OrganizationSubscriptionExpireDate)
		SLATE_ATTRIBUTE(bool, OrganizationInTrial)
	SLATE_END_ARGS()

	UENUM()
	enum class EProjectSetupPage : uint8
	{
		Intro,
		DeveloperKey,
		OrganizationDetails,
		ExportPath,
		Complete,
		DynamicObjects
	};
	EProjectSetupPage CurrentPageEnum;

	void Construct(const FArguments& Args);
	void CheckForExpiredDeveloperKey(FString developerKey);
	void OnDeveloperKeyResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	FString DisplayAPIKey;
	FText GetDisplayAPIKey() const;
	void OnAPIKeyChanged(const FText& Text);

	FString DisplayDeveloperKey;
	FText GetDisplayDeveloperKey() const;
	void OnDeveloperKeyChanged(const FText& Text);
	
	TSharedPtr<STextBlock> OrgNameTextBlock;
	TSharedPtr<STextBlock> OrgSubscriptionTextBlock;
	TSharedPtr<STextBlock> OrgExpiryTextBlock;
	TSharedPtr<STextBlock> OrgTrialTextBlock;

	FReply DebugNextPage();
	FReply DebugPreviousPage();
	
	FText GetHeaderTitle() const;
	EVisibility IsIntroVisible() const;
	EVisibility IsDevKeyVisible() const;
	EVisibility IsOrganizationDetailsVisible() const;
	EVisibility IsExportPathVisible() const;
	EVisibility IsCompleteVisible() const;
	EVisibility IsDynamicObjectsVisible() const;

	FReply NextPage();
	EVisibility NextButtonVisibility() const;
	bool NextButtonEnabled() const;
	FText NextButtonText() const;
	EVisibility BackButtonVisibility() const;
	FReply LastPage();

	const FSlateBrush* GetVideoImage() const;
	FSlateBrush* VideoImage;



	const FSlateBrush* GetExportPathStateIcon() const;

	FText GetExportPathTooltipText() const;

	void OnExportPathChanged(const FText& Text);


	void FetchApplicationKey(FString developerKey);
	void GetApplicationKeyResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	void FetchOrganizationDetails(FString developerKey);
	void GetOrganizationDetailsResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	FReply OpenDynamicObjectWindow();
	FReply OpenSceneSetupWindow();
};