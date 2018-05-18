#pragma once

#include "CognitiveVREditorPrivatePCH.h"
#include "CognitiveEditorTools.h"
#include "CognitiveEditorData.h"
#include "CognitiveVRSettings.h"
#include "IDetailCustomization.h"
#include "PropertyEditing.h"
//#include "DetailCustomizationsPrivatePCH.h"
#include "SDynamicObjectListWidget.h"
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
	//SLATE_ARGUMENT(FCognitiveEditorTools*, CognitiveEditorTools)
	SLATE_END_ARGS()

	void Construct(const FArguments& Args);


	FString DisplayAPIKey;
	FText GetDisplayAPIKey() const;
	void OnAPIKeyChanged(const FText& Text);

	FString DisplayDeveloperKey;
	FText GetDisplayDeveloperKey() const;
	void OnDeveloperKeyChanged(const FText& Text);



	TArray<TSharedPtr<FDynamicData>> GetSceneDynamics();

	/* Adds a new textbox with the string to the list */
	TSharedRef<ITableRow> OnGenerateRowForList(TSharedPtr<FDynamicData> Item, const TSharedRef<STableViewBase>& OwnerTable);

	/* The list of strings */
	TArray<TSharedPtr<FDynamicData>> Items;
	//FCognitiveTools* CognitiveTools;

	int32 CurrentPage;
	bool SceneWasExported = false;

	
	//FString APIKey;
	//FText GetAPIKey() const;

	//FText GetDeveloperKey() const;

	

	EVisibility IsIntroVisible() const;
	EVisibility IsKeysVisible() const;
	EVisibility IsDynamicsVisible() const;
	EVisibility IsExportVisible() const;
	EVisibility IsUploadVisible() const;
	EVisibility IsCompleteVisible() const;
	FReply NextPage();
	EVisibility NextButtonVisibility() const;
	bool NextButtonEnabled() const;
	FText NextButtonText() const;
	EVisibility BackButtonVisibility() const;
	FReply LastPage();

	FReply Export_Selected();
	FReply Export_All();

	EVisibility ARButtonVisibility() const;
	FReply ARSkipExport();

	TArray<TSharedPtr<FString>> NEWEXPORTFILES;

	TSharedRef<ITableRow> OnGenerateSceneExportFileRow(TSharedPtr<FString> InItem, const TSharedRef<STableViewBase>& OwnerTable);

	TSharedPtr<SDynamicObjectListWidget> SceneDynamicObjectList;

	FText DisplayDynamicObjectsCountInScene() const;
	FReply RefreshDisplayDynamicObjectsCountInScene();
	EVisibility GetDuplicateDyanmicObjectVisibility() const;
	EVisibility DuplicateDyanmicObjectVisibility;

	FReply SetUniqueDynamicIds();
	bool DuplicateDynamicIdsInScene() const;

	int32 CountDynamicObjectsInScene() const;

	FText DynamicCountInScene;

	/* The actual UI list */
	TSharedPtr< SListView< TSharedPtr<FDynamicData> > > ListViewWidget;

	FReply SelectDynamic(TSharedPtr<FDynamicData> data);

	void RefreshList();
};