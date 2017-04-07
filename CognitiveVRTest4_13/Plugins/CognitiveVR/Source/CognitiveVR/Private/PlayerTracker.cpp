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
	InitializePlayerTracker();

	Super::BeginPlay();
	Http = &FHttpModule::Get();

	UTextureRenderTarget2D* renderTarget;
	renderTarget = NewObject<UTextureRenderTarget2D>();
	renderTarget->AddToRoot();
	renderTarget->ClearColor = FLinearColor::White;
	renderTarget->bHDR = false;
	renderTarget->InitAutoFormat(256, 256); //auto init from value bHDR

	sceneCapture = NewObject<USceneCaptureComponent2D>(this);
	sceneCapture->SetupAttachment(this);
	sceneCapture->SetRelativeLocation(FVector::ZeroVector);
	sceneCapture->SetRelativeRotation(FQuat::Identity);
	sceneCapture->TextureTarget = renderTarget;
	sceneCapture->RegisterComponent();
	sceneCapture->CaptureSource = SCS_FinalColorLDR;

	sceneCapture->PostProcessSettings.AddBlendable(SceneDepthMat, 1);
}

void UPlayerTracker::AddJsonEvent(FJsonObject* newEvent)
{
	TSharedPtr<FJsonObject>snapObj = MakeShareable(newEvent);
	events.Add(snapObj);
}

// Called every frame
void UPlayerTracker::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	if (s.Get() == NULL)
	{
		return;
	}

	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );
	
	currentTime += DeltaTime;
	if (currentTime > PlayerSnapshotInterval)
	{
		currentTime -= PlayerSnapshotInterval;
		//write to json

		TSharedPtr<FJsonObject>snapObj = MakeShareable(new FJsonObject);

		double ts = Util::GetTimestamp();

		//time
		snapObj->SetNumberField("time", ts);

		//positions
		TArray<TSharedPtr<FJsonValue>> posArray;
		TSharedPtr<FJsonValueNumber> JsonValue;
		JsonValue = MakeShareable(new FJsonValueNumber(-(int32)GetComponentLocation().X)); //right
		posArray.Add(JsonValue);
		JsonValue = MakeShareable(new FJsonValueNumber((int32)GetComponentLocation().Z)); //up
		posArray.Add(JsonValue);
		JsonValue = MakeShareable(new FJsonValueNumber((int32)GetComponentLocation().Y));  //forward
		posArray.Add(JsonValue);

		snapObj->SetArrayField("p", posArray);

		//gaze
		TArray<TSharedPtr<FJsonValue>> gazeArray;
		FVector gazePoint = GetGazePoint();
		JsonValue = MakeShareable(new FJsonValueNumber(-(int32)gazePoint.X));
		gazeArray.Add(JsonValue);
		JsonValue = MakeShareable(new FJsonValueNumber((int32)gazePoint.Z));
		gazeArray.Add(JsonValue);
		JsonValue = MakeShareable(new FJsonValueNumber((int32)gazePoint.Y));
		gazeArray.Add(JsonValue);

		snapObj->SetArrayField("g", gazeArray);


		//rotation
		TArray<TSharedPtr<FJsonValue>> rotArray;

		FQuat quat = GetComponentQuat();
		FRotator rot = GetComponentToWorld().Rotator();
		rot.Yaw -= 90;
		quat = rot.Quaternion();

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
FVector UPlayerTracker::GetGazePoint()
{
	float distance = GetPixelDepth(0, maxDistance);
	FRotator rot = GetComponentRotation();
	FVector rotv = rot.Vector();
	rotv *= distance;

	//add location
	FVector loc = GetComponentLocation();
	
	FVector returnVector;
	returnVector.X = loc.X + rotv.X;
	returnVector.Y = loc.Y + rotv.Y;
	returnVector.Z = loc.Z + rotv.Z;
	return returnVector;
}

void UPlayerTracker::SendGazeEventDataToSceneExplorer()
{
	UWorld* myworld = GetWorld();
	if (myworld == NULL) { return; }

	FString currentSceneName = myworld->GetMapName();
	currentSceneName.RemoveFromStart(myworld->StreamingLevelsPrefix);
	UPlayerTracker::SendGazeEventDataToSceneExplorer(currentSceneName);
}

FString UPlayerTracker::GetSceneKey(FString sceneName)
{
	
	//GConfig->GetArray()
	FConfigSection* Section = GConfig->GetSectionPrivate(TEXT("/Script/CognitiveVR.CognitiveVRSettings"), false, true, GEngineIni);
	if (Section == NULL)
	{
		return "";
	}
	for (FConfigSection::TIterator It(*Section); It; ++It)
	{
		if (It.Key() == TEXT("SceneData"))
		{
			FString name;
			FString key;
			It.Value().GetValue().Split(TEXT(","), &name, &key);
			if (*name == sceneName)
			{
				GLog->Log("-----> UPlayerTracker::GetSceneKey found key for scene " + name);
				return key;
			}
			else
			{
				GLog->Log("UPlayerTracker::GetSceneKey found key for scene " + name);
			}
		}
	}

	//no matches anywhere
	CognitiveLog::Warning("UPlayerTracker::GetSceneKey ------- no matches in ini");
	return "";
}

void UPlayerTracker::SendJson(FString endpoint, FString json)
{
	if (!FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Get()->HasStartedSession())
	{
		CognitiveLog::Warning("UPlayerTRacker::SendJson CognitiveVRProvider has not started a session!");
		return;
	}
	//TODO should not send data if cognitivevrprovider has not initialized itself

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

	FString currentSceneName = myworld->GetMapName();
	currentSceneName.RemoveFromStart(myworld->StreamingLevelsPrefix);

	FString sceneKey = up->GetSceneKey(currentSceneName);
	if (sceneKey == "")
	{
		CognitiveLog::Warning("UPlayerTracker::SendJson does not have scenekey. fail!");
		return;
	}

	std::string stdjson(TCHAR_TO_UTF8(*json));
	std::string ep(TCHAR_TO_UTF8(*endpoint));
	CognitiveLog::Info("PlayerTracker::SendJson sending json to" + ep +": " + stdjson);

	FString url = "https://sceneexplorer.com/api/";


	//json to scene endpoint

	TSharedRef<IHttpRequest> RequestGaze = up->Http->CreateRequest();
	RequestGaze->SetContentAsString(json);
	RequestGaze->SetURL(url + endpoint +"/" + sceneKey);
	RequestGaze->SetVerb("POST");
	RequestGaze->SetHeader("Content-Type", TEXT("application/json"));
	RequestGaze->ProcessRequest();
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
	SendJson("gaze", GazeString);
	
	//EVENTS

	FString EventString = UPlayerTracker::EventSnapshotsToString();
	SendJson("events", EventString);
}

FString UPlayerTracker::EventSnapshotsToString()
{
	TSharedPtr<FJsonObject>wholeObj = MakeShareable(new FJsonObject);
	TArray<TSharedPtr<FJsonValue>> dataArray;
	FAnalyticsProviderCognitiveVR* cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Get();

	wholeObj->SetStringField("userid", cog->GetDeviceID());
	wholeObj->SetNumberField("timestamp", cog->GetSessionTimestamp());
	wholeObj->SetStringField("sessionid", cog->GetSessionID());
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
	wholeObj->SetNumberField("timestamp", cog->GetSessionTimestamp());
	wholeObj->SetStringField("sessionid", cog->GetSessionID());
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

/*
void UPlayerTracker::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	
	std::string reason;
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