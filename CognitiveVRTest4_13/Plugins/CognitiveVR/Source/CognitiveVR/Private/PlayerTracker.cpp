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

	s->SetWorld(GetWorld());

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
			CognitiveLog::Info("UPlayerTracker::TickComponent--------------------------no controllers. skip");
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
			//GLog->Log("hit "+Hit.Actor.Get()->GetName());
			UActorComponent* hitActorComponent = Hit.Actor.Get()->GetComponentByClass(UDynamicObject::StaticClass());
			if (hitActorComponent != NULL)
			{
				UDynamicObject* hitDynamicObject = Cast<UDynamicObject>(hitActorComponent);
				if (hitDynamicObject != NULL && hitDynamicObject->TrackGaze && hitDynamicObject->GetObjectId().IsValid())
				{
					hitDynamic = true;

					FVector localHitPosition = hitDynamicObject->GetOwner()->GetActorTransform().InverseTransformPosition(Hit.ImpactPoint);

					localHitPosition *= hitDynamicObject->GetOwner()->GetActorTransform().GetScale3D();

					/*DrawDebugLine(
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
					);*/



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
			CognitiveLog::Info("UPlayerTracker::TickComponent--------------------------no controllers skip");
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

		sceneCapture->CaptureSceneDeferred();
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

	return distance;
}

//puts together the world position of the player's gaze point. each tick
FVector UPlayerTracker::GetGazePoint(FVector location, FRotator rotator)
{
	float distance = GetPixelDepth(0, maxDistance);
	FVector rotv = rotator.Vector();
	rotv *= distance;

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
		CognitiveLog::Info("PlayerTracker::SendGazeEventDataToSceneExplorer WORLD DOESNT EXIST");
		return;
	}

	FString currentSceneName = myworld->GetMapName();
	currentSceneName.RemoveFromStart(myworld->StreamingLevelsPrefix);
	UPlayerTracker::SendGazeEventDataToSceneExplorer(currentSceneName);
}



void UPlayerTracker::SendGazeEventDataToSceneExplorer(FString sceneName)
{
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

	wholeObj->SetStringField("hmdtype", DeviceNameString);

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


void UPlayerTracker::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	FString reason;
	bool shouldEndSession = true;
	switch (EndPlayReason)
	{
	case EEndPlayReason::Destroyed: reason = "destroyed";
		//this should never be destroyed, only when changing level. pro tips - doesn't actually use the 'level transition' reason for endplay
		shouldEndSession = false;
		break;
	case EEndPlayReason::EndPlayInEditor: reason = "end PIE";
		break;
	case EEndPlayReason::LevelTransition: reason = "level transition";
		shouldEndSession = false;
		break;
	case EEndPlayReason::Quit: reason = "quit";
		break;
	case EEndPlayReason::RemovedFromWorld: reason = "removed from world";
		break;
	default:
		reason = "default";
		break;
	}

	if (s.Get() != NULL)
	{
		if (shouldEndSession)
		{
			//this will send gaze and event data to scene explorer from THIS playertracker
			SendGazeEventDataToSceneExplorer();

			//cognitivevr manager can't find playercontroller0, but will send events to dash and dynamics+sensors to SE
			s->EndSession();
		}
	}
	Super::EndPlay(EndPlayReason);
}