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
    
    // Get display name for selected feature
    FText GetSelectedFeatureDisplayName() const;

    struct FFeature { FName Key; FText DisplayName; const FSlateBrush* Icon; FText Desc; };
    TArray<FFeature> FeatureList;

    bool bIsDeveloperKeyValid = false;

    // Detail holder updates dynamically
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
    FReply OnToggleRemoteControlsComponent();
    bool HasRemoteControlsComponent() const;
    //custom events
	FReply OnLaunchCustomEvents();
    //media
    FReply OnOpenMediaDashboard();
    FReply OnAddMediaComponent();
    //developer key validation
    void CheckForExpiredDeveloperKey(FString developerKey);
    void OnDeveloperKeyResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
    //other
    FReply OnBackClicked();
};
