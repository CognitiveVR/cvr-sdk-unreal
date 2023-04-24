#pragma once

#include "CognitiveVR/Private/util/util.h"
#include "CognitiveVR/Public/CognitiveVR.h"
#include "RoomSize.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class URoomSize : public UActorComponent
{
	GENERATED_BODY()

	public:
		URoomSize();

	private:
		virtual void BeginPlay() override;

		UFUNCTION()
		void OnSessionBegin();
		virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};