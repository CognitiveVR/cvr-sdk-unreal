#include "CognitiveVREditorPrivatePCH.h"
#include "SDynamicObjectWebListWidget.h"

void SDynamicObjectWebListWidget::Construct(const FArguments& Args)
{
	//DelimiterImage = InArgs._DelimiterImage;
	//Items = Args._DynamicData;

	ChildSlot
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.FillHeight(1)
			[
				SAssignNew(ListViewWidget, SListView<TSharedPtr<FDynamicData>>)
				.ItemHeight(24)
				.ListItemsSource(&Items) //The Items array is the source of this listview
				.OnGenerateRow(this, &SDynamicObjectWebListWidget::OnGenerateRowForList)
				.HeaderRow(
					SNew(SHeaderRow)
					+ SHeaderRow::Column("name")
					.FillWidth(1)
					[
						SNew(STextBlock)
						//.MinDesiredWidth(256)
						.Text(FText::FromString("Name"))
					]

					+ SHeaderRow::Column("mesh")
					.FillWidth(1)
					[
						SNew(STextBlock)
						//.MinDesiredWidth(256)
						.Text(FText::FromString("MeshName"))
					]

					+ SHeaderRow::Column("id")
					.FillWidth(0.3)
					[
						SNew(STextBlock)
						//.MinDesiredWidth(512)
						.Text(FText::FromString("Id"))
					]
				)
			]
		];
}

FReply SDynamicObjectWebListWidget::ButtonPressed()
{
	//Adds a new item to the array (do whatever you want with this)
	//Items.Add(MakeShareable(new FString("Hello 1")));
	//Items = CognitiveTools->GetSceneDynamics();
	
	//Items = FCognitiveTools::GetSceneDynamics();
	
	Items = SceneExplorerDynamics;
	ListViewWidget->RequestListRefresh();

	//Update the listview
	//if (ListViewWidget.IsValid())
		//ListViewWidget->RequestListRefresh();

	return FReply::Handled();
}


TSharedRef<ITableRow> SDynamicObjectWebListWidget::OnGenerateRowForList(TSharedPtr<FDynamicData> InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	return
		SNew(SComboRow< TSharedPtr<FDynamicData> >, OwnerTable)
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
			.Text(FText::FromString(FString::FromInt(InItem->Id)))
		]
		];
}