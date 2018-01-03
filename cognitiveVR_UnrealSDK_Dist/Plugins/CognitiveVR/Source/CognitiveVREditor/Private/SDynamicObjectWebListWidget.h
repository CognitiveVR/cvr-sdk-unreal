#pragma once

#include "CognitiveVREditorPrivatePCH.h"
#include "CognitiveTools.h"
#include "CognitiveEditorData.h"
#include "CognitiveVRSettings.h"
#include "IDetailCustomization.h"
#include "PropertyEditing.h"
//#include "DetailCustomizationsPrivatePCH.h"
#include "PropertyCustomizationHelpers.h"
#include "Json.h"
#include "SCheckBox.h"
#include "STableRow.h"
#include "STextComboBox.h"
#include "SListView.h"

class FCognitiveTools;

class SDynamicObjectWebListWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SDynamicObjectWebListWidget){}
	SLATE_ARGUMENT(TArray<TSharedPtr<FDynamicData>>, Items)
	SLATE_ARGUMENT(FCognitiveTools*, CognitiveTools)
	SLATE_END_ARGS()

	void Construct(const FArguments& Args);
	void RefreshList();

	/* Adds a new textbox with the string to the list */
	TSharedRef<ITableRow> OnGenerateRowForList(TSharedPtr<FDynamicData> Item, const TSharedRef<STableViewBase>& OwnerTable);

	/* The list of strings */
	TArray<TSharedPtr<FDynamicData>> Items;
	FCognitiveTools* CognitiveTools;

	/* The actual UI list */
	TSharedPtr< SListView< TSharedPtr<FDynamicData> > > ListViewWidget;

	//virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
};