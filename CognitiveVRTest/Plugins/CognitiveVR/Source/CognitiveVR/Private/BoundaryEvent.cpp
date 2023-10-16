// Fill out your copyright notice in the Description page of Project Settings.

#include "CognitiveVR/Private/BoundaryEvent.h"
#include "DrawDebugHelpers.h"
#include "Interfaces/IPluginManager.h"

// Sets default values
UBoundaryEvent::UBoundaryEvent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


void UBoundaryEvent::BeginPlay()
{
	if (HasBegunPlay()) { return; }
	Super::BeginPlay();

	auto cognitive = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	if (cognitive.IsValid())
	{
		cognitive->OnSessionBegin.AddDynamic(this, &UBoundaryEvent::OnSessionBegin);
		cognitive->OnPreSessionEnd.AddDynamic(this, &UBoundaryEvent::OnSessionEnd);
		if (cognitive->HasStartedSession())
		{
			OnSessionBegin();
		}
	}
}


void UBoundaryEvent::OnSessionBegin()
{
	auto world = ACognitiveVRActor::GetCognitiveSessionWorld();
	if (world == nullptr) { return; }
	BoundaryCrossed = false;
	StillOutsideBoundary = false;
	StationaryPoints.Add(FVector(75, 50, 0));
	StationaryPoints.Add(FVector(75, -70, 0));
	StationaryPoints.Add(FVector(-65, -70, 0));
	StationaryPoints.Add(FVector(-65, 50, 0));

	world->GetTimerManager().SetTimer(IntervalHandle, FTimerDelegate::CreateUObject(this, &UBoundaryEvent::EndInterval), Interval, true);
}

void UBoundaryEvent::EndInterval()
{
	auto cognitive = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();

	if (BoundaryCrossed && !StillOutsideBoundary)
	{
		cognitive->customEventRecorder->Send("c3d.user.exited.boundary");
		StillOutsideBoundary = true;
	}
}

void UBoundaryEvent::OnSessionEnd()
{
	auto world = ACognitiveVRActor::GetCognitiveSessionWorld();
	if (world == nullptr) { return; }
	world->GetTimerManager().ClearTimer(IntervalHandle);
}


void UBoundaryEvent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	auto cognitive = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	if (!cognitive.IsValid())
	{
		return;
	}
	if (!cognitive->HasStartedSession())
	{
		return;
	}

	//check if player hmd is outside of bounds
	if (UHeadMountedDisplayFunctionLibrary::HasValidTrackingPosition())
	{

		if (cognitive->TryGetRoomSize(RoomSize))
		{
			if (RoomSize.X > 0 && RoomSize.Y > 0) //room scale boundary
			{
				//get the 4 corners of the boundary in world space
				cognitive->TryGetHMDGuardianPoints(GuardianPoints);

				//get player position in world space
				cognitive->TryGetPlayerHMDPosition(HMDWorldPos);


				//compare the points
				bool isInsideBoundary = cognitive->IsPointInPolygon4(GuardianPoints, HMDWorldPos);

				//we do this instead of using the functions to check for node or point intersection 
				//because they were tested and shown to not be consistent amongst UE versions and also
				//would not work properly when deployed to the quest 2.


				//if we are not inside the boundary anymore, switch the flag to true so it can be captured by the timer event
				if (!isInsideBoundary)
				{
					BoundaryCrossed = true;
				}
				else
				{
					BoundaryCrossed = false;
					StillOutsideBoundary = false;
				}
			}
			else //stationary boundary
			{
				//for stationary, we compare the local (inside tracking area) position of the hmd against pre-set points'
				cognitive->TryGetHMDPose(HMDRotation, HMDPosition, HMDNeckPos);

				bool isInsideBoundary = cognitive->IsPointInPolygon4(StationaryPoints, HMDPosition);


				if (!isInsideBoundary)
				{
					BoundaryCrossed = true;
				}
				else
				{
					BoundaryCrossed = false;
					StillOutsideBoundary = false;
				}
			}
		}

	}

}


