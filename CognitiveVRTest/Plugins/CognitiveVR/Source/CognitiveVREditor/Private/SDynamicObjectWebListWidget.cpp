#include "CognitiveVREditorPrivatePCH.h"
#include "SDynamicObjectWebListWidget.h"

void SDynamicObjectWebListWidget::Construct(const FArguments& Args)
{
	ChildSlot
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.FillHeight(1)
			[
				SAssignNew(ListViewWidget, SListView<TSharedPtr<cognitivevrapi::FDynamicData>>)
				.ItemHeight(24)
				.ListItemsSource(&Items) //The Items array is the source of this listview
				.OnGenerateRow(this, &SDynamicObjectWebListWidget::OnGenerateRowForList)
				.HeaderRow(
					SNew(SHeaderRow)
					+ SHeaderRow::Column("name")
					.FillWidth(1)
					[
						SNew(STextBlock)
						.Text(FText::FromString("Name"))
					]

					+ SHeaderRow::Column("mesh")
					.FillWidth(1)
					[
						SNew(STextBlock)
						.Text(FText::FromString("MeshName"))
					]

					+ SHeaderRow::Column("id")
					.FillWidth(0.3)
					[
						SNew(STextBlock)
						.Text(FText::FromString("Id"))
					]
				)
			]
		];
}

void SDynamicObjectWebListWidget::RefreshList()
{	
	//Items = SceneExplorerDynamics;
	ListViewWidget->RequestListRefresh();
}


TSharedRef<ITableRow> SDynamicObjectWebListWidget::OnGenerateRowForList(TSharedPtr<cognitivevrapi::FDynamicData> InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	return
		SNew(SComboRow< TSharedPtr<cognitivevrapi::FDynamicData> >, OwnerTable)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
		.FillWidth(1)
		.Padding(2.0f)
		[
			SNew(STextBlock)
			.Text(FText::FromString(InItem->Name))
		]
	+ SHorizontalBox::Slot()
		.FillWidth(1)
		.Padding(2.0f)
		[
			SNew(STextBlock)
			.Text(FText::FromString(InItem->MeshName))
		]
	+ SHorizontalBox::Slot()
		.FillWidth(0.3)
		.Padding(2.0f)
		[
			SNew(STextBlock)
			.Text(FText::FromString(InItem->Id))
		]
		];
}