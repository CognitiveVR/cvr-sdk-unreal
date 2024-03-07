#pragma once

#include "CognitiveEditorTools.h"
#include "CognitiveEditorData.h"
#include "Cognitive3DSettings.h"
#include "IDetailCustomization.h"
#include "PropertyEditing.h"
#include "PropertyCustomizationHelpers.h"
#include "Json.h"
#include "SCheckBox.h"
#include "STableRow.h"
#include "STextComboBox.h"
#include "STreeView.h"
#include "SListView.h"
#include "ActorListView.h"
#include "DynamicObjectManagerWidget.h"

class SDynamicTableItem;

class SDynamicObjectTableWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SDynamicObjectTableWidget){}
	SLATE_END_ARGS()

	void Construct(const FArguments& Args);
	/* Adds a new textbox with the string to the list */
	TSharedRef<ITableRow> OnGenerateRowForTable(TSharedPtr<FDynamicData> Item, const TSharedRef<STableViewBase>& OwnerTable);
	TSharedPtr< SListView<TSharedPtr<FDynamicData>> > TableViewWidget;

	void OnSelectionChanged(TSharedPtr<FDynamicData> InNode, ESelectInfo::Type SelectInfo);
	FReply SelectDynamic(TSharedPtr<FDynamicData> data);

	void RefreshTable();

	FText ExportStatusText(TSharedPtr<FDynamicData> data);	
	int32 GetSelectedDataCount();
};

class SDynamicTableItem : public SMultiColumnTableRow< TSharedPtr<class FDynamicData> >
{
public:

	SLATE_BEGIN_ARGS(SDynamicTableItem) { }
		SLATE_ARGUMENT(FText, Name)
		SLATE_ARGUMENT(FText, MeshName)
		SLATE_ARGUMENT(FText, Id)
		SLATE_ARGUMENT(bool, Exported)
		SLATE_ARGUMENT(bool, Uploaded)
	SLATE_END_ARGS()

public:

	/**
	* Constructs the application.
	*
	* @param InArgs - The Slate argument list.
	*/
	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView);
	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override;
private:
	const FSlateBrush* GetExportedStateIcon() const;
	const FSlateBrush* GetUploadedStateIcon() const;
	FText GetExportedTooltip() const;
	FText GetUploadedTooltip() const;

	FText Name;
	FText MeshName;
	FText Id;
	bool Exported;
	bool Uploaded;

	FText GetDefaultResponse() const;
};