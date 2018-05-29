#pragma once

#include "CognitiveVREditorPrivatePCH.h"
#include "CognitiveVRSettings.h"
#include "IDetailCustomization.h"
#include "PropertyEditing.h"
//#include "DetailCustomizationsPrivatePCH.h"
#include "PropertyCustomizationHelpers.h"
#include "Json.h"
#include "SCheckBox.h"
#include "STableRow.h"
#include "SFStringListWidget.h"
#include "CognitiveEditorTools.h"
#include "STextComboBox.h"
#include "SDynamicObjectListWidget.h"
#include "SDynamicObjectWebListWidget.h"

//customization ui for cognitive settings. most of the functionality calls FCognitiveEditorTools

class UCognitiveVRSettings;

class FCognitiveSettingsCustomization : public IDetailCustomization
{
public:
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

	static TSharedRef<IDetailCustomization> MakeInstance();
	UCognitiveVRSettings *Settings;
	IDetailLayoutBuilder *DetailLayoutPtr;

	TArray<TSharedPtr<FDynamicData>> GetSceneDynamics();
	FReply RefreshDisplayDynamicObjectsCountInScene();

	//TSharedPtr<SVerticalBox> SetDynamicBoxContent();

	TSharedRef<ITableRow> OnGenerateWorkspaceRow(TSharedPtr<FEditorSceneData> InItem, const TSharedRef<STableViewBase>& OwnerTable);
	TSharedRef<ITableRow> OnGenerateDynamicRow(TSharedPtr<FDynamicData> InItem, const TSharedRef<STableViewBase>& OwnerTable);

	TSharedPtr<SDynamicObjectListWidget> SceneDynamicObjectList;
	TSharedPtr<SDynamicObjectWebListWidget> WebDynamicList;
	TSharedPtr<SFStringListWidget> SubDirectoryListWidget;


	FReply CopyDynamicSubDirectories();
	//TArray<TSharedPtr<FString>>DynamicSubDirectories;


	void OnChangedExcludeMesh(const FText& InNewValue);

	void OnChangedExportMinimumSize(float InNewValue);
	void OnCommitedExportMinimumSize(float InNewValue, ETextCommit::Type CommitType);

	void OnChangedExportMaximumSize(float InNewValue);
	void OnCommitedExportMaximumSize(float InNewValue, ETextCommit::Type CommitType);

	void OnChangedExportMinPolygon(int32 InNewValue);
	void OnCommitedExportMinPolygon(int32 InNewValue, ETextCommit::Type CommitType);

	void OnChangedExportMaxPolygon(int32 InNewValue);
	void OnCommitedExportMaxPolygon(int32 InNewValue, ETextCommit::Type CommitType);

	void OnChangedExportTextureRefactor(int32 InNewValue);
	void OnCommitedExportTextureRefactor(int32 InNewValue, ETextCommit::Type CommitType);

	void OnCheckStateChangedStaticOnly(const bool& InNewValue);
};