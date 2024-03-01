#pragma once

#include "Cognitive3D/Private/util/util.h"
#include "Cognitive3D/Public/Cognitive3D.h"
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