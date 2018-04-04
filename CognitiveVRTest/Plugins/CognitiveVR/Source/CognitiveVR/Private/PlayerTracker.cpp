// Fill out your copyright notice in the Description page of Project Settings.

#include "CognitiveVR.h"
#include "PlayerTracker.h"
#include "CognitiveVRSettings.h"
#include "Util.h"


// Sets default values for this component's properties
UPlayerTracker::UPlayerTracker()
{
	PrimaryComponentTick.bCanEverTick = true;

	FString ValueReceived;

	//gaze batch size
	ValueReceived = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "GazeBatchSize", false);
	if (ValueReceived.Len() > 0)
	{
		int32 sensorLimit = FCString::Atoi(*ValueReceived);
		if (sensorLimit > 0)
		{
			GazeBatchSize = sensorLimit;
		}
	}

	ValueReceived = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "GazeFromVisualRaycast", false);
	if (ValueReceived.Len() > 0)
	{
		if (ValueReceived == "false")
			GazeFromVisualRaycast = false;
		else
			GazeFromVisualRaycast = true;
	}

	ValueReceived = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "GazeFromPhysicsRaycast", false);
	if (ValueReceived.Len() > 0)
	{
		if (ValueReceived == "false")
			GazeFromPhysicsRaycast = false;
		else
			GazeFromPhysicsRaycast = true;
	}
}

void UPlayerTracker::BeginPlay()
{
	FlushPersistentDebugLines(GetWorld());

	s = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider();
	if (s.IsValid())
	{
		FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider()->SetWorld(GetWorld());
		//s->SetWorld(GetWorld());

		Super::BeginPlay();
	}
	else
	{
		GLog->Log("UPlayerTracker::BeginPlay cannot find CognitiveVRProvider!");
	}
}

// Called every frame
void UPlayerTracker::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	if (s.Get() == NULL)
	{
		return;
	}

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	currentTime += DeltaTime;
	if (currentTime < PlayerSnapshotInterval)
	{
		return;
	}

	currentTime -= PlayerSnapshotInterval;

	FVector pos;
	FVector gaze;
	FRotator rot;
	double time = Util::GetTimestamp();
	FString objectid = "";


	TArray<APlayerController*, FDefaultAllocator> controllers;
	GEngine->GetAllLocalPlayerControllers(controllers);
	if (controllers.Num() == 0)
	{
		CognitiveLog::Info("UPlayerTracker::TickComponent--------------------------no controllers. skip");
		//return FVector();
		return;
	}

	//FVector finalLoc;

	controllers[0]->GetPlayerViewPoint(pos, rot);

	FTransform camManTransform = controllers[0]->PlayerCameraManager->GetActorTransform();
	//FVector camManForward = controllers[0]->PlayerCameraManager->GetActorForwardVector();

	FRotator temphmdrot;
	FVector temphmdpos;
	UHeadMountedDisplayFunctionLibrary::GetOrientationAndPosition(temphmdrot, temphmdpos);

	FVector finalPos = camManTransform.TransformPosition(temphmdrot.UnrotateVector(temphmdpos));
	captureLocation = finalPos - temphmdpos;
	pos = finalPos - temphmdpos;

	captureRotation = rot;

	//look at dynamic object

	FCollisionQueryParams Params; // You can use this to customize various properties about the trace
	Params.AddIgnoredActor(GetOwner()); // Ignore the player's pawn


	FHitResult Hit; // The hit result gets populated by the line trace

	FVector Start = captureLocation;
	FVector End = captureLocation + captureRotation.Vector() * 10000.0f;
	
	bool bHit = false;
	if (GazeFromVisualRaycast)
	{
		FCollisionQueryParams params = FCollisionQueryParams(FName(), true);
		bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECollisionChannel::ECC_Visibility, params);
	}
	else if (GazeFromPhysicsRaycast)
	{
		FCollisionObjectQueryParams params = FCollisionObjectQueryParams();
		params.AddObjectTypesToQuery(ECC_WorldStatic);
		params.AddObjectTypesToQuery(ECC_WorldDynamic);
		//FCollisionResponseParams otherParams = FCollisionResponseParams();

		bHit = GetWorld()->LineTraceSingleByObjectType(Hit, Start, End, params);
		//bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECollisionChannel::ECC_Pawn,params,otherParams);
	}

	bool hitDynamic = false;
	if (bHit)
	{
		gaze = Hit.ImpactPoint;

		if (Hit.Actor.IsValid())
		{
			UActorComponent* hitActorComponent = Hit.Actor.Get()->GetComponentByClass(UDynamicObject::StaticClass());
			if (hitActorComponent != NULL)
			{
				UDynamicObject* hitDynamicObject = Cast<UDynamicObject>(hitActorComponent);
				if (hitDynamicObject != NULL && hitDynamicObject->TrackGaze && hitDynamicObject->GetObjectId().IsValid())
				{
					hitDynamic = true;

					FVector localHitPosition = hitDynamicObject->GetOwner()->GetActorTransform().InverseTransformPosition(Hit.ImpactPoint);

					localHitPosition *= hitDynamicObject->GetOwner()->GetActorTransform().GetScale3D();

					objectid = hitDynamicObject->GetObjectId()->Id;
					gaze.X = localHitPosition.X;
					gaze.Y = localHitPosition.Y;
					gaze.Z = localHitPosition.Z;
				}
			}
			else
			{
				//hit an actor that is not a dynamic object
			}
		}
		else
		{
			//hit some csg or something that is not an actor
		}
		BuildSnapshot(captureLocation, gaze, captureRotation, time, objectid);
	}
	else
	{
		//hit nothing. use position and rotation only
		BuildSnapshot(captureLocation, captureRotation, time);
	}
}

void UPlayerTracker::BuildSnapshot(FVector position, FVector gaze, FRotator rotation, double time, FString objectId)
{
	TSharedPtr<FJsonObject>snapObj = MakeShareable(new FJsonObject);

	snapObj->SetNumberField("time", time);

	//positions
	TArray<TSharedPtr<FJsonValue>> posArray;
	TSharedPtr<FJsonValueNumber> JsonValue;
	JsonValue = MakeShareable(new FJsonValueNumber(-(int32)position.X)); //right
	posArray.Add(JsonValue);
	JsonValue = MakeShareable(new FJsonValueNumber((int32)position.Z)); //up
	posArray.Add(JsonValue);
	JsonValue = MakeShareable(new FJsonValueNumber((int32)position.Y));  //forward
	posArray.Add(JsonValue);

	snapObj->SetArrayField("p", posArray);

	if (objectId != "")
	{
		snapObj->SetStringField("o", objectId);
	}

	TArray<TSharedPtr<FJsonValue>> gazeArray;
	JsonValue = MakeShareable(new FJsonValueNumber(-(int32)gaze.X));
	gazeArray.Add(JsonValue);
	JsonValue = MakeShareable(new FJsonValueNumber((int32)gaze.Z));
	gazeArray.Add(JsonValue);
	JsonValue = MakeShareable(new FJsonValueNumber((int32)gaze.Y));
	gazeArray.Add(JsonValue);

	snapObj->SetArrayField("g", gazeArray);

	//rotation
	TArray<TSharedPtr<FJsonValue>> rotArray;

	FQuat quat;
	FRotator adjustedRot = rotation;
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
		SendData();
	}
}

void UPlayerTracker::BuildSnapshot(FVector position, FRotator rotation, double time)
{
	TSharedPtr<FJsonObject>snapObj = MakeShareable(new FJsonObject);

	snapObj->SetNumberField("time", time);

	//positions
	TArray<TSharedPtr<FJsonValue>> posArray;
	TSharedPtr<FJsonValueNumber> JsonValue;
	JsonValue = MakeShareable(new FJsonValueNumber(-(int32)position.X)); //right
	posArray.Add(JsonValue);
	JsonValue = MakeShareable(new FJsonValueNumber((int32)position.Z)); //up
	posArray.Add(JsonValue);
	JsonValue = MakeShareable(new FJsonValueNumber((int32)position.Y));  //forward
	posArray.Add(JsonValue);

	snapObj->SetArrayField("p", posArray);

	//rotation
	TArray<TSharedPtr<FJsonValue>> rotArray;

	FQuat quat;
	FRotator adjustedRot = rotation;
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
		SendData();
	}
}

void UPlayerTracker::SendData()
{
	auto scenedata = s->GetCurrentSceneData();
	if (s->GetCurrentSceneVersionNumber().Len() == 0) { return; }
	if (!scenedata.IsValid()){return;}

	//GAZE

	if (snapshots.Num() == 0) { return; }

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

	wholeObj->SetNumberField("interval", PlayerSnapshotInterval);

	wholeObj->SetArrayField("data", dataArray);

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(wholeObj.ToSharedRef(), Writer);

	if (OutputString.Len() > 0)
	{
		s->network->NetworkCall("gaze", OutputString);
	}
	snapshots.Empty();
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
			SendData();

			//TODO this why is endplay on gaze recorder not on core?
			//cognitivevr manager can't find playercontroller0, but will send events to dash and dynamics+sensors to SE
			s->EndSession();
		}
	}
	Super::EndPlay(EndPlayReason);
}