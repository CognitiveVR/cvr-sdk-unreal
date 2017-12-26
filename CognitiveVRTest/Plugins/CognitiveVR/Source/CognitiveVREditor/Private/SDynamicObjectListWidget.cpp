#include "CognitiveVREditorPrivatePCH.h"
#include "SDynamicObjectListWidget.h"

void SDynamicObjectListWidget::Construct(const FArguments& Args)
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
				.ListItemsSource(&SceneDynamics) //The Items array is the source of this listview
				.OnGenerateRow(this, &SDynamicObjectListWidget::OnGenerateRowForList)
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
			/*+SVerticalBox::Slot()
			//.FillHeight(1)
			//.MaxHeight(24)
			.AutoHeight()
			[
				SNew(SButton)
				.Text(FText::FromString("Refresh"))
				.OnClicked(this, &SDynamicObjectListWidget::ButtonPressed)
			]*/
		];
}

void SDynamicObjectListWidget::RefreshList()
{
	ListViewWidget->RequestListRefresh();
}

FReply SDynamicObjectListWidget::ButtonPressed()
{
	//Adds a new item to the array (do whatever you want with this)
	//Items.Add(MakeShareable(new FString("Hello 1")));
	//Items = CognitiveTools->GetSceneDynamics();
	
	//Items = FCognitiveTools::GetSceneDynamics();
	
	//Items = SceneDynamics;
	ListViewWidget->RequestListRefresh();

	//Update the listview
	//if (ListViewWidget.IsValid())
		//ListViewWidget->RequestListRefresh();

	return FReply::Handled();
}


TSharedRef<ITableRow> SDynamicObjectListWidget::OnGenerateRowForList(TSharedPtr<FDynamicData> InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	//FOnClicked::CreateSP(this, &SDynamicObjectListWidget::SelectDynamic, InItem);

	return
		SNew(SComboRow< TSharedPtr<FDynamicData> >, OwnerTable)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.MaxWidth(16)
			.AutoWidth()
		//.HAlign(EHorizontalAlignment::HAlign_Center)
		//.VAlign(EVerticalAlignment::VAlign_Center)
			.Padding(2.0f)
			[
				SNew(SBox)
				.HeightOverride(16)
				.HeightOverride(16)
				[
					//SNew(STextBlock)
					//.Text(FText::FromString(InItem->Name))
					SNew(SButton)
					//.Text(FText::FromString(InItem->Name))
					.OnClicked(FOnClicked::CreateSP(this, &SDynamicObjectListWidget::SelectDynamic, InItem))
				]
			]
			+ SHorizontalBox::Slot()
		.FillWidth(1)
		.Padding(2.0f)
		[
			SNew(STextBlock)
			.Text(FText::FromString(InItem->Name))
			//SNew(SButton)
			//.Text(FText::FromString(InItem->Name))
			//.OnClicked(FOnClicked::CreateSP(this, &SDynamicObjectListWidget::SelectDynamic, InItem))
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

FReply SDynamicObjectListWidget::SelectDynamic(TSharedPtr<FDynamicData> data)
{
	GLog->Log("select dynamic " + data->Name);

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