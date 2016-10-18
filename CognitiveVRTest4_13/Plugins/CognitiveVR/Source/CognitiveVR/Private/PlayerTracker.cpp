// Fill out your copyright notice in the Description page of Project Settings.

#include "CognitiveVR.h"
#include "PlayerTracker.h"


// Sets default values for this component's properties
UPlayerTracker::UPlayerTracker()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	bWantsBeginPlay = true;
	PrimaryComponentTick.bCanEverTick = true;
	//

	// ...
}


// Called when the game starts
void UPlayerTracker::BeginPlay()
{
	Super::BeginPlay();
	Http = &FHttpModule::Get();

	USceneCaptureComponent2D* scc;
	scc = this->GetAttachmentRootActor()->FindComponentByClass<USceneCaptureComponent2D>();
	if (scc == NULL)
	{

		//create a scenecaptureactor (should really just inherit from this)
		//create a rendertarget
		//renderTarget = NewObject<UTextureRenderTarget2D>();

		renderTarget = NewObject<UTextureRenderTarget2D>();
		//renderTarget->bHDR = bHDR;
		renderTarget->ClearColor = FLinearColor::White;
		//renderTarget->TargetGamma = Gamma;
		renderTarget->InitAutoFormat(256, 256); //auto init from value bHDR

		//FName sccName("SceneCaptureComponent");
		//USceneCaptureComponent2D* scc = ConstructObject<USceneCaptureComponent2D>(CompClass, this, sccName);
	
		
	


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


// Called every frame
void UPlayerTracker::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );
	//return;
	
	currentTime += DeltaTime;
	if (currentTime > 1)
	{
		
		UE_LOG(LogTemp, Warning, TEXT("Player Tracker Tick"));
		currentTime -= 1;
		//write to json

		TSharedPtr<FJsonObject>snapObj = MakeShareable(new FJsonObject);

		//time
		snapObj->SetNumberField("time", 5);

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
		ticksTillSend--;
		if (ticksTillSend <= 0)
		{
			SendData(GetWorld()->GetMapName());
			ticksTillSend = 10;
			//should i clear the snapshots? meh, it's just a test
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

	//return -1;

	TArray<FColor> SurfData;
	//rendertarget->GameThread_GetRenderTargetResource();	

	FRenderTarget *RenderTarget = renderTarget->GameThread_GetRenderTargetResource();


	RenderTarget->ReadPixels(SurfData);

	// Index formula

	FIntPoint size = RenderTarget->GetSizeXY();

	//int x = Texture->GetSizeX();

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


void UPlayerTracker::SendData(FString sceneName)
{
	//SceneKeys[sceneName]
	if (SceneKeys.Contains(sceneName))
	{
		//load this from the ini file?
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("scene keys doesn't contain key for scene "));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("SEND"));
	//{object}
	//[array]
	/*
	{"userid":"somevalue",
	"data":[{"time":123.45,"p":[12.31,45.61,78.91],"g":[12.31,45.61,78.91]}]}
	*/

	
	
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

	//ini or settings?
	//FString sceneKey = "b80e3e3b-60ed-4586-b865-7420ea76543c";
	FString sceneKey = "5aab820a-d3df-4ec4-98cb-b8c80fc73722";
	FString url = "https://sceneexplorer.com/api/gaze/" + sceneKey;

	UE_LOG(LogTemp, Warning, TEXT("URL is %s"), *url);
	UE_LOG(LogTemp, Warning, TEXT("content is %s"), *OutputString);

	if (!sendToServer) { return; }

	TSharedRef<IHttpRequest> Request = Http->CreateRequest();
	//Request->OnProcessRequestComplete().BindUObject(this, &AHttpActor::OnResponseReceived);
	Request->SetContentAsString(OutputString);
	Request->SetURL(url);
	Request->SetVerb("POST");
	Request->SetHeader("Content-Type", TEXT("application/json"));
	Request->ProcessRequest();
	
}

