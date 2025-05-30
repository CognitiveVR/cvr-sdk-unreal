/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/

#include "Cognitive3D/Private/C3DComponents/BoundaryEvent.h"
#include "DrawDebugHelpers.h"
#include "Interfaces/IPluginManager.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#ifdef INCLUDE_PICO_PLUGIN
#include "PXR_HMDFunctionLibrary.h"
#endif
#include "Cognitive3D/Public/Cognitive3DActor.h"
#include "Cognitive3D/Private/C3DApi/CustomEventRecorder.h"

// Sets default values
UBoundaryEvent::UBoundaryEvent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


void UBoundaryEvent::BeginPlay()
{
	if (HasBegunPlay()) { return; }
	Super::BeginPlay();

	auto cognitive = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (cognitive.IsValid())
	{
		//set boundary type session property
		if (cognitive->TryGetRoomSize(RoomSize))
		{
			if (RoomSize.X == RoomSize.Y) //stationary, both dimensions exactly the same. UE5 shows 1.21
			{
				FString boundaryType = TEXT("Stationary");
				cognitive->SetSessionProperty(TEXT("c3d.boundaryType"), boundaryType);
			}
			else if (RoomSize.X > 0 && RoomSize.Y > 0) //room scale boundary
			{
				FString boundaryType = TEXT("Room Scale");
				cognitive->SetSessionProperty(TEXT("c3d.boundaryType"), boundaryType);
			}
			else //stationary boundary
			{
				FString boundaryType = TEXT("Stationary");
				cognitive->SetSessionProperty(TEXT("c3d.boundaryType"), boundaryType);
			}
		}

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
	auto world = ACognitive3DActor::GetCognitiveSessionWorld();
	if (world == nullptr) { return; }

	BoundaryCrossed = false;
	StillOutsideBoundary = false;
	PicoCheckBoundary = false;
	StationaryPoints.Add(FVector(75, 50, 0));
	StationaryPoints.Add(FVector(75, -70, 0));
	StationaryPoints.Add(FVector(-65, -70, 0));
	StationaryPoints.Add(FVector(-65, 50, 0));

	world->GetTimerManager().SetTimer(IntervalHandle, FTimerDelegate::CreateUObject(this, &UBoundaryEvent::EndInterval), Interval, true);
}

void UBoundaryEvent::EndInterval()
{
	auto cognitive = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();

#ifdef INCLUDE_PICO_PLUGIN
	bool isPicoTriggered = false;
	float PicoClosestDistance = 0;
	FVector PicoClosestPoint;
	FVector PicoClosestPointNormal;
	UPICOXRHMDFunctionLibrary::PXR_BoundaryTestNode(EPICOXRNodeType::Head, EPICOXRBoundaryType::PlayArea, isPicoTriggered, PicoClosestDistance, PicoClosestPoint, PicoClosestPointNormal);
	if (!PicoCheckBoundary && isPicoTriggered)
	{
		PicoCheckBoundary = true;
		cognitive->customEventRecorder->Send("c3d.user.exited.boundary");
	}
	else if (PicoCheckBoundary && !isPicoTriggered)
	{
		PicoCheckBoundary = false;
	}
#else
	if (BoundaryCrossed && !StillOutsideBoundary)
	{
		cognitive->customEventRecorder->Send("c3d.user.exited.boundary");
		StillOutsideBoundary = true;
	}
#endif
}

void UBoundaryEvent::OnSessionEnd()
{
	auto world = ACognitive3DActor::GetCognitiveSessionWorld();
	if (world == nullptr) { return; }
	world->GetTimerManager().ClearTimer(IntervalHandle);
}


void UBoundaryEvent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	auto cognitive = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
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
				cognitive->TryGetHMDGuardianPoints(GuardianPoints, false);

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


