#pragma once

#include "CognitiveVREditorPrivatePCH.h"
#include "CognitiveTools.h"
#include "CognitiveEditorData.h"
#include "CognitiveVRSettings.h"
#include "IDetailCustomization.h"
#include "PropertyEditing.h"
//#include "DetailCustomizationsPrivatePCH.h"
#include "PropertyCustomizationHelpers.h"
#include "Json.h"
#include "SCheckBox.h"
#include "STableRow.h"
#include "STextComboBox.h"
#include "SListView.h"
#include "Runtime/SlateCore/Public/Layout/Visibility.h"

class FCognitiveTools;

class SSceneSetupWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSceneSetupWidget){}
	SLATE_ARGUMENT(TArray<TSharedPtr<FDynamicData>>, Items)
	SLATE_ARGUMENT(FCognitiveTools*, CognitiveTools)
	SLATE_END_ARGS()

	void Construct(const FArguments& Args);

	/* Adds a new textbox with the string to the list */
	TSharedRef<ITableRow> OnGenerateRowForList(TSharedPtr<FDynamicData> Item, const TSharedRef<STableViewBase>& OwnerTable);

	/* The list of strings */
	TArray<TSharedPtr<FDynamicData>> Items;
	FCognitiveTools* CognitiveTools;

	int32 CurrentPage;

	void OnAPIKeyChanged(const FText& Text);
	FString APIKey;
	FText GetAPIKey() const;

	FText GetDeveloperKey() const;

	void OnDeveloperKeyChanged(const FText& Text);

	EVisibility IsIntroVisible() const;
	EVisibility IsKeysVisible() const;
	EVisibility IsDynamicsVisible() const;
	EVisibility IsExportVisible() const;
	EVisibility IsUploadVisible() const;
	EVisibility IsCompleteVisible() const;
	FReply NextPage();
	FReply LastPage();
	/* The actual UI list */
	TSharedPtr< SListView< TSharedPtr<FDynamicData> > > ListViewWidget;

	FReply SelectDynamic(TSharedPtr<FDynamicData> data);

	void RefreshList();
};