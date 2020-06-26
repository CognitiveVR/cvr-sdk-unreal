// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"
#include "Input/Reply.h"
#include "Layout/Visibility.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SWidget.h"
#include "Styling/SlateTypes.h"
#include "Framework/SlateDelegates.h"
#include "Widgets/Text/STextBlock.h"
#include "Framework/Views/ITypedTableView.h"
#include "Widgets/Views/STableViewBase.h"
#include "Framework/Views/TableViewTypeTraits.h"
#include "Widgets/Views/STableRow.h"
#include "Types/SlateConstants.h"
#include "Widgets/Layout/SScrollBar.h"
#include "Framework/Layout/Overscroll.h"
#include "Widgets/Views/SListView.h"

//this is a customized list view. it updates the selected actors in the 3d view based on selected items in the list and updates the list when actors are selected

/**
 * A ListView widget observes an array of data items and creates visual representations of these items.
 * ListView relies on the property that holding a reference to a value ensures its existence. In other words,
 * neither SActorListView<FString> nor SActorListView<FString*> are valid, while SActorListView< TSharedPtr<FString> > and
 * SActorListView< UObject* > are valid.
 *
 * A trivial use case appear below:
 *
 *   Given: TArray< TSharedPtr<FString> > Items;
 *
 *   SNew( SActorListView< TSharedPtr<FString> > )
 *     .ItemHeight(24)
 *     .ListItemsSource( &Items )
 *     .OnGenerateRow( SActorListView< TSharedPtr<FString> >::MakeOnGenerateWidget( this, &MyClass::OnGenerateRowForList ) )
 *
 * In the example we make all our widgets be 24 screen units tall. The ListView will create widgets based on data items
 * in the Items TArray. When the ListView needs to generate an item, it will do so using the OnGenerateWidgetForList method.
 *
 * A sample implementation of OnGenerateWidgetForList would simply return a TextBlock with the corresponding text:
 *
 * TSharedRef<ITableRow> OnGenerateWidgetForList( TSharedPtr<FString> InItem, const TSharedRef<STableViewBase>& OwnerTable )
 * {
 *     return SNew(STextBlock).Text( (*InItem) )
 * }
 *
 */
class SActorListView : public SListView<TSharedPtr<FDynamicData>>
{
public:
	bool modifierDown = false;

	virtual FReply OnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override
	{
		modifierDown = InKeyEvent.IsLeftShiftDown() || InKeyEvent.IsRightShiftDown();
		return STableViewBase::OnKeyUp(MyGeometry, InKeyEvent);
	}

	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override
	{
		modifierDown = InKeyEvent.IsLeftShiftDown() || InKeyEvent.IsRightShiftDown();
		return STableViewBase::OnKeyDown(MyGeometry, InKeyEvent);
	}

	virtual void Private_SignalSelectionChanged(ESelectInfo::Type SelectInfo) override
	{
		if (!modifierDown)
		{
			GEditor->SelectNone(false, true, false);
		}

		if (SelectionMode.Get() == ESelectionMode::None)
		{
			return;
		}

		if (OnSelectionChanged.IsBound())
		{
			NullableItemType SelectedItem = (SelectedItems.Num() > 0)
				? (*typename TSet<TSharedPtr<FDynamicData>>::TIterator(SelectedItems))
				: TListTypeTraits< TSharedPtr<FDynamicData> >::MakeNullPtr();

			OnSelectionChanged.ExecuteIfBound(SelectedItem, SelectInfo);
		}

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
			//if (dynamic->GetOwner()->GetName() != data->Name) { continue; }
			//if (dynamic->CustomId != data->Id) { continue; }
			//if (dynamic->MeshName != data->MeshName) { continue; }
			for (auto &elem : SelectedItems)
			{
				if (elem->Id == dynamic->CustomId)
				{
					GEditor->SelectActor((*ActorItr), true, true, true, true);
					break;
				}
			}
		}
	}
};