// Fill out your copyright notice in the Description page of Project Settings.

#include "CognitiveVR.h"
#include "PlayerTracker.h"
#include "CognitiveVRSettings.h"
#include "Util.h"


// Sets default values for this component's properties
UPlayerTracker::UPlayerTracker()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	bWantsBeginPlay = true;
	PrimaryComponentTick.bCanEverTick = true;

	FString ValueReceived;

	ValueReceived = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "GazeBatchSize", false);

	if (ValueReceived.Len() > 0)
	{
		int32 sensorLimit = FCString::Atoi(*ValueReceived);
		if (sensorLimit > 0)
		{
			GazeBatchSize = sensorLimit;
		}
	}
}

void UPlayerTracker::InitializePlayerTracker()
{
	if (SceneDepthMat == NULL)
	{
		UMaterialInterface *materialInterface = Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), NULL, *materialPath));
		if (materialInterface != NULL)
		{
			SceneDepthMat = materialInterface->GetMaterial();
		}
	}
	s = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider();
}

void UPlayerTracker::BeginPlay()
{
	FlushPersistentDebugLines(GetWorld());

	InitializePlayerTracker();

	Super::BeginPlay();
	//Http = &FHttpModule::Get();

	UTextureRenderTarget2D* renderTarget;
	renderTarget = NewObject<UTextureRenderTarget2D>();
	renderTarget->AddToRoot();
	renderTarget->ClearColor = FLinearColor::White;
	renderTarget->bHDR = false;
	renderTarget->InitAutoFormat(256, 256); //auto init from value bHDR

	SceneCaptureActor = GetWorld()->SpawnActor<AActor>(FVector::ZeroVector, FRotator::ZeroRotator);
	//SceneCaptureActor->SetActorLabel("Scene Capture Actor");

	transformComponent = NewObject<USceneComponent>(SceneCaptureActor);

	sceneCapture = NewObject<USceneCaptureComponent2D>(transformComponent);
	//sceneCapture->SetupAttachment(this);
	//sceneCapture->SetRelativeLocation(FVector::ZeroVector);
	//sceneCapture->SetRelativeRotation(FQuat::Identity);
	sceneCapture->TextureTarget = renderTarget;
	sceneCapture->RegisterComponent();
	sceneCapture->CaptureSource = SCS_FinalColorLDR;
	//sceneCapture->

	sceneCapture->PostProcessSettings.AddBlendable(SceneDepthMat, 1);
	//sceneCapture->bCaptureEveryFrame = false;
	//sceneCapture->bCaptureOnMovement = false;
}

void UPlayerTracker::AddJsonEvent(FJsonObject* newEvent)
{
	TSharedPtr<FJsonObject>snapObj = MakeShareable(newEvent);
	events.Add(snapObj);
}

// Called every frame
void UPlayerTracker::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	if (s.Get() == NULL)
	{
		return;
	}

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	framesTillRender--;

	if (waitingForDeferred && framesTillRender < 0)
	{
		waitingForDeferred = false;

		TArray<APlayerController*, FDefaultAllocator> controllers;
		GEngine->GetAllLocalPlayerControllers(controllers);
		if (controllers.Num() == 0)
		{
			CognitiveLog::Info("--------------------------no controllers");
			//return FVector();
			return;
		}
		/*
		//FVector finalLoc;
		FVector loc;
		FRotator rot;

		controllers[0]->GetPlayerViewPoint(loc, rot);

		FTransform camManTransform = controllers[0]->PlayerCameraManager->GetActorTransform();
		//FVector camManForward = controllers[0]->PlayerCameraManager->GetActorForwardVector();

		FRotator hmdrot;
		FVector hmdpos;
		UHeadMountedDisplayFunctionLibrary::GetOrientationAndPosition(hmdrot, hmdpos);

		FVector finalPos = camManTransform.TransformPosition(hmdrot.UnrotateVector(hmdpos));
		//sceneCapture->SetWorldRotation(rot);
		//sceneCapture->SetWorldLocation(finalPos);
		*/
		//CognitiveLog::Info("--------------------------TICK");
		//write to json

		TSharedPtr<FJsonObject>snapObj = MakeShareable(new FJsonObject);

		double ts = Util::GetTimestamp();

		//time
		snapObj->SetNumberField("time", ts);

		//positions
		TArray<TSharedPtr<FJsonValue>> posArray;
		TSharedPtr<FJsonValueNumber> JsonValue;
		JsonValue = MakeShareable(new FJsonValueNumber(-(int32)captureLocation.X)); //right
		posArray.Add(JsonValue);
		JsonValue = MakeShareable(new FJsonValueNumber((int32)captureLocation.Z)); //up
		posArray.Add(JsonValue);
		JsonValue = MakeShareable(new FJsonValueNumber((int32)captureLocation.Y));  //forward
		posArray.Add(JsonValue);

		snapObj->SetArrayField("p", posArray);



		//look at dynamic object
		
		FCollisionQueryParams Params; // You can use this to customize various properties about the trace
		Params.AddIgnoredActor(GetOwner()); // Ignore the player's pawn

		
		FHitResult Hit; // The hit result gets populated by the line trace

		// Raycast out from the camera, only collide with pawns (they are on the ECC_Pawn collision channel)
		FVector Start = captureLocation;
		FVector End = captureLocation + captureRotation.Vector() * 10000.0f;
		bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Pawn, Params);

		//gaze
		TArray<TSharedPtr<FJsonValue>> gazeArray;

		bool hitDynamic = false;
		if (bHit)
		{
			// Hit.Actor contains a weak pointer to the Actor that the trace hit
			//return Cast<APawn>(Hit.Actor.Get());
			//GLog->Log("hit "+Hit.Actor.Get()->GetName());
			UActorComponent* hitActorComponent = Hit.Actor.Get()->GetComponentByClass(UDynamicObject::StaticClass());
			if (hitActorComponent != NULL)
			{
				UDynamicObject* hitDynamicObject = Cast<UDynamicObject>(hitActorComponent);
				if (hitDynamicObject != NULL && hitDynamicObject->TrackGaze)
				{
					hitDynamic = true;

					FVector localHitPosition = hitDynamicObject->GetOwner()->GetActorTransform().InverseTransformPosition(Hit.ImpactPoint);

					localHitPosition *= hitDynamicObject->GetOwner()->GetActorTransform().GetScale3D();

					DrawDebugLine(
						GetWorld(),
						hitDynamicObject->GetOwner()->GetActorLocation() + localHitPosition,
						hitDynamicObject->GetOwner()->GetActorLocation() + localHitPosition + FVector::UpVector * 100,
						FColor(255, 0, 0),
						false, 3, 0,
						3
					);

					DrawDebugLine(
						GetWorld(),
						Hit.ImpactPoint,
						Hit.ImpactPoint + FVector::UpVector * 50,
						FColor(0, 255, 0),
						false, 3, 0,
						3
					);

					snapObj->SetNumberField("o", hitDynamicObject->GetObjectId()->Id);

					//FVector gazePoint = GetGazePoint(captureLocation, captureRotation);
					JsonValue = MakeShareable(new FJsonValueNumber(-(int32)localHitPosition.X));
					gazeArray.Add(JsonValue);
					JsonValue = MakeShareable(new FJsonValueNumber((int32)localHitPosition.Z));
					gazeArray.Add(JsonValue);
					JsonValue = MakeShareable(new FJsonValueNumber((int32)localHitPosition.Y));
					gazeArray.Add(JsonValue);
				}
			}
		}

		if (!hitDynamic)
		{
			FVector gazePoint = GetGazePoint(captureLocation, captureRotation);
			JsonValue = MakeShareable(new FJsonValueNumber(-(int32)gazePoint.X));
			gazeArray.Add(JsonValue);
			JsonValue = MakeShareable(new FJsonValueNumber((int32)gazePoint.Z));
			gazeArray.Add(JsonValue);
			JsonValue = MakeShareable(new FJsonValueNumber((int32)gazePoint.Y));
			gazeArray.Add(JsonValue);
		}

		//DrawDebugLine(GetWorld(), finalPos, gazePoint, FColor(100, 0, 100), true, 10);
		//DrawDebugPoint(GetWorld(), gazePoint, 20, FColor(255, 0, 255), true, 10);

		snapObj->SetArrayField("g", gazeArray);

		//rotation
		TArray<TSharedPtr<FJsonValue>> rotArray;

		FQuat quat;
		FRotator adjustedRot = captureRotation;
		adjustedRot.Yaw -= 90;
		quat = adjustedRot.Quaternion();

		JsonValue = MakeShareable(new FJsonValueNumber(quat.Y));
		rotArray.Add(JsonValue);
		JsonValue = MakeShareable(new FJsonValueNumber(quat.Z));
		rotArray.Add(JsonValue);
		JsonValue = MakeShareable(new FJsonValueNumber(quat.X));
		rotArray.Add(JsonValue);
		JsonValue = MakeShareable(new FJsonValueNumber(quat.W));
		rotArray.Add(JsonValue);

		snapObj->SetArrayField("r", rotArray);


		snapshots.Add(snapObj);
		if (snapshots.Num() > GazeBatchSize)
		{
			SendGazeEventDataToSceneExplorer();
			//s->FlushEvents();
			snapshots.Empty();
			events.Empty();
		}
	}

	currentTime += DeltaTime;
	if (currentTime > PlayerSnapshotInterval)
	{
		currentTime -= PlayerSnapshotInterval;

		TArray<APlayerController*, FDefaultAllocator> controllers;
		GEngine->GetAllLocalPlayerControllers(controllers);
		if (controllers.Num() == 0)
		{
			CognitiveLog::Info("--------------------------no controllers");
			//return FVector();
			return;
		}

		//FVector finalLoc;
		FVector loc;
		FRotator rot;

		controllers[0]->GetPlayerViewPoint(loc, rot);

		FTransform camManTransform = controllers[0]->PlayerCameraManager->GetActorTransform();
		//FVector camManForward = controllers[0]->PlayerCameraManager->GetActorForwardVector();

		FRotator hmdrot;
		FVector hmdpos;
		UHeadMountedDisplayFunctionLibrary::GetOrientationAndPosition(hmdrot, hmdpos);

		FVector finalPos = camManTransform.TransformPosition(hmdrot.UnrotateVector(hmdpos));
		sceneCapture->SetWorldRotation(rot);
		sceneCapture->SetWorldLocation(finalPos);

		captureLocation = finalPos;
		captureRotation = rot;

		//UE_LOG(LogTemp, Warning, TEXT("SceneCaptureActor Location is %s"), *SceneCaptureActor->GetActorLocation().ToString());

		sceneCapture->CaptureSceneDeferred();
		//sceneCapture->CaptureScene();
		waitingForDeferred = true;
		framesTillRender = 1;
	}
}

float UPlayerTracker::GetPixelDepth(float minvalue, float maxvalue)
{
	if (sceneCapture->TextureTarget == NULL)
	{
		CognitiveLog::Warning("UPlayerTracker::GetPixelDepth render target size is null");

		return -1;
	}

	// Creates Texture2D to store TextureRenderTarget content
	UTexture2D *Texture = UTexture2D::CreateTransient(sceneCapture->TextureTarget->SizeX, sceneCapture->TextureTarget->SizeY, PF_B8G8R8A8);

	if (Texture == NULL)
	{
		CognitiveLog::Warning("UPlayerTracker::GetPixelDepth TEMP Texture IS NULL");

		return -1;
	}

#if WITH_EDITORONLY_DATA
	Texture->MipGenSettings = TMGS_NoMipmaps; //not sure if this is required
#endif

	Texture->SRGB = sceneCapture->TextureTarget->SRGB;

	TArray<FColor> SurfData;

	FRenderTarget *RenderTarget = sceneCapture->TextureTarget->GameThread_GetRenderTargetResource();

	RenderTarget->ReadPixels(SurfData);

	// Index formula

	FIntPoint size = RenderTarget->GetSizeXY();

	FColor PixelColor = SurfData[size.X / 2 + size.Y / 2 * sceneCapture->TextureTarget->SizeX];

	float nf = (double)PixelColor.R / 255.0;

	float distance = FMath::Lerp(minvalue, maxvalue, nf);

	//CognitiveLog::Info("--------------------------PlayerTracker::GetPixelDepth %d");
	//UE_LOG(LogTemp, Warning, TEXT("--------------------------PlayerTracker::GetPixelDepth %f"), distance);

	return distance;
}

//puts together the world position of the player's gaze point. each tick
FVector UPlayerTracker::GetGazePoint(FVector location, FRotator rotator)
{
	float distance = GetPixelDepth(0, maxDistance);
	//FRotator rot = GetComponentRotation();
	FVector rotv = rotator.Vector();
	rotv *= distance;

	//add location
	//FVector loc = location;

	FVector returnVector;
	returnVector.X = location.X + rotv.X;
	returnVector.Y = location.Y + rotv.Y;
	returnVector.Z = location.Z + rotv.Z;
	return returnVector;
}

void UPlayerTracker::SendGazeEventDataToSceneExplorer()
{
	UWorld* myworld = GetWorld();
	if (myworld == NULL)
	{
		CognitiveLog::Info("--------------------------WORLD DOESNT EXIST");
		return;
	}

	FString currentSceneName = myworld->GetMapName();
	currentSceneName.RemoveFromStart(myworld->StreamingLevelsPrefix);
	UPlayerTracker::SendGazeEventDataToSceneExplorer(currentSceneName);
}



void UPlayerTracker::SendGazeEventDataToSceneExplorer(FString sceneName)
{
	//second send events and gaze to scene explorer using the player tracker
	TArray<APlayerController*, FDefaultAllocator> controllers;
	GEngine->GetAllLocalPlayerControllers(controllers);
	if (controllers.Num() == 0)
	{
		CognitiveLog::Warning("PlayerTracker::SendJson no local player controllers");
		return;
	}
	UPlayerTracker* up = controllers[0]->GetPawn()->FindComponentByClass<UPlayerTracker>();
	if (up == NULL)
	{
		CognitiveLog::Warning("PlayerTracker::SendJson no player tracker component");
		return;
	}

	UWorld* myworld = up->GetWorld();
	if (myworld == NULL)
	{
		CognitiveLog::Warning("PlayerTracker::SendJson no world");
		return;
	}
	CognitiveLog::Info("UPlayerTracker::SendData");

	//GAZE

	//TODO where do i clear snapshots?
	FString GazeString = UPlayerTracker::GazeSnapshotsToString();
	//FAnalyticsProviderCognitiveVR::SendJson("gaze", GazeString);
	s->SendJson("gaze", GazeString);
	//EVENTS

	FString EventString = UPlayerTracker::EventSnapshotsToString();
	//FAnalyticsProviderCognitiveVR::SendJson("events", EventString);
	s->SendJson("events", EventString);
}

FString UPlayerTracker::EventSnapshotsToString()
{
	TSharedPtr<FJsonObject>wholeObj = MakeShareable(new FJsonObject);
	TArray<TSharedPtr<FJsonValue>> dataArray;
	FAnalyticsProviderCognitiveVR* cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Get();

	wholeObj->SetStringField("userid", cog->GetDeviceID());
	wholeObj->SetNumberField("timestamp", (int32)cog->GetSessionTimestamp());
	wholeObj->SetStringField("sessionid", cog->GetCognitiveSessionID());
	wholeObj->SetNumberField("part", jsonEventPart);
	jsonEventPart++;

	for (int32 i = 0; i != events.Num(); ++i)
	{
		dataArray.Add(MakeShareable(new FJsonValueObject(events[i])));
	}

	wholeObj->SetArrayField("data", dataArray);

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(wholeObj.ToSharedRef(), Writer);
	return OutputString;
}

FString UPlayerTracker::GazeSnapshotsToString()
{
	TSharedPtr<FJsonObject>wholeObj = MakeShareable(new FJsonObject);
	TArray<TSharedPtr<FJsonValue>> dataArray;
	FAnalyticsProviderCognitiveVR* cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Get();

	wholeObj->SetStringField("userid", cog->GetDeviceID());
	wholeObj->SetNumberField("timestamp", (int32)cog->GetSessionTimestamp());
	wholeObj->SetStringField("sessionid", cog->GetCognitiveSessionID());
	wholeObj->SetNumberField("part", jsonGazePart);
	jsonGazePart++;

	FName DeviceName(NAME_None);
	FString DeviceNameString = "unknown";

	if (GEngine->HMDDevice.IsValid())
	{
		DeviceName = GEngine->HMDDevice->GetDeviceName();
		DeviceNameString = cognitivevrapi::Util::GetDeviceName(DeviceName.ToString());
	}

	wholeObj->SetStringField("hmdtype", DeviceName.ToString());

	for (int32 i = 0; i != snapshots.Num(); ++i)
	{
		TSharedPtr<FJsonValueObject> snapshotValue;
		snapshotValue = MakeShareable(new FJsonValueObject(snapshots[i]));
		dataArray.Add(snapshotValue);
	}

	wholeObj->SetArrayField("data", dataArray);

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(wholeObj.ToSharedRef(), Writer);
	return OutputString;
}

/*
void UPlayerTracker::BlueprintSendData()
{

TSharedPtr<IAnalyticsProvider> Provider = FAnalytics::Get().GetDefaultConfiguredProvider();
FAnalyticsProviderCognitiveVR* cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Get();
if (!Provider.IsValid() || !bHasSessionStarted || cog == NULL)
{
CognitiveLog::Error("UCognitiveVRBlueprints::BeginEndTransaction could not get provider!");
return;
}

TArray<APlayerController*, FDefaultAllocator> controllers;
GEngine->GetAllLocalPlayerControllers(controllers);
if (controllers.Num() == 0)
{
CognitiveLog::Info("UPlayerTracker::RequestSendData no player controllers");
return;
}
UPlayerTracker* up = controllers[0]->GetPawn()->FindComponentByClass<UPlayerTracker>();
if (up == NULL)
{
CognitiveLog::Warning("UPlayerTracker::RequestSendData No Player Tracker Component! cannot send data?!?....there's got to be a better way!");
return;
}
up->SendData();

}
*/

void UPlayerTracker::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	FString reason;
	switch (EndPlayReason)
	{
	case EEndPlayReason::Destroyed: reason = "destroyed";
		break;
	case EEndPlayReason::EndPlayInEditor: reason = "end PIE";
		break;
	case EEndPlayReason::LevelTransition: reason = "level transition";
		break;
	case EEndPlayReason::Quit: reason = "quit";
		break;
	case EEndPlayReason::RemovedFromWorld: reason = "removed from world";
		break;
	default:
		reason = "default";
		break;
	}

	GLog->Log("player tracker end play reason " + reason);
	if (s.Get() != NULL)
	{
		GLog->Log("PLAYER TRACKER END PLAY. END SESSION TOO");
		s->EndSession();
	}
}
/*

CognitiveLog::Info("UPlayerTracker::EndPlay reason:" + reason);

//TODO this doesn't work, but it totally crashes when session has not been started

if (s.Get() == NULL)
{
CognitiveLog::Info("UPlayerTracker::EndPlay CognitiveVRProvider is null. do not send data on end play");
}
else
{
if (SendDataOnEndPlay)
{
s->FlushEvents();
}
if (EndSessionOnEndPlay)
{
s->EndSession();
}
else
{
if (s.Get()->transaction == NULL)
{
CognitiveLog::Info("UPlayerTracker::EndPlay transactions is null. cannot end session");
}
else
{
s->transaction->End("Session");
}
}
}

Super::EndPlay(EndPlayReason);
}
*/