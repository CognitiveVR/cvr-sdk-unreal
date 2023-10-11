// Fill out your copyright notice in the Description page of Project Settings.

#include "CognitiveVR/Private/BoundaryEvent.h"
#if ENGINE_MAJOR_VERSION == 4
#include "OculusFunctionLibrary.h"
#elif ENGINE_MAJOR_VERSION == 5
#include "OculusXRFunctionLibrary.h"
#endif
#include "DrawDebugHelpers.h"


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
	StationaryPoints.Add(FVector(75, 50, 0));
	StationaryPoints.Add(FVector(75, -70, 0));
	StationaryPoints.Add(FVector(-65, -70, 0));
	StationaryPoints.Add(FVector(-65, 50, 0));

	world->GetTimerManager().SetTimer(IntervalHandle, FTimerDelegate::CreateUObject(this, &UBoundaryEvent::EndInterval), Interval, true);
}

void UBoundaryEvent::EndInterval()
{
	auto cognitive = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();

	if (BoundaryCrossed)
	{
		UE_LOG(LogTemp, Warning, TEXT("c3d.user.exited.boundary"));
		GEngine->AddOnScreenDebugMessage(3, 8, FColor::Yellow, TEXT("c3d.user.exited.boundary"));
		cognitive->customEventRecorder->Send("c3d.user.exited.boundary");
	}
}

void UBoundaryEvent::OnSessionEnd()
{
	auto world = ACognitiveVRActor::GetCognitiveSessionWorld();
	if (world == nullptr) { return; }
	world->GetTimerManager().ClearTimer(IntervalHandle);
}

//Find out if a point resides inside a polygon
bool UBoundaryEvent::IsPointInPolygon4(TArray<FVector> polygon, FVector testPoint)
{
	bool result = false;
	int j = polygon.Num() - 1;
	for (int i = 0; i < polygon.Num(); i++)
	{
		if (polygon[i].Y < testPoint.Y && polygon[j].Y >= testPoint.Y || polygon[j].Y < testPoint.Y && polygon[i].Y >= testPoint.Y)
		{
			if (polygon[i].X + (testPoint.Y - polygon[i].Y) / (polygon[j].Y - polygon[i].Y) * (polygon[j].X - polygon[i].X) < testPoint.X)
			{
				result = !result;
			}
		}
		j = i;
	}
	return result;
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
		//oculus hmd
#if OCULUS_HMD_SUPPORTED_PLATFORMS

		if (cognitive->TryGetRoomSize(RoomSize))
		{
			if (RoomSize.X > 0 && RoomSize.Y > 0) //room scale boundary
			{
				//get the 4 corners of the boundary in world space
#if ENGINE_MAJOR_VERSION == 4 
				TArray<FVector> GuardianPoints = UOculusFunctionLibrary::GetGuardianPoints(EBoundaryType::Boundary_PlayArea, false);
#elif ENGINE_MAJOR_VERSION == 5
				TArray<FVector> GuardianPoints = UOculusXRFunctionLibrary::GetGuardianPoints(EOculusXRBoundaryType::Boundary_PlayArea, false);
#endif
				for (int i = 0; i < GuardianPoints.Num(); i++)
				{
					FString PointsMessage = "Guardian Boundary Points: " + FString::SanitizeFloat(GuardianPoints[i].X) + ", " + FString::SanitizeFloat(GuardianPoints[i].Y) + ", " + FString::SanitizeFloat(GuardianPoints[i].Z);
					GEngine->AddOnScreenDebugMessage(90 + i, 50, FColor::Blue, PointsMessage);
				}

				//get player position in world space
				cognitive->TryGetPlayerHMDPosition(HMDWorldPos);


				//compare the points
				bool isInsideBoundary = IsPointInPolygon4(GuardianPoints, HMDWorldPos);

				//if we are not inside the boundary anymore, switch the flag to true so it can be captured by the timer event
				if (!isInsideBoundary)
				{
					BoundaryCrossed = true;
				}
				else
				{
					BoundaryCrossed = false;
				}
			}
			else //stationary boundary
			{
				//for stationary, we compare the local (inside tracking area) position of the hmd against pre-set points
#if ENGINE_MAJOR_VERSION == 4 
				UOculusFunctionLibrary::GetPose(HMDRotation, HMDPosition, HMDNeckPos, true, true, FVector::OneVector); //position inside bounds
#elif ENGINE_MAJOR_VERSION == 5
				UOculusXRFunctionLibrary::GetPose(HMDRotation, HMDPosition, HMDNeckPos, true, true, FVector::OneVector);
#endif
				
				cognitive->TryGetPlayerHMDPosition(HMDWorldPos);

				bool isInsideBoundary = IsPointInPolygon4(StationaryPoints, HMDPosition);

				if (!isInsideBoundary)
				{
					BoundaryCrossed = true;
				}
				else
				{
					BoundaryCrossed = false;
				}
			}
		}
	
#else
		//todo: other platforms
		
#endif

	}

}


