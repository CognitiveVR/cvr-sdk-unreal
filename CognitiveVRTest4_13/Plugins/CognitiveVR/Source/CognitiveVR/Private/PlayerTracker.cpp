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
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );
	
	currentTime += DeltaTime;
	if (currentTime > PlayerSnapshotInterval)
	{
		currentTime -= PlayerSnapshotInterval;
		//write to json

		TSharedPtr<FJsonObject>snapObj = MakeShareable(new FJsonObject);

		std::string ts = Util::GetTimestampStr();
		FString fs(ts.c_str());
		double time = FCString::Atod(*fs);

		//time
		snapObj->SetNumberField("time", time);
		//UGameplayStatics::GetRealTimeSeconds(GetWorld()); //time since level start

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

		snapshots.Add(snapObj);
		if (snapshots.Num() > MaxSnapshots)
		{
			SendData();
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

void UPlayerTracker::SendData()
{
	UWorld* myworld = GetWorld();
	if (myworld == NULL) { return; }

	FString currentSceneName = myworld->GetMapName();
	currentSceneName.RemoveFromStart(myworld->StreamingLevelsPrefix);
	UPlayerTracker::SendData(currentSceneName);
}

FString UPlayerTracker::GetSceneKey(FString sceneName)
{
	FConfigSection* ScenePairs = GConfig->GetSectionPrivate(TEXT("/Script/CognitiveVR.CognitiveVRSettings"), false, true, GEngineIni);
	for (FConfigSection::TIterator It(*ScenePairs); It; ++It)
	{
		if (It.Key() == TEXT("SceneKeyPair"))
		{
			FName SceneName = NAME_None;
			FName SceneKey;

			if (FParse::Value(*It.Value().GetValue(), TEXT("SceneName="), SceneName))
			{
				if (FParse::Value(*It.Value().GetValue(), TEXT("SceneKey="), SceneKey))
				{
					if (!SceneKey.IsValid() || SceneKey == NAME_None)
					{
						CognitiveLog::Warning("UPlayerTracker::GetSceneKey - key is invalid or none");
						//something wrong happened
					}
					else
					{
						if (FName(*sceneName) == SceneName)
						{
							//found match
							return SceneKey.ToString();
						}
						else
						{
							//not a match
						}
					}
				}
			}
		}
	}

	//no matches anywhere
	CognitiveLog::Warning("UPlayerTracker::GetSceneKey ------- no matches in ini");
	return "";
}

void UPlayerTracker::SendData(FString sceneName)
{
	CognitiveLog::Info("UPlayerTracker::SendData");
	FString sceneKey = UPlayerTracker::GetSceneKey(sceneName);

	FString url = "https://sceneexplorer.com/api/";


	//GAZE

	TSharedRef<IHttpRequest> RequestGaze = Http->CreateRequest();
	FString GazeString = UPlayerTracker::GazeSnapshotsToString();
	RequestGaze->SetContentAsString(GazeString);
	RequestGaze->SetURL(url + "gaze/" + sceneKey);
	RequestGaze->SetVerb("POST");
	RequestGaze->SetHeader("Content-Type", TEXT("application/json"));
	RequestGaze->ProcessRequest();
	
	//EVENTS

	TSharedRef<IHttpRequest> RequestEvents = Http->CreateRequest();
	FString EventString = UPlayerTracker::EventSnapshotsToString();
	RequestEvents->SetContentAsString(EventString);
	RequestEvents->SetURL(url+"events/"+sceneKey);
	RequestEvents->SetVerb("POST");
	RequestEvents->SetHeader("Content-Type", TEXT("application/json"));
	RequestEvents->ProcessRequest();

}

FString UPlayerTracker::EventSnapshotsToString()
{
	TSharedPtr<FJsonObject>wholeObj = MakeShareable(new FJsonObject);
	TArray<TSharedPtr<FJsonValue>> dataArray;
	FAnalyticsProviderCognitiveVR* cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Get();

	wholeObj->SetStringField("userid", cog->GetUserID());

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

	wholeObj->SetStringField("userid", cog->GetUserID());

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

void UPlayerTracker::RequestSendData()
{
	TArray<APlayerController*, FDefaultAllocator> controllers;
	GEngine->GetAllLocalPlayerControllers(controllers);
	if (controllers.Num() == 0)
	{
		return;
	}
	UPlayerTracker* up = controllers[0]->GetPawn()->FindComponentByClass<UPlayerTracker>();
	if (up == NULL)
	{
		return;
	}
	up->SendData();
}

