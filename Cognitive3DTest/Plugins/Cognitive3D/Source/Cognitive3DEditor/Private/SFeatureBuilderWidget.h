// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "DynamicObjectManagerWidget.h"

/**
 * 
 */
class SFeatureBuilderWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SFeatureBuilderWidget)
	{}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

private:
    enum class EPageMode : uint8 { List = 0, Detail = 1 };
    EPageMode CurrentMode = EPageMode::List;
    FName SelectedFeature;

    // Switcher index getter
    int32 GetPageIndex() const { return static_cast<int32>(CurrentMode); }

    struct FFeature { FName Key; FText DisplayName; const FSlateBrush* Icon; FText Desc; };
    TArray<FFeature> FeatureList;

    // Detail holder—updates dynamically
    TSharedPtr<SBox> DetailBox;

    // Builds each page
    TSharedRef<SWidget> BuildFeatureList();
    TSharedRef<SWidget> BuildFeatureDetail();
    TSharedRef<SWidget> CreateDetailWidget();

    // Callbacks
    FReply OnFeatureClicked(FName FeatureKey);
    //exitpoll
    FReply OnLaunchExitPoll();
    FReply OnAddExitPollBlueprint();
    //remote controls
    FReply OnLaunchRemoteControls();
    FReply OnAddRemoteControlsComponent();
    //other
    FReply OnBackClicked();
};
