// Fill out your copyright notice in the Description page of Project Settings.

#include "CognitiveVR.h"
#include "DynamicObject.h"
#include "CognitiveVRSettings.h"
#include "Util.h"


// Sets default values for this component's properties
UDynamicObject::UDynamicObject()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	//bWantsBeginPlay = true;
	PrimaryComponentTick.bCanEverTick = true;
}


void UDynamicObject::BeginPlay()
{
	s = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider();

	Super::BeginPlay();
}

// Called every frame
void UDynamicObject::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );
	
	currentTime += DeltaTime;
	if (currentTime > SnapshotInterval)
	{
		currentTime -= SnapshotInterval;
		//write to json

		TSharedPtr<FJsonObject>snapObj = MakeShareable(new FJsonObject);

		double ts = Util::GetTimestamp();

		//time
		snapObj->SetNumberField("time", ts);

		//positions
		TArray<TSharedPtr<FJsonValue>> posArray;
		TSharedPtr<FJsonValueNumber> JsonValue;
		
		JsonValue = MakeShareable(new FJsonValueNumber(-(int32)GetOwner()->GetActorLocation().X)); //right
		posArray.Add(JsonValue);
		JsonValue = MakeShareable(new FJsonValueNumber((int32)GetOwner()->GetActorLocation().Z)); //up
		posArray.Add(JsonValue);
		JsonValue = MakeShareable(new FJsonValueNumber((int32)GetOwner()->GetActorLocation().Y));  //forward
		posArray.Add(JsonValue);

		snapObj->SetArrayField("p", posArray);

		//rotation
		TArray<TSharedPtr<FJsonValue>> rotArray;

		FQuat quat = GetOwner()->GetActorQuat();
		FRotator rot = GetOwner()->GetActorTransform().Rotator();
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
		if (snapshots.Num() > MaxSnapshots)
		{
			SendData();
			snapshots.Empty();
		}
	}
}


void UDynamicObject::SendData()
{
	UWorld* myworld = GetWorld();
	if (myworld == NULL) { return; }

	FString currentSceneName = myworld->GetMapName();
	currentSceneName.RemoveFromStart(myworld->StreamingLevelsPrefix);
	UDynamicObject::SendData(currentSceneName);
}

void UDynamicObject::SendData(FString sceneName)
{
	CognitiveLog::Info("UDynamicObject::SendData");

	FString EventString = UDynamicObject::DynamicSnapshotsToString();
	UPlayerTracker::SendJson("dynamics", EventString);
}

FString UDynamicObject::DynamicSnapshotsToString()
{
	TSharedPtr<FJsonObject>wholeObj = MakeShareable(new FJsonObject);
	TArray<TSharedPtr<FJsonValue>> dataArray;
	FAnalyticsProviderCognitiveVR* cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Get();

	wholeObj->SetStringField("userid", cog->GetDeviceID());
	wholeObj->SetNumberField("timestamp", cog->GetSessionTimestamp());
	wholeObj->SetStringField("sessionId", cog->GetSessionID());
	wholeObj->SetNumberField("part", jsonPart);
	jsonPart++;

	for (int32 i = 0; i != snapshots.Num(); ++i)
	{
		dataArray.Add(MakeShareable(new FJsonValueObject(snapshots[i])));
	}

	wholeObj->SetArrayField("data", dataArray);

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(wholeObj.ToSharedRef(), Writer);
	return OutputString;
}