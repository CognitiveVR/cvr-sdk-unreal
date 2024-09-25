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
					.FixedWidth(90)
					[
						SNew(SRichTextBlock)
						.DecoratorStyleSet(&FEditorStyle::Get())
						.Text(FText::FromString("<RichTextBlock.BoldHighlight>Mesh Exported</>"))
					]
					+ SHeaderRow::Column("uploadedmesh")
					.FixedWidth(90)
					[
						SNew(SRichTextBlock)
						.DecoratorStyleSet(&FEditorStyle::Get())
						.Text(FText::FromString("<RichTextBlock.BoldHighlight>Mesh Uploaded</>"))
					]
					+ SHeaderRow::Column("uploadedid")
					.FixedWidth(90)
					[
						SNew(SRichTextBlock)
						.DecoratorStyleSet(&FEditorStyle::Get())
						.Text(FText::FromString("<RichTextBlock.BoldHighlight>Id Uploaded</>"))
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
	FString searchMesh = InItem->MeshName;
	
	TSharedPtr<FString> sceneName = SDynamicObjectManagerWidget::SceneDisplayName;

	//check if the current scene is set up correctly with a SceneId and return the table row
	if (!FCognitiveEditorTools::GetInstance()->SceneHasSceneId(*sceneName))
	{
		//check if the export folder has files for this dynamic object
		bool hasExportedMesh = !InItem->MeshName.IsEmpty() && FCognitiveEditorTools::GetInstance()->DynamicMeshDirectoryExists(InItem->MeshName);

		return SNew(SDynamicTableItem, OwnerTable)
			.Name(FText::FromString(InItem->Name))
			.MeshName(FText::FromString(InItem->MeshName))
			.Id(FText::FromString(InItem->Id))
			.Exported(hasExportedMesh)
			.UploadedMesh(false)
			.UploadedId(false);
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

	//
	bool hasUploadedMesh = false;
	auto FoundId = SDynamicObjectManagerWidget::dashboardObjects.FindByPredicate([searchMesh](const FDashboardObject& InItem2)
	{
		return InItem2.meshName == searchMesh;
	});
	hasUploadedMesh = FoundId != NULL;


	//check if the export folder has files for this dynamic object
	bool hasExportedMesh = !InItem->MeshName.IsEmpty() && FCognitiveEditorTools::GetInstance()->DynamicMeshDirectoryExists(InItem->MeshName);

	return SNew(SDynamicTableItem, OwnerTable)
		.Name(FText::FromString(InItem->Name))
		.MeshName(FText::FromString(InItem->MeshName))
		.Id(FText::FromString(InItem->Id))
		.Exported(hasExportedMesh)
		.UploadedMesh(hasUploadedMesh)
		.UploadedId(hasUploadedId);
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
	UploadedId = InArgs._UploadedId;
	UploadedMesh = InArgs._UploadedMesh;

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
			.WidthOverride(30)
			//.Padding(FMargin(3, 0))
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SImage)
				.ToolTipText(this, &SDynamicTableItem::GetExportedTooltip)
				.Image(this, &SDynamicTableItem::GetExportedStateIcon)
			];
	}
	if (ColumnName == TEXT("uploadedmesh"))
	{
		return	SNew(SBox)
			.HeightOverride(20)
			.WidthOverride(30)
			//.Padding(FMargin(3, 0))
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SImage)
				.ToolTipText(this, &SDynamicTableItem::GetUploadedMeshTooltip)
				.Image(this, &SDynamicTableItem::GetUploadedMeshStateIcon)
			];
	}
	if (ColumnName == TEXT("uploadedid"))
	{
		return	SNew(SBox)
			.HeightOverride(20)
			.WidthOverride(30)
			//.Padding(FMargin(3, 0))
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SImage)
				.ToolTipText(this, &SDynamicTableItem::GetUploadedIdTooltip)
				.Image(this, &SDynamicTableItem::GetUploadedIdStateIcon)
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

const FSlateBrush* SDynamicTableItem::GetUploadedIdStateIcon() const
{
	if (UploadedId)
	{
		return FCognitiveEditorTools::GetInstance()->BoxCheckIcon;
	}
	return FCognitiveEditorTools::GetInstance()->BoxEmptyIcon;
}

const FSlateBrush* SDynamicTableItem::GetUploadedMeshStateIcon() const
{
	if (UploadedMesh)
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
			return FText::FromString("Associated Mesh has been exported to temporary directory");
		}
		return FText::FromString("Associated Mesh has NOT been exported to temporary directory");
	}
	if (Exported)
	{
		return FText::FromString("Mesh has been exported to temporary directory");
	}
	return FText::FromString("Mesh has NOT been exported to temporary directory");
}

FText SDynamicTableItem::GetUploadedIdTooltip() const
{
	if (Id.ToString().Contains("Id Pool Asset"))
	{
		if (UploadedId)
		{
			return FText::FromString("Id Pool Ids have been uploaded to the dashboard");
		}
		return FText::FromString("Id Pool Ids have NOT been uploaded to the dashboard");
	}
	else if (Id.ToString().Contains("Id Pool"))
	{
		if (UploadedId)
		{
			return FText::FromString("Id Pool Ids have been uploaded to the dashboard");
		}
		return FText::FromString("Id Pool Ids have NOT been uploaded to the dashboard");
	}
	else if (Id.ToString().Contains("generated"))
	{
		return FText::FromString("Id generated at runtime");
	}
	if (UploadedId)
	{
		return FText::FromString("Id have been uploaded to the dashboard");
	}
	return FText::FromString("Id have NOT been uploaded to the dashboard");
}

FText SDynamicTableItem::GetUploadedMeshTooltip() const
{
	if (Id.ToString().Contains("Id Pool Asset") || Id.ToString().Contains("Id Pool"))
	{
		if (UploadedMesh)
		{
			return FText::FromString("Id Pool Mesh has been uploaded to the dashboard");
		}
		return FText::FromString("Id Pool Mesh has NOT been uploaded to the dashboard");
	}
	if (UploadedMesh)
	{
		return FText::FromString("Mesh has been uploaded to the dashboard");
	}
	return FText::FromString("Mesh has NOT been uploaded to the dashboard");
}

FText SDynamicTableItem::GetDefaultResponse() const
{
	return FText::FromString("NONE");
}