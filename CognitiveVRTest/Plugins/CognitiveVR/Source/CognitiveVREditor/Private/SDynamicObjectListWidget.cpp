
#include "SDynamicObjectListWidget.h"

void SDynamicObjectListWidget::Construct(const FArguments& Args)
{
	ChildSlot
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			//.FillHeight(1)
			[
				SAssignNew(ListViewWidget, SActorListView)
				.ItemHeight(24)
				.ListItemsSource(&FCognitiveEditorTools::GetInstance()->SceneDynamics) //The Items array is the source of this listview
				.OnGenerateRow(this, &SDynamicObjectListWidget::OnGenerateRowForList)
				.HeaderRow(
					SNew(SHeaderRow)
					+ SHeaderRow::Column("name")
					.FillWidth(1)
					[
						SNew(SRichTextBlock)
						.DecoratorStyleSet(&FEditorStyle::Get())
						.Text(FText::FromString("<RichTextBlock.BoldHighlight>Name</>"))
					]

					+ SHeaderRow::Column("mesh")
					.FillWidth(1)
					[
						SNew(SRichTextBlock)
						.DecoratorStyleSet(&FEditorStyle::Get())
						.Text(FText::FromString("<RichTextBlock.BoldHighlight>Mesh Name</>"))
					]

					+ SHeaderRow::Column("id")
					.FillWidth(1)
					[
						SNew(SRichTextBlock)
						.DecoratorStyleSet(&FEditorStyle::Get())
						.Text(FText::FromString("<RichTextBlock.BoldHighlight>Id</>"))
					]
					+ SHeaderRow::Column("exported")
					.FillWidth(1)
					[
						SNew(SRichTextBlock)
						.DecoratorStyleSet(&FEditorStyle::Get())
						.Text(FText::FromString("<RichTextBlock.BoldHighlight>Exported</>"))
					]
				)
			]
		];
}

void SDynamicObjectListWidget::RefreshList()
{
	ListViewWidget->RequestListRefresh();
}

FReply SDynamicObjectListWidget::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	return ListViewWidget->OnMouseButtonDown(MyGeometry, MouseEvent);
}

TSharedRef<ITableRow> SDynamicObjectListWidget::OnGenerateRowForList(TSharedPtr<FDynamicData> InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	return
		SNew(SComboRow< TSharedPtr<FDynamicData> >, OwnerTable)
		[
			SNew(SHorizontalBox)
			/*+ SHorizontalBox::Slot()
			.MaxWidth(64)
			.AutoWidth()
			.Padding(2.0f)
			[
				SNew(SBox)
				.WidthOverride(64)
				.HeightOverride(20)
				[
					SNew(SButton)
					.OnClicked(FOnClicked::CreateSP(this, &SDynamicObjectListWidget::SelectDynamic, InItem))
					.Text(FText::FromString("Select Actor"))
				]
			]*/
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
			.FillWidth(1)
			.Padding(2.0f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(InItem->Id))
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1)
			.Padding(2.0f)
			[
				SNew(STextBlock)
				.Text(ExportStatusText(InItem))
			]
		];
}

//bool SDynamicObjectListWidget::IsShiftPressed() const
//{
//	//never returns true. slate doesn't have a viewport. how can it recieve inputs??
//	for (auto& elem : GEditor->AllViewportClients)
//	{
//		if (elem->IsShiftPressed()) { return true; }
//		//if (elem->KeyState(EKeys::LeftShift)) { return true; }
//		//if (elem->KeyState(EKeys::RightShift)) { return true; }
//	}
//	return GEditor->GetActiveViewport()->KeyState(EKeys::LeftShift) || GEditor->GetActiveViewport()->KeyState(EKeys::RightShift);
//}

FReply SDynamicObjectListWidget::SelectDynamic(TSharedPtr<FDynamicData> data)
{
	for (TActorIterator<AActor> ActorItr(GWorld); ActorItr; ++ActorItr)
	{
		// Same as with the Object Iterator, access the subclass instance with the * or -> operators.
		//AStaticMeshActor *Mesh = *ActorItr;

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
		ListViewWidget->SetSelection(data);

		break;
	}

	return FReply::Handled();
}

FText SDynamicObjectListWidget::ExportStatusText(TSharedPtr<FDynamicData> data)
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