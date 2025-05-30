// Fill out your copyright notice in the Description page of Project Settings.


#include "C3DApi/BoundaryRecorder.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#ifdef INCLUDE_PICO_PLUGIN
#include "PXR_HMDFunctionLibrary.h"
#endif
#include "Cognitive3D/Public/Cognitive3DActor.h"
#include "Cognitive3D/Private/C3DApi/CustomEventRecorder.h"
#include "Cognitive3D/Private/C3DNetwork/Network.h"
#include <Kismet/GameplayStatics.h>

BoundaryRecorder::BoundaryRecorder()
{
	cog = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
}

BoundaryRecorder::~BoundaryRecorder()
{
}

void BoundaryRecorder::StartSession()
{
	Data.Empty();
	Shapes.Empty();
	jsonPart = 1;
	FString ValueReceived;
	FString C3DSettingsPath = cog->GetSettingsFilePathRuntime();
	GConfig->LoadFile(C3DSettingsPath);
	//gaze batch size
	ValueReceived = FAnalytics::Get().GetConfigValueFromIni(C3DSettingsPath, "/Script/Cognitive3D.Cognitive3DSettings", "GazeBatchSize", false);
	if (ValueReceived.Len() > 0)
	{
		int32 sensorLimit = FCString::Atoi(*ValueReceived);
		if (sensorLimit > 0)
		{
			//GazeBatchSize = sensorLimit;
		}
	}
	auto world = ACognitive3DActor::GetCognitiveSessionWorld();
	if (world == nullptr)
	{
		GLog->Log("BoundaryRecorder::StartSession world from ACognitive3DActor is null!");
		return;
	}
	
	///
	if (world)
	{
		FTimerDelegate TimerDel;
		TimerDel.BindLambda([this]()
			{
				this->PerformInitialCapture();
			});

		world->GetTimerManager().SetTimer(
			DelayedCaptureTimerHandle,
			TimerDel,
			3.0f,
			false
		);

		FTimerDelegate IntervalTimer;
		IntervalTimer.BindLambda([this]()
			{
				this->BoundaryCheckInterval();
			});

		world->GetTimerManager().SetTimer(
			IntervalTimerHandle,
			IntervalTimer,
			0.1f,
			true,
			3.1f
		);
	}
}

void BoundaryRecorder::PreSessionEnd()
{
}

void BoundaryRecorder::PostSessionEnd()
{
	auto world = ACognitive3DActor::GetCognitiveSessionWorld();
	if (world == nullptr)
	{
		GLog->Log("BoundaryRecorder::StartSession world from ACognitive3DActor is null!");
		return;
	}
	world->GetTimerManager().ClearTimer(DelayedCaptureTimerHandle);
}

bool BoundaryRecorder::SerializeToJsonString(FString& OutJsonString)
{
	TSharedRef<FJsonObject> RootObject = MakeShared<FJsonObject>();

	// --- Data Array ---
	TArray<TSharedPtr<FJsonValue>> DataArray;
	for (const FDataEntry& Entry : Data)
	{
		TSharedRef<FJsonObject> DataObj = MakeShared<FJsonObject>();
		DataObj->SetNumberField(TEXT("time"), Entry.Time);

		// Validate P and R
		if (!Entry.P.ContainsNaN() && !Entry.R.ContainsNaN())
		{
			TArray<TSharedPtr<FJsonValue>> PArray = {
				MakeShared<FJsonValueNumber>(-Entry.P.X),
				MakeShared<FJsonValueNumber>(Entry.P.Z),
				MakeShared<FJsonValueNumber>(Entry.P.Y)
			};
			DataObj->SetArrayField(TEXT("p"), PArray);

			TArray<TSharedPtr<FJsonValue>> RArray = {
				MakeShared<FJsonValueNumber>(Entry.R.Y),  //x //y
				MakeShared<FJsonValueNumber>(Entry.R.Z),  //y //z
				MakeShared<FJsonValueNumber>(Entry.R.X),  //z //x
				MakeShared<FJsonValueNumber>(Entry.R.W)   //w //w
			};
			DataObj->SetArrayField(TEXT("r"), RArray);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Invalid NaN in position or rotation!"));
		}

		DataArray.Add(MakeShared<FJsonValueObject>(DataObj));
	}
	RootObject->SetArrayField(TEXT("data"), DataArray);

	// --- Shapes Array ---
	TArray<TSharedPtr<FJsonValue>> ShapesArray;
	for (const FShapeEntry& Shape : Shapes)
	{
		TSharedRef<FJsonObject> ShapeObj = MakeShared<FJsonObject>();
		ShapeObj->SetNumberField(TEXT("time"), Shape.Time);

		// Points array
		TArray<TSharedPtr<FJsonValue>> PointsArray;
		for (const FVector& Point : Shape.Points)
		{
			if (!Point.ContainsNaN())
			{
				TArray<TSharedPtr<FJsonValue>> SinglePoint = {
					MakeShared<FJsonValueNumber>(-Point.X),
					MakeShared<FJsonValueNumber>(Point.Z),
					MakeShared<FJsonValueNumber>(Point.Y)
				};
				PointsArray.Add(MakeShared<FJsonValueArray>(SinglePoint));
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Invalid NaN in boundary point!"));
			}
		}
		ShapeObj->SetArrayField(TEXT("points"), PointsArray);

		ShapesArray.Add(MakeShared<FJsonValueObject>(ShapeObj));
	}
	RootObject->SetArrayField(TEXT("shapes"), ShapesArray);

	RootObject->SetStringField(TEXT("userid"), cog->GetUserID());
	RootObject->SetNumberField(TEXT("timestamp"), cog->GetSessionTimestamp());
	RootObject->SetStringField(TEXT("sessionid"), cog->GetSessionID());

	TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> Writer = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&OutJsonString);
	bool bSuccess = FJsonSerializer::Serialize(RootObject, Writer);
	Writer->Close();

	if (!bSuccess)
	{
		UE_LOG(LogTemp, Error, TEXT("FJsonSerializer::Serialize failed"));
	}

	return bSuccess;
}

bool BoundaryRecorder::DeserializeFromJsonString(const FString& InJsonString)
{
	return false;
}

void BoundaryRecorder::AddGuardianBoundaryPoints(TArray<FVector>& GuardianPoints)
{
	FShapeEntry ShapeEntry;
	ShapeEntry.Time = FUtil::GetTimestamp();

	for (const FVector& Point : GuardianPoints)
	{
		ShapeEntry.Points.Add(Point);
	}

	Shapes.Add(ShapeEntry);
}

void BoundaryRecorder::CapturePlayerTransform()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GWorld, 0);
	if (!PC) return;

	APawn* Pawn = PC->GetPawn();
	if (!Pawn) return;

	FVector PlayerLocation = Pawn->GetActorLocation();
	LastPlayerLocation = PlayerLocation;
	FRotator PlayerRotation = Pawn->GetActorRotation();
	FQuat PlayerQuat = PlayerRotation.Quaternion();

	BoundaryRecorder::FDataEntry Entry;
	Entry.Time = FUtil::GetTimestamp();
	Entry.P = PlayerLocation;
	Entry.R = PlayerQuat;

	Data.Add(Entry);
}

void BoundaryRecorder::BoundaryCheckInterval()
{
	if (!cog) return;
	if (!cog->HasStartedSession()) return;
	bool bShouldSend = false;

	// Check if the player has moved significantly
	APlayerController* PC = UGameplayStatics::GetPlayerController(GWorld, 0);
	if (PC)
	{
		APawn* Pawn = PC->GetPawn();
		if (Pawn)
		{
			FVector PlayerLocation = Pawn->GetActorLocation();
			if ((PlayerLocation - LastPlayerLocation).SizeSquared() > MovementThresholdSqr) // Adjust threshold as needed
			{
				Data.Empty();
				CapturePlayerTransform();
				bShouldSend = true;
			}
		}
	}

	// check for changes in the boundary/guardian points
	TArray<FVector> GuardianPoints;
	if (cog->TryGetHMDGuardianPoints(GuardianPoints, true))
	{
		if (HaveBoundaryPointsChanged(GuardianPoints))
		{
			Shapes.Empty();
			AddGuardianBoundaryPoints(GuardianPoints);
			LastCapturedGuardianPoints = GuardianPoints;
			bShouldSend = true;
		}
	}

	if (bShouldSend)
	{
		SendData(false);
	}
}

void BoundaryRecorder::SendData(bool bCacheData)
{
	if (!SerializeToJsonString(JsonDataString))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to serialize JSON"));
		return;
	}

	if (JsonDataString.Len() > 0)
	{
		cog->network->NetworkCall("boundary", JsonDataString, bCacheData);
	}
}

void BoundaryRecorder::PerformInitialCapture()
{
	//UE_LOG(LogTemp, Log, TEXT("Performing Initial Capture"));
	if (!cog)
	{
		UE_LOG(LogTemp, Error, TEXT("Cached Cognitive reference is null."));
		return;
	}
	auto world = ACognitive3DActor::GetCognitiveSessionWorld();
	if (world == nullptr)
	{
		GLog->Log("BoundaryRecorder::StartSession world from ACognitive3DActor is null!");
		return;
	}

	// --- Capture Player Transform ---
	APlayerController* PC = UGameplayStatics::GetPlayerController(world, 0);
	if (PC)
	{
		APawn* Pawn = PC->GetPawn();
		if (Pawn)
		{
			FVector PlayerLocation = Pawn->GetActorLocation();//(-Pawn->GetActorLocation().X, Pawn->GetActorLocation().Z, Pawn->GetActorLocation().Y);
			LastPlayerLocation = PlayerLocation;
			FRotator PlayerRotation = Pawn->GetActorRotation();
			FQuat PlayerQuat = PlayerRotation.Quaternion();

			FDataEntry Entry;
			Entry.Time = FUtil::GetTimestamp(); //cog->GetSessionTimestamp();
			Entry.P = PlayerLocation;
			Entry.R = PlayerQuat;

			Data.Add(Entry);
			UE_LOG(LogTemp, Log, TEXT("Captured Player Transform: %s | %s"), *PlayerLocation.ToString(), *PlayerQuat.ToString());
		}
	}

	// --- Capture Guardian Boundary Points ---
	TArray<FVector> GuardianPoints;
	if (cog->TryGetHMDGuardianPoints(GuardianPoints, true))
	{
		AddGuardianBoundaryPoints(GuardianPoints);
		LastCapturedGuardianPoints = GuardianPoints;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to get Guardian Points."));
	}

	// --- Send JSON after capturing everything ---
	SendData(false); // Or true, if you plan to cache

}

bool BoundaryRecorder::HaveBoundaryPointsChanged(const TArray<FVector>& NewPoints) const
{
	if (NewPoints.Num() != LastCapturedGuardianPoints.Num())
	{
		return true;
	}
	for (int32 i = 0; i < NewPoints.Num(); ++i)
	{
		// FVector::Equals uses a tolerance on each component
		if (!NewPoints[i].Equals(LastCapturedGuardianPoints[i], BoundaryTolerance))
		{
			return true;
		}
	}
	return false;
}
