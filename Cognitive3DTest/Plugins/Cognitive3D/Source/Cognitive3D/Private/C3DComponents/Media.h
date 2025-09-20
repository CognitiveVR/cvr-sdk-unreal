// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/MeshComponent.h"
#include "DynamicObject.h"
#include "MediaPlayer.h"
#include "MediaSoundComponent.h"
#include "MediaTexture.h"
#include "Media.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class COGNITIVE3D_API UMedia : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UMedia();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

#if WITH_EDITOR
	virtual void OnRegister() override;
#endif

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, Category = "Media Source")
	FString SelectedMediaSourceId;

	UPROPERTY(VisibleAnywhere, Category = "Media Source")
	FString MediaName;

	UPROPERTY(VisibleAnywhere, Category = "Media Source")
	FString MediaId;

	UPROPERTY(VisibleAnywhere, Category = "Media Source")
	FString MediaDescription;

	TSharedPtr<FAnalyticsProviderCognitive3D> cognitive;

private:
	// Reference to the associated media player
	UPROPERTY()
	UMediaPlayer* AssociatedMediaPlayer;

	// Event handlers
	UFUNCTION()
	void OnMediaPlayerOpened(FString OpenedUrl);

	UFUNCTION()
	void OnMediaPlayerPlaybackResumed();

	UFUNCTION()
	void OnMediaPlayerPlaybackSuspended();

	UFUNCTION()
	void OnMediaPlayerClosed();

	// Helper function to find associated media player
	void FindAndBindToMediaPlayer();
	void UnbindFromMediaPlayer();


};
