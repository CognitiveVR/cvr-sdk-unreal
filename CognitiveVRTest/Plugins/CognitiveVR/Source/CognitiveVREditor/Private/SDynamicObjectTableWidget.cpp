
#include "SDynamicObjectTableWidget.h"

void SDynamicObjectTableWidget::Construct(const FArguments& Args)
{
	ChildSlot
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			[
				SAssignNew(TableViewWidget, SListView<TSharedPtr<FDynamicData>>)
				.ItemHeight(24)
				.ListItemsSource(&FCognitiveEditorTools::GetInstance()->SceneDynamics) //The Items array is the source of this listview
				.OnGenerateRow(this, &SDynamicObjectTableWidget::OnGenerateRowForTable)
				.SelectionMode(ESelectionMode::Multi)
				.OnSelectionChanged(this, &SDynamicObjectTableWidget::OnSelectionChanged)

				.HeaderRow(
					SNew(SHeaderRow)
					+ SHeaderRow::Column("name")
					[
						SNew(SRichTextBlock)
						.DecoratorStyleSet(&FEditorStyle::Get())
						.Text(FText::FromString("<RichTextBlock.BoldHighlight>Name</>"))
					]

					+ SHeaderRow::Column("meshname")
					[
						SNew(SRichTextBlock)
						.DecoratorStyleSet(&FEditorStyle::Get())
						.Text(FText::FromString("<RichTextBlock.BoldHighlight>Mesh Name</>"))
					]

					+ SHeaderRow::Column("id")
					[
						SNew(SRichTextBlock)
						.DecoratorStyleSet(&FEditorStyle::Get())
						.Text(FText::FromString("<RichTextBlock.BoldHighlight>Id</>"))
					]
					+ SHeaderRow::Column("exported")
					[
						SNew(SRichTextBlock)
						.DecoratorStyleSet(&FEditorStyle::Get())
						.Text(FText::FromString("<RichTextBlock.BoldHighlight>Exported</>"))
					]
					+ SHeaderRow::Column("uploaded")
					[
						SNew(SRichTextBlock)
						.DecoratorStyleSet(&FEditorStyle::Get())
						.Text(FText::FromString("<RichTextBlock.BoldHighlight>Uploaded</>"))
					]
				)
			]
		];
}

void SDynamicObjectTableWidget::RefreshTable()
{
	TableViewWidget->RequestListRefresh();
}

void SDynamicObjectTableWidget::OnSelectionChanged(TSharedPtr<FDynamicData> InNode, ESelectInfo::Type SelectInfo)
{
	if (SelectInfo == ESelectInfo::Direct)
	{
		return;
	}
	SelectDynamic(InNode);
	//OnComponentSelected.ExecuteIfBound(InNode);
}

TSharedRef<ITableRow> SDynamicObjectTableWidget::OnGenerateRowForTable(TSharedPtr<FDynamicData> InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(SDynamicTableItem, OwnerTable)
		.Name(FText::FromString(InItem->Name))
		.MeshName(FText::FromString(InItem->MeshName))
		.Id(FText::FromString(InItem->Id))
		.Exported(true)
		.Uploaded(true);
}

FReply SDynamicObjectTableWidget::SelectDynamic(TSharedPtr<FDynamicData> data)
{
	for (TActorIterator<AActor> ActorItr(GWorld); ActorItr; ++ActorItr)
	{
		UActorComponent* actorComponent = (*ActorItr)->GetComponentByClass(UDynamicObject::StaticClass());
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

		GEditor->SelectActor((*ActorItr), true, true, true, true);
		TableViewWidget->SetSelection(data);

		break;
	}

	return FReply::Handled();
}

FText SDynamicObjectTableWidget::ExportStatusText(TSharedPtr<FDynamicData> data)
{
	//check directory for obj/gltf files

	auto tools = FCognitiveEditorTools::GetInstance();	
	//
	FString path = tools->GetDynamicsExportDirectory() + "/" + data->MeshName + "/" + data->MeshName;
	FString objpath = path + ".obj";
	FString gltfpath = path + ".gltf";
	
	if (FPaths::FileExists(*objpath))
	{
		return FText::FromString("obj");
	}
	if (FPaths::FileExists(*gltfpath))
	{
		return FText::FromString("gltf");
	}

	//get gltf or obj

	return FText::FromString("none");
}

void SDynamicTableItem::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView)
{
	Name = InArgs._Name;
	MeshName = InArgs._MeshName;
	Id = InArgs._Id;
	Exported = true;
	Uploaded = true;

	SMultiColumnTableRow<TSharedPtr<FDynamicData> >::Construct(FSuperRowType::FArguments(), InOwnerTableView);
}

TSharedRef<SWidget> SDynamicTableItem::GenerateWidgetForColumn(const FName& ColumnName)
{
	if (ColumnName == TEXT("name"))
	{
		return	SNew(SBox)
			.HeightOverride(20)
			.Padding(FMargin(3, 0))
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(Name)
				.Font(IDetailLayoutBuilder::GetDetailFont())
			];
	}
	if (ColumnName == TEXT("meshname"))
	{
		return	SNew(SBox)
			.HeightOverride(20)
			.Padding(FMargin(3, 0))
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(MeshName)
				.Font(IDetailLayoutBuilder::GetDetailFont())
			];
	}
	if (ColumnName == TEXT("id"))
	{
		return	SNew(SBox)
			.HeightOverride(20)
			.Padding(FMargin(3, 0))
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(Id)
				.Font(IDetailLayoutBuilder::GetDetailFont())
			];
	}
	if (ColumnName == TEXT("exported"))
	{
		return	SNew(SBox)
			.HeightOverride(20)
			.Padding(FMargin(3, 0))
			.VAlign(VAlign_Center)
			[
				SNew(SCheckBox)
				.IsChecked(ECheckBoxState::Checked)
				.IsEnabled(false)
			];
	}
	if (ColumnName == TEXT("uploaded"))
	{
		return	SNew(SBox)
			.HeightOverride(20)
			.Padding(FMargin(3, 0))
			.VAlign(VAlign_Center)
			[
				SNew(SCheckBox)
				.IsChecked(ECheckBoxState::Checked)
				.IsEnabled(false)
			];
	}

	return SNullWidget::NullWidget;
}

FText SDynamicTableItem::GetDefaultResponse() const
{
	return FText::FromString("NONE");
}