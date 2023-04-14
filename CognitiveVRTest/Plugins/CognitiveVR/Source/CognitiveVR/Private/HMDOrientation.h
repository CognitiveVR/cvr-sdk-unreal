#pragma once

#include "CognitiveVR/Private/util/util.h"
#include "CognitiveVR/Public/CognitiveVR.h"
#include "HMDOrientation.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UHMDOrientation : public UActorComponent
{
	GENERATED_BODY()

	public:
		UHMDOrientation();

	private:
		virtual void BeginPlay() override;
		virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
		TSharedPtr<FAnalyticsProviderCognitiveVR> provider;
		
		TArray<APlayerController*, FDefaultAllocator> controllers;

		float Interval = 1;
		float currentTime = 0;
		void RecordYaw();
		void RecordPitch();

};