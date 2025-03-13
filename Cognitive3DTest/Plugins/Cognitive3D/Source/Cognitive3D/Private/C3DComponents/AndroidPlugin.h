// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AndroidPlugin.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UAndroidPlugin : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UAndroidPlugin();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UFUNCTION()
	void OnSessionBegin();
	UFUNCTION()
	void OnSessionEnd();

	FString C3DSettingsPath;

	FString FolderPath;
	FString FolderPathCrashLog;
	FString CurrentFilePath;
	FString PreviousSessionFilePath;
	FString WriteDataFilePath;

	void OnLevelLoad(UWorld* world);

	void LogFileHasContent();
	FString ExtractTimestampFromLine(const FString& Line);
};
