/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/

#include "DynamicObjectTableWidget.h"

void SDynamicObjectTableWidget::Construct(const FArguments& Args)
{
	ChildSlot
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			[
				SAssignNew(TableViewWidget, SActorListView)
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
					.FixedWidth(80)
					[
						SNew(SRichTextBlock)
						.DecoratorStyleSet(&FEditorStyle::Get())
						.Text(FText::FromString("<RichTextBlock.BoldHighlight>Exported</>"))
					]
					+ SHeaderRow::Column("uploaded")
					.FixedWidth(80)
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
}

TSharedRef<ITableRow> SDynamicObjectTableWidget::OnGenerateRowForTable(TSharedPtr<FDynamicData> InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	//find matching InItem by id
	bool hasUploadedId;
	FString searchId = InItem->Id;

	//check if the current scene is set up correctly with a SceneId and return the table row
	if (!FCognitiveEditorTools::GetInstance()->CurrentSceneHasSceneId())
	{
		//check if the export folder has files for this dynamic object
		bool hasExportedMesh = !InItem->MeshName.IsEmpty() && FCognitiveEditorTools::GetInstance()->DynamicMeshDirectoryExists(InItem->MeshName);

		return SNew(SDynamicTableItem, OwnerTable)
			.Name(FText::FromString(InItem->Name))
			.MeshName(FText::FromString(InItem->MeshName))
			.Id(FText::FromString(InItem->Id))
			.Exported(hasExportedMesh)
			.Uploaded(false);
	}

	if (InItem->DynamicType == EDynamicTypes::DynamicIdPoolAsset || InItem->DynamicType == EDynamicTypes::DynamicIdPool)
	{
		int32 idsFound = 0;
		for (auto ids : InItem->DynamicPoolIds)
		{
			auto FoundId2 = SDynamicObjectManagerWidget::dashboardObjects.FindByPredicate([ids](const FDashboardObject& InItem3)
				{
					return InItem3.sdkId == ids;
				}
			);
			if (FoundId2 == nullptr)
			{
				continue;
			}
			idsFound++;

			if (idsFound == InItem->DynamicPoolIds.Num())
			{
				hasUploadedId = FoundId2 != NULL;
			}
		}
	}
	else
	{
		auto FoundId = SDynamicObjectManagerWidget::dashboardObjects.FindByPredicate([searchId](const FDashboardObject& InItem2)
			{
				return InItem2.sdkId == searchId;
			});
		hasUploadedId = FoundId != NULL;
	}

	//check if the export folder has files for this dynamic object
	bool hasExportedMesh = !InItem->MeshName.IsEmpty() && FCognitiveEditorTools::GetInstance()->DynamicMeshDirectoryExists(InItem->MeshName);

	return SNew(SDynamicTableItem, OwnerTable)
		.Name(FText::FromString(InItem->Name))
		.MeshName(FText::FromString(InItem->MeshName))
		.Id(FText::FromString(InItem->Id))
		.Exported(hasExportedMesh)
		.Uploaded(hasUploadedId);
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

int32 SDynamicObjectTableWidget::GetSelectedDataCount()
{
	return TableViewWidget->GetNumItemsSelected();
}

void SDynamicTableItem::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView)
{
	Name = InArgs._Name;
	MeshName = InArgs._MeshName;
	Id = InArgs._Id;
	Exported = InArgs._Exported;
	Uploaded = InArgs._Uploaded;

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
			.WidthOverride(20)
			//.Padding(FMargin(3, 0))
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SImage)
				.ToolTipText(this, &SDynamicTableItem::GetExportedTooltip)
				.Image(this, &SDynamicTableItem::GetExportedStateIcon)
			];
	}
	if (ColumnName == TEXT("uploaded"))
	{
		return	SNew(SBox)
			.HeightOverride(20)
			.WidthOverride(20)
			//.Padding(FMargin(3, 0))
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SImage)
				.ToolTipText(this, &SDynamicTableItem::GetUploadedTooltip)
				.Image(this, &SDynamicTableItem::GetUploadedStateIcon)
			];
	}

	return SNullWidget::NullWidget;
}


const FSlateBrush* SDynamicTableItem::GetExportedStateIcon() const
{
	if (Exported)
	{
		return FCognitiveEditorTools::GetInstance()->BoxCheckIcon;
	}
	return FCognitiveEditorTools::GetInstance()->BoxEmptyIcon;
}

const FSlateBrush* SDynamicTableItem::GetUploadedStateIcon() const
{
	if (Uploaded)
	{
		return FCognitiveEditorTools::GetInstance()->BoxCheckIcon;
	}
	return FCognitiveEditorTools::GetInstance()->BoxEmptyIcon;
}

FText SDynamicTableItem::GetExportedTooltip() const
{
	if (Id.ToString().Contains("Id Pool Asset"))
	{
		if (Exported)
		{
			return FText::FromString("Associated Mesh has been exported to temrporary directory");
		}
		return FText::FromString("Associated Mesh has NOT been exported to temrporary directory");
	}
	if (Exported)
	{
		return FText::FromString("Mesh has been exported to temrporary directory");
	}
	return FText::FromString("Mesh has NOT been exported to temrporary directory");
}

FText SDynamicTableItem::GetUploadedTooltip() const
{
	if (Id.ToString().Contains("Id Pool Asset"))
	{
		if (Uploaded)
		{
			return FText::FromString("Id Pool Ids have been uploaded to the dashboard");
		}
		return FText::FromString("Id Pool Ids have NOT been uploaded to the dashboard");
	}
	else if (Id.ToString().Contains("Id Pool"))
	{
		if (Uploaded)
		{
			return FText::FromString("Mesh and Id Pool Ids have been uploaded to the dashboard");
		}
		return FText::FromString("Mesh and Id Pool Ids have NOT been uploaded to the dashboard");
	}
	else if (Id.ToString().Contains("generated"))
	{
		return FText::FromString("Id generated at runtime");
	}
	if (Uploaded)
	{
		return FText::FromString("Mesh and Id have been uploaded to the dashboard");
	}
	return FText::FromString("Mesh and Id have NOT been uploaded to the dashboard");
}

FText SDynamicTableItem::GetDefaultResponse() const
{
	return FText::FromString("NONE");
}