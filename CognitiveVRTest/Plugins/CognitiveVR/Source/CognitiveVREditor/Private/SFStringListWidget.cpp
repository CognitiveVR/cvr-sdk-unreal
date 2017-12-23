#include "CognitiveVREditorPrivatePCH.h"
#include "SFStringListWidget.h"

void SFStringListWidget::Construct(const FArguments& Args)
{
	//DelimiterImage = InArgs._DelimiterImage;
	//Items = Args._DynamicData;

	ChildSlot
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			[
				SNew(SBox)
				.HeightOverride(24)
				[
					SNew(SButton)
					.Text(FText::FromString("Refresh"))
					.OnClicked(this, &SFStringListWidget::ButtonPressed)
				]
			]
			+SVerticalBox::Slot()
			//.FillHeight(2)
			[
				SAssignNew(ListViewWidget, SListView<TSharedPtr<FString>>)
				.ItemHeight(24)
				.ListItemsSource(&Items) //The Items array is the source of this listview
				.OnGenerateRow(this, &SFStringListWidget::OnGenerateRowForList)
				/*.HeaderRow(
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
				)*/
			]
		];
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

FReply SFStringListWidget::ButtonPressed()
{
	//Adds a new item to the array (do whatever you want with this)

	Items = SubDirectoryNames;
	Items.Add(MakeShareable(new FString("Hello 1")));

	//Update the listview
	ListViewWidget->RequestListRefresh();

	return FReply::Handled();
}