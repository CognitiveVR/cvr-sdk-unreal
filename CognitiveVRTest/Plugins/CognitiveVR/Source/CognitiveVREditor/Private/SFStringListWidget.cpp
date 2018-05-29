#include "CognitiveVREditorPrivatePCH.h"
#include "SFStringListWidget.h"

void SFStringListWidget::Construct(const FArguments& Args)
{
	ChildSlot
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(EHorizontalAlignment::HAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString("Sub Directories"))
			]
			+SVerticalBox::Slot()
			[
				SAssignNew(ListViewWidget, SListView<TSharedPtr<FString>>)
				.ItemHeight(24)
				.ListItemsSource(&Items) //The Items array is the source of this listview
				.OnGenerateRow(this, &SFStringListWidget::OnGenerateRowForList)
			]
		];
}

void SFStringListWidget::RefreshList()
{
	ListViewWidget->RequestListRefresh();
}

TSharedRef<ITableRow> SFStringListWidget::OnGenerateRowForList(TSharedPtr<FString> InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	return
		SNew(SComboRow< TSharedPtr<FString> >, OwnerTable)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1)
			.Padding(2.0f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(*InItem))
			]
		];
}