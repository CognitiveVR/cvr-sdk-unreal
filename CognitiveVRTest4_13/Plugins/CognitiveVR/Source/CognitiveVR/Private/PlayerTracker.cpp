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
	UE_LOG(LogTemp, Warning, TEXT("player tracker constructor"));
}

void UPlayerTracker::BeginPlay()
{
	UE_LOG(LogTemp, Warning, TEXT("player tracker begin play"));
	Super::BeginPlay();
	Http = &FHttpModule::Get();

	//RegisterComponent();

	USceneCaptureComponent2D* scc;
	scc = this->GetAttachmentRootActor()->FindComponentByClass<USceneCaptureComponent2D>();
	if (scc == NULL)
	{
		renderTarget = NewObject<UTextureRenderTarget2D>();
		renderTarget->ClearColor = FLinearColor::White;
		renderTarget->InitAutoFormat(256, 256); //auto init from value bHDR
	
		UE_LOG(LogTemp, Warning, TEXT("=====create new scene capture component"));
		scc = NewObject<USceneCaptureComponent2D>();;
		
		//scc->SetupAttachment()
		scc->SetupAttachment(this);
		scc->SetRelativeLocation(FVector::ZeroVector);
		scc->SetRelativeRotation(FQuat::Identity);
		scc->TextureTarget = renderTarget;

		scc->CaptureSource = SCS_FinalColorLDR;
		scc->AddOrUpdateBlendable(SceneDepthMat); //TODO load this from project
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("found scene capture component"));
		renderTarget = scc->TextureTarget;
	}
}

void UPlayerTracker::AddJsonEvent(FJsonObject* newEvent)
{
	TSharedPtr<FJsonObject>snapObj = MakeShareable(newEvent);

	UE_LOG(LogTemp, Warning, TEXT("add json event"));

	events.Add(snapObj);
}

// Called every frame
void UPlayerTracker::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );
	
	currentTime += DeltaTime;
	if (currentTime > PlayerSnapshotInterval)
	{
		
		UE_LOG(LogTemp, Warning, TEXT("Player Tracker Tick"));
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
		JsonValue = MakeShareable(new FJsonValueNumber(-GetComponentLocation().X)); //right
		posArray.Add(JsonValue);
		JsonValue = MakeShareable(new FJsonValueNumber(GetComponentLocation().Z)); //up
		posArray.Add(JsonValue);
		JsonValue = MakeShareable(new FJsonValueNumber(GetComponentLocation().Y));  //forward
		posArray.Add(JsonValue);

		snapObj->SetArrayField("p", posArray);

		//gaze
		TArray<TSharedPtr<FJsonValue>> gazeArray;
		FVector gazePoint = GetGazePoint();
		JsonValue = MakeShareable(new FJsonValueNumber(-gazePoint.X));
		gazeArray.Add(JsonValue);
		JsonValue = MakeShareable(new FJsonValueNumber(gazePoint.Z));
		gazeArray.Add(JsonValue);
		JsonValue = MakeShareable(new FJsonValueNumber(gazePoint.Y));
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
	if (renderTarget == NULL)
	{
		if (renderTarget->SizeX <= 0)
			UE_LOG(LogTemp, Warning, TEXT("render target size is 0"));
		UE_LOG(LogTemp, Warning, TEXT("RENDER TARGET IS NULL"));

		return -1;
	}

	// Creates Texture2D to store TextureRenderTarget content
	UTexture2D *Texture = UTexture2D::CreateTransient(renderTarget->SizeX, renderTarget->SizeY, PF_B8G8R8A8);

	if (Texture == NULL)
	{
		UE_LOG(LogTemp, Warning, TEXT("TEMP Texture IS NULL"));

		return -1;

	}

#if WITH_EDITORONLY_DATA
	Texture->MipGenSettings = TMGS_NoMipmaps;
#endif
	Texture->SRGB = renderTarget->SRGB;

	TArray<FColor> SurfData;

	FRenderTarget *RenderTarget = renderTarget->GameThread_GetRenderTargetResource();


	RenderTarget->ReadPixels(SurfData);

	// Index formula

	FIntPoint size = RenderTarget->GetSizeXY();

	FColor PixelColor = SurfData[size.X / 2 + size.Y / 2 * renderTarget->SizeX];

	float nf = PixelColor.R / 255.0;

	float distance = FMath::Lerp(minvalue, maxvalue, nf);

	return distance;
}

FVector UPlayerTracker::GetGazePoint()
{
	float distance = GetPixelDepth(0, 8192);
	FRotator rot = GetComponentRotation();
	FVector rotv = rot.Vector();
	rotv *= distance;

	//add location
	FVector loc = GetComponentLocation();
	
	FVector returnVector;
	returnVector.X = loc.X + rotv.Y;
	returnVector.Y = loc.Y + rotv.Y;
	returnVector.Z = loc.Z + rotv.Z;
	return returnVector;
}

void UPlayerTracker::SendData()
{
	UE_LOG(LogTemp, Warning, TEXT("SEND"));
	UWorld* myworld = GetWorld();
	
	if (myworld == NULL) { UE_LOG(LogTemp, Warning, TEXT("NO WORLD")); return; }
	
	UE_LOG(LogTemp, Warning, TEXT("YES WORLD"));

	//return;

	FString currentSceneName = myworld->GetMapName();

	currentSceneName.RemoveFromStart(myworld->StreamingLevelsPrefix);

	//
	UE_LOG(LogTemp, Warning, TEXT("PlayerTracker::SendData scene name %s"),*currentSceneName);	
	
	UPlayerTracker::SendData(currentSceneName);
}

FString UPlayerTracker::GetSceneKey(FString sceneName)
{
	TArray<TSharedPtr<FSceneKeyPair>> ListSceneKeys;

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
						UE_LOG(LogTemp, Warning, TEXT("player tracker::get scene key - something wrong happened"));
						//something wrong happened
					}
					else
					{
						if (FName(*sceneName) == SceneName)
						{
							//found match
							UE_LOG(LogTemp, Warning, TEXT("player tracker::get scene key - FOUND A MATCH!!!!!!"));
							UE_LOG(LogTemp, Warning, TEXT("PlayerTracker::scene key %s"), *SceneKey.ToString());
							return SceneKey.ToString();
						}
						else
						{
							UE_LOG(LogTemp, Warning, TEXT("player tracker::get scene key - not a match"));
							//not a match
						}
					}
				}
			}
		}
	}

	//no matches anywhere
	UE_LOG(LogTemp, Warning, TEXT("player tracker::get scene key ------- no matches in ini"));
	return "";
}

void UPlayerTracker::SendData(FString sceneName)
{
	UE_LOG(LogTemp, Warning, TEXT("SEND"));

	// = "5aab820a-d3df-4ec4-98cb-b8c80fc73722";
	FString sceneKey = UPlayerTracker::GetSceneKey(sceneName);

	UE_LOG(LogTemp, Warning, TEXT("PlayerTracker::SendData scene KEY %s"), *sceneKey);

	FString url = "https://sceneexplorer.com/api/";

	if (!sendToServer) { return; }


	//GAZE

	TSharedRef<IHttpRequest> RequestGaze = Http->CreateRequest();
	FString GazeString = UPlayerTracker::GazeSnapshotsToString();
	RequestGaze->SetContentAsString(GazeString);
	UE_LOG(LogTemp, Warning, TEXT("URL is %s"), *url);
	UE_LOG(LogTemp, Warning, TEXT("content is %s"), *GazeString);
	RequestGaze->SetURL(url + "gaze/" + sceneKey);
	RequestGaze->SetVerb("POST");
	RequestGaze->SetHeader("Content-Type", TEXT("application/json"));
	RequestGaze->ProcessRequest();
	
	//EVENTS

	TSharedRef<IHttpRequest> RequestEvents = Http->CreateRequest();
	FString EventString = UPlayerTracker::EventSnapshotsToString();
	RequestEvents->SetContentAsString(EventString);
	UE_LOG(LogTemp, Warning, TEXT("URL is %s"), *url);
	UE_LOG(LogTemp, Warning, TEXT("content is %s"), *EventString);
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
		dataArray.Add(MakeShareable(new FJsonValueObject(snapshots[i])));
	}

	wholeObj->SetArrayField("data", dataArray);

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(wholeObj.ToSharedRef(), Writer);
	return OutputString;
}

/*
{"userid":"somevalue",
"data":[{"time":123.45,"p":[12.31,45.61,78.91],"g":[12.31,45.61,78.91]}]}
*/
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

