/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/

#pragma once

#include "Cognitive3D/Private/C3DUtil/Util.h"
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