#include "CognitiveVREditorPrivatePCH.h"
#include "SDynamicObjectListWidget.h"

void SDynamicObjectListWidget::Construct(const FArguments& Args)
{
	ChildSlot
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.FillHeight(1)
			[
				SAssignNew(ListViewWidget, SListView<TSharedPtr<FDynamicData>>)
				.ItemHeight(24)
				.ListItemsSource(&SceneDynamics) //The Items array is the source of this listview
				.OnGenerateRow(this, &SDynamicObjectListWidget::OnGenerateRowForList)
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

void SDynamicObjectListWidget::RefreshList()
{
	ListViewWidget->RequestListRefresh();
}

TSharedRef<ITableRow> SDynamicObjectListWidget::OnGenerateRowForList(TSharedPtr<FDynamicData> InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	return
		SNew(SComboRow< TSharedPtr<FDynamicData> >, OwnerTable)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.MaxWidth(16)
			.AutoWidth()
			.Padding(2.0f)
			[
				SNew(SBox)
				.HeightOverride(16)
				.HeightOverride(16)
				[
					SNew(SButton)
					.OnClicked(FOnClicked::CreateSP(this, &SDynamicObjectListWidget::SelectDynamic, InItem))
				]
			]
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

FReply SDynamicObjectListWidget::SelectDynamic(TSharedPtr<FDynamicData> data)
{
	GEditor->SelectNone(false, true, false);

	for (TActorIterator<AStaticMeshActor> ActorItr(GWorld); ActorItr; ++ActorItr)
	{
		// Same as with the Object Iterator, access the subclass instance with the * or -> operators.
		AStaticMeshActor *Mesh = *ActorItr;

		UActorComponent* actorComponent = Mesh->GetComponentByClass(UDynamicObject::StaticClass());
		if (actorComponent == NULL)
		{
			continue;
		}
		UDynamicObject* dynamic = Cast<UDynamicObject>(actorComponent);
		if (dynamic == NULL)
		{
			continue;
		}
		if (dynamic->GetOwner()->GetName() != data->Name) { continue; }
		if (dynamic->CustomId != data->Id) { continue; }
		if (dynamic->MeshName != data->MeshName) { continue; }

		GEditor->SelectActor(Mesh, true, true, true, true);

		break;
	}

	return FReply::Handled();
}