#pragma once

#include "Components/SceneComponent.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "PropertyHandle.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "DetailCategoryBuilder.h"
#include "IDetailsView.h"
#include "DynamicObject.h"
#include "CognitiveEditorTools.h"
#include "BaseEditorTool.h"
#include "coreminimal.h"
#include "UObject/WeakObjectPtr.h"
#include "Input/Reply.h"
#include "IDetailCustomization.h"
#include "DynamicIdPoolAsset.h"
#include "CognitiveEditorTools.h"

//custom details inspector for dynamic object component

class UDynamicIdPoolAssetDetails : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();
private:
	/** IDetailCustomization interface */
	virtual void CustomizeDetails( IDetailLayoutBuilder& DetailLayout ) override;
	FReply Upload();
	FReply GenerateId();
	bool CanUploadIds() const;
	EVisibility HasMeshBeenExported() const;
	TWeakObjectPtr<UDynamicIdPoolAsset> SelectedTextAsset;
};
