#include "CognitiveVR/Private/RoomSize.h"

URoomSize::URoomSize()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void URoomSize::BeginPlay()
{
	if (HasBegunPlay()) { return; }
	Super::BeginPlay();

	auto cognitive = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	if (cognitive.IsValid())
	{
		cognitive->OnSessionBegin.AddDynamic(this, &URoomSize::OnSessionBegin);
		if (cognitive->HasStartedSession())
		{
			OnSessionBegin();
		}
	}
}

void URoomSize::OnSessionBegin()
{
	auto cognitive = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	if (cognitive.IsValid())
	{
		FVector size;
		if (cognitive->TryGetRoomSize(size))
		{
			//convert room size to meters
			float floorsize = (size.X / 100.0f) * (size.Y / 100.0f);
			floorsize = FMath::CeilToInt(floorsize * 100.0f) / 100.0f;

			//format description
			float sizeX = FMath::CeilToInt(size.X) / 100.0f;
			float sizeY = FMath::CeilToInt(size.Y) / 100.0f;
			FString description = FString::SanitizeFloat(sizeX) + " x " + FString::SanitizeFloat(sizeY);

			//write session properties
			cognitive->SetSessionProperty("c3d.roomsizeMeters", floorsize);
			cognitive->SetSessionProperty("c3d.roomsizeDescriptionMeters", description);
		}
	}
}

void URoomSize::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	auto cognitive = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	if (cognitive.IsValid())
	{
		cognitive->OnSessionBegin.RemoveDynamic(this, &URoomSize::OnSessionBegin);
	}
}