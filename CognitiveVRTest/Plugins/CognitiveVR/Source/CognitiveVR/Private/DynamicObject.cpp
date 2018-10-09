// Fill out your copyright notice in the Description page of Project Settings.

#include "CognitiveVR.h"
#include "DynamicObject.h"
#include "CognitiveVRSettings.h"
#include "Util.h"

TArray<FDynamicObjectSnapshot> snapshots;
TArray<cognitivevrapi::FDynamicObjectManifestEntry> manifest;
TArray<cognitivevrapi::FDynamicObjectManifestEntry> newManifest;
TArray<TSharedPtr<cognitivevrapi::FDynamicObjectId>> allObjectIds;
int32 jsonPart = 1;
int32 MaxSnapshots = -1;

int32 MinTimer = 5;
int32 AutoTimer = 10;
int32 ExtremeBatchSize = 128;
float NextSendTime = 0;
float LastSendTime = -60;
FTimerHandle CognitiveDynamicAutoSendHandle;

// Sets default values for this component's properties
UDynamicObject::UDynamicObject()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	//bWantsBeginPlay = true;
	PrimaryComponentTick.bCanEverTick = true;
}

void UDynamicObject::TryGenerateMeshName()
{
	if (MeshName.IsEmpty())
	{
		UActorComponent* actorComponent = GetOwner()->GetComponentByClass(UStaticMeshComponent::StaticClass());
		if (actorComponent == NULL)
		{
			return;
		}
		UStaticMeshComponent* staticmeshComponent = Cast<UStaticMeshComponent>(actorComponent);
		if (staticmeshComponent == NULL || staticmeshComponent->GetStaticMesh() == NULL)
		{
			return;
		}
		UseCustomMeshName = true;
		MeshName = staticmeshComponent->GetStaticMesh()->GetName();
	}
}

void UDynamicObject::GenerateCustomId()
{
	UseCustomId = true;
	CustomId = FGuid::NewGuid().ToString();
}

void UDynamicObject::TryGenerateCustomIdAndMesh()
{
	if (MeshName.IsEmpty())
	{
		if (GetOwner() == NULL)
		{
			return;
		}

		UActorComponent* actorComponent = GetOwner()->GetComponentByClass(UStaticMeshComponent::StaticClass());
		if (actorComponent == NULL)
		{
			return;
		}
		UStaticMeshComponent* staticmeshComponent = Cast<UStaticMeshComponent>(actorComponent);
		if (staticmeshComponent == NULL || staticmeshComponent->GetStaticMesh() == NULL)
		{
			return;
		}
		UseCustomMeshName = true;
		UseCustomId = true;
		MeshName = staticmeshComponent->GetStaticMesh()->GetName();
		CustomId = FGuid::NewGuid().ToString();
		GWorld->MarkPackageDirty();
	}
}

void UDynamicObject::BeginPlay()
{
	Super::BeginPlay();

	s = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider();
	if (!s.IsValid())
	{
		GLog->Log("UDynamicObject::BeginPlay cannot find CognitiveVRProvider!");
		return;
	}

	if (MaxSnapshots < 0)
	{
		MaxSnapshots = 16;
		FString ValueReceived;

		ValueReceived = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "DynamicDataLimit", false);
		if (ValueReceived.Len() > 0)
		{
			int32 dynamicLimit = FCString::Atoi(*ValueReceived);
			if (dynamicLimit > 0)
			{
				MaxSnapshots = dynamicLimit;
			}
		}

		ValueReceived = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "DynamicExtremeLimit", false);
		if (ValueReceived.Len() > 0)
		{
			int32 parsedValue = FCString::Atoi(*ValueReceived);
			if (parsedValue > 0)
			{
				ExtremeBatchSize = parsedValue;
			}
		}

		ValueReceived = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "DynamicMinTimer", false);
		if (ValueReceived.Len() > 0)
		{
			int32 parsedValue = FCString::Atoi(*ValueReceived);
			if (parsedValue > 0)
			{
				MinTimer = parsedValue;
			}
		}

		ValueReceived = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/CognitiveVR.CognitiveVRSettings", "DynamicAutoTimer", false);
		if (ValueReceived.Len() > 0)
		{
			int32 parsedValue = FCString::Atoi(*ValueReceived);
			if (parsedValue > 0)
			{
				AutoTimer = parsedValue;
				s->GetWorld()->GetGameInstance()->GetTimerManager().SetTimer(CognitiveDynamicAutoSendHandle, FTimerDelegate::CreateStatic(&UDynamicObject::SendData), AutoTimer, false);
			}
		}
	}

	//even if session has not started, still collect data
	//session must be started to send
	//scene id must be valid to send

	//actor component
	//LastPosition = GetOwner()->GetActorLocation();
	//LastForward = GetOwner()->GetActorForwardVector();

	//scene component
	LastPosition = GetComponentLocation();
	LastForward = GetComponentTransform().TransformVector(FVector::ForwardVector);

	if (!UseCustomMeshName)
	{
		UseCustomMeshName = true;
		if (CommonMeshName == ECommonMeshName::OculusTouchLeft)
		{
			MeshName = "oculustouchleft";
		}
		else if (CommonMeshName == ECommonMeshName::OculusTouchRight)
		{
			MeshName = "oculustouchright";
		}
		else if (CommonMeshName == ECommonMeshName::ViveController)
		{
			MeshName = "vivecontroller";
		}
		else if (CommonMeshName == ECommonMeshName::ViveTracker)
		{
			MeshName = "vivetracker";
		}
	}

	if (SnapshotOnBeginPlay)
	{
		FDynamicObjectSnapshot initSnapshot = MakeSnapshot();
		SnapshotBoolProperty(initSnapshot, "enable", true);
		if (initSnapshot.time > 1)
		{
			snapshots.Add(initSnapshot);
		}

		if (snapshots.Num() + newManifest.Num() > MaxSnapshots)
		{
			TrySendData();
		}
	}
}

/*FDynamicObjectSnapshot* FDynamicObjectSnapshot::SnapshotProperty(FString key, FString value)
{
	this->StringProperties.Add(key, value);	
	return this;
}
FDynamicObjectSnapshot* FDynamicObjectSnapshot::SnapshotProperty(FString key, bool value)
{
	this->BoolProperties.Add(key, value);
	return this;
}
FDynamicObjectSnapshot* FDynamicObjectSnapshot::SnapshotProperty(FString key, int32 value)
{
	this->IntegerProperties.Add(key, value);
	return this;
}
FDynamicObjectSnapshot* FDynamicObjectSnapshot::SnapshotProperty(FString key, double value)
{
	this->DoubleProperties.Add(key, value);
	return this;
}*/

TSharedPtr<cognitivevrapi::FDynamicObjectId> UDynamicObject::GetUniqueId(FString meshName)
{
	TSharedPtr<cognitivevrapi::FDynamicObjectId> freeId;
	static int32 originalId = 1000;
	originalId++;

	freeId = MakeShareable(new cognitivevrapi::FDynamicObjectId(FString::FromInt(originalId), meshName));
	return freeId;
}

TSharedPtr<cognitivevrapi::FDynamicObjectId> UDynamicObject::GetObjectId()
{
	if (!ObjectID.IsValid() || ObjectID->Id == "")
	{
		GenerateObjectId();
	}
	return ObjectID;
}

void UDynamicObject::GenerateObjectId()
{
	if (!UseCustomId)
	{
		TSharedPtr<cognitivevrapi::FDynamicObjectId> recycledId;
		bool foundRecycleId = false;

		for (int32 i = 0; i < allObjectIds.Num(); i++)
		{
			if (!allObjectIds[i]->Used && allObjectIds[i]->MeshName == MeshName)
			{
				foundRecycleId = true;
				recycledId = allObjectIds[i];
				break;
			}
		}
		if (foundRecycleId)
		{
			ObjectID = recycledId;
			ObjectID->Used = true;
			cognitivevrapi::CognitiveLog::Info("UDynamicObject::Recycle ObjectID! " + MeshName);
		}
		else
		{
			cognitivevrapi::CognitiveLog::Info("UDynamicObject::Get new ObjectID! " + MeshName);
			ObjectID = GetUniqueId(MeshName);

			allObjectIds.Add(ObjectID);
		}

		cognitivevrapi::FDynamicObjectManifestEntry entry = cognitivevrapi::FDynamicObjectManifestEntry(ObjectID->Id, GetOwner()->GetName(), MeshName);
		if (!GroupName.IsEmpty())
		{
			entry.SetProperty("groupname", GroupName);
		}
		manifest.Add(entry);
		newManifest.Add(entry);
	}
	else
	{
		ObjectID = MakeShareable(new cognitivevrapi::FDynamicObjectId(CustomId, MeshName));
		cognitivevrapi::FDynamicObjectManifestEntry entry = cognitivevrapi::FDynamicObjectManifestEntry(ObjectID->Id, GetOwner()->GetName(), MeshName);
		if (!GroupName.IsEmpty())
		{
			//entry.
			entry.SetProperty("groupname", GroupName);
		}
		manifest.Add(entry);
		newManifest.Add(entry);
	}

	//if (manifest.Num() == 1)
	//{
		//cognitivevrapi::CognitiveLog::Info("DynamicObject::MakeSnapshot Register Provider->OnSendData for Dynamics");
		//s->OnSendData.AddStatic(SendData);
		//TODO register dynamic object send data to some event, or just call manually from core
	//}
}

void UDynamicObject::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );

	if (!s->HasStartedSession()) { return; }
	if (!SnapshotOnInterval) { return; }

	currentTime += DeltaTime;
	if (currentTime > SnapshotInterval)
	{
		currentTime -= SnapshotInterval;

		FVector currentForward = GetComponentTransform().TransformVector(FVector::ForwardVector);// GetOwner()->GetActorForwardVector();

		currentForward.Normalize();
		
		float dotRot = FVector::DotProduct(LastForward, currentForward);

		float actualDegrees = FMath::Acos(FMath::Clamp<float>(dotRot, -1.0, 1.0)) * 57.29578;

		if ((LastPosition - GetComponentLocation()).Size() > PositionThreshold)
		{
			//moved
		}
		else if (actualDegrees > RotationThreshold) //rotator stuff
		{
			//rotated
		}
		else if (DirtyEngagements.Num() > 0)
		{
			//dirty engagements are written an inactive ones are removed in MakeSnapshot()
		}
		else
		{
			//hasn't moved enough
			return;
		}

		if (DirtyEngagements.Num() > 0)
		{
			//engagement update
			for (auto& element : DirtyEngagements)
			{
				element.EngagementTime += SnapshotInterval;
			}
		}

		//actor component
		//LastPosition = GetOwner()->GetActorLocation();
		//LastForward = GetOwner()->GetActorForwardVector();

		//scene component
		LastPosition = GetComponentLocation();
		LastForward = GetComponentTransform().TransformVector(FVector::ForwardVector);
		
		FDynamicObjectSnapshot snapObj = MakeSnapshot();

		//TODO allow recording of dynamics before session start, but don't 'leak' snapshots to a new session
		if (snapObj.time > 1)
		{
			snapshots.Add(snapObj);
			if (snapshots.Num() + newManifest.Num() > MaxSnapshots)
			{
				TrySendData();
			}
		}
	}
}

FDynamicObjectSnapshot UDynamicObject::MakeSnapshot()
{

	//TODO check that session ends correctly from editor
	/*if (cognitivevrapi::Util::GetTimestamp() < s->LastSesisonTimestamp)
	{
		FDynamicObjectSnapshot snapshot = FDynamicObjectSnapshot();
		return snapshot;
	}*/

	//decide if the object needs a new entry in the manifest
	bool needObjectId = false;
	if (!ObjectID.IsValid() || ObjectID->Id == "")
	{
		needObjectId = true;
	}
	else
	{
		cognitivevrapi::FDynamicObjectManifestEntry entry;
		bool foundEntry = false;
		for (int32 i = 0; i < manifest.Num(); i++)
		{
			if (manifest[i].Id == ObjectID->Id)
			{
				foundEntry = true;
				break;
			}
		}
		if (!foundEntry)
		{
			needObjectId = true;
		}
	}

	if (needObjectId)
	{
		GenerateObjectId();
	}

	FDynamicObjectSnapshot snapshot = FDynamicObjectSnapshot();

	double ts = cognitivevrapi::Util::GetTimestamp();

	snapshot.time = ts;
	snapshot.id = ObjectID->Id;
	//snapshot.position = FVector(-(int32)GetOwner()->GetActorLocation().X, (int32)GetOwner()->GetActorLocation().Z, (int32)GetOwner()->GetActorLocation().Y);
	snapshot.position = FVector(-(int32)GetComponentLocation().X, (int32)GetComponentLocation().Z, (int32)GetComponentLocation().Y);
	

	FQuat quat;
	//FRotator rot = GetOwner()->GetActorRotation();
	FRotator rot = GetComponentRotation();
	quat = rot.Quaternion();

	snapshot.rotation = FQuat(quat.X, quat.Z, quat.Y, quat.W);

	//TODO snapshot properties. eg size, color, texture

	for (auto& element : DirtyEngagements)
	{
		//copying event because it could be removed below if inactive
		auto engage = cognitivevrapi::FEngagementEvent(element.EngagementType, element.Parent, element.EngagementNumber);
		engage.EngagementTime = element.EngagementTime;

		snapshot.Engagements.Add(engage);
	}

	DirtyEngagements.RemoveAll([=](const cognitivevrapi::FEngagementEvent& engage) { return engage.Active == false; });

	return snapshot;
}

TSharedPtr<FJsonValueObject> UDynamicObject::WriteSnapshotToJson(FDynamicObjectSnapshot snapshot)
{
	TSharedPtr<FJsonObject>snapObj = MakeShareable(new FJsonObject);

	//id
	snapObj->SetStringField("id", snapshot.id);

	//time
	snapObj->SetNumberField("time", snapshot.time);

	//return MakeShareable(new FJsonValueObject(snapObj));

	//positions
	TArray<TSharedPtr<FJsonValue>> posArray;
	TSharedPtr<FJsonValueNumber> JsonValue;

	JsonValue = MakeShareable(new FJsonValueNumber(snapshot.position.X)); //right
	posArray.Add(JsonValue);
	JsonValue = MakeShareable(new FJsonValueNumber(snapshot.position.Y)); //up
	posArray.Add(JsonValue);
	JsonValue = MakeShareable(new FJsonValueNumber(snapshot.position.Z));  //forward
	posArray.Add(JsonValue);

	snapObj->SetArrayField("p", posArray);

	//rotation
	TArray<TSharedPtr<FJsonValue>> rotArray;

	JsonValue = MakeShareable(new FJsonValueNumber(-snapshot.rotation.X));
	rotArray.Add(JsonValue);
	JsonValue = MakeShareable(new FJsonValueNumber(snapshot.rotation.Y));
	rotArray.Add(JsonValue);
	JsonValue = MakeShareable(new FJsonValueNumber(snapshot.rotation.Z));
	rotArray.Add(JsonValue);
	JsonValue = MakeShareable(new FJsonValueNumber(snapshot.rotation.W));
	rotArray.Add(JsonValue);

	snapObj->SetArrayField("r", rotArray);



	TArray<TSharedPtr<FJsonValueObject>> properties;

	TSharedPtr<FJsonObject> tempProperty = MakeShareable(new FJsonObject);
	for (auto& Elem : snapshot.BoolProperties)
	{
		tempProperty.Get()->Values.Empty();
		tempProperty->SetBoolField(Elem.Key, Elem.Value);
		TSharedPtr< FJsonValueObject > propertiesValue = MakeShareable(new FJsonValueObject(tempProperty));
		properties.Add(propertiesValue);
	}
	for (auto& Elem : snapshot.IntegerProperties)
	{
		tempProperty.Get()->Values.Empty();
		tempProperty->SetNumberField(Elem.Key, Elem.Value);
		TSharedPtr< FJsonValueObject > propertiesValue = MakeShareable(new FJsonValueObject(tempProperty));
		properties.Add(propertiesValue);
	}
	for (auto& Elem : snapshot.FloatProperties)
	{
		tempProperty.Get()->Values.Empty();
		tempProperty->SetNumberField(Elem.Key, Elem.Value);
		TSharedPtr< FJsonValueObject > propertiesValue = MakeShareable(new FJsonValueObject(tempProperty));
		properties.Add(propertiesValue);
	}
	for (auto& Elem : snapshot.StringProperties)
	{
		tempProperty.Get()->Values.Empty();
		tempProperty->SetStringField(Elem.Key, Elem.Value);
		TSharedPtr< FJsonValueObject > propertiesValue = MakeShareable(new FJsonValueObject(tempProperty));
		properties.Add(propertiesValue);
	}

	TArray< TSharedPtr<FJsonValue> > ObjArray;
	for (int32 i = 0; i < properties.Num(); i++)
	{
		ObjArray.Add(properties[i]);
	}

	if (ObjArray.Num() > 0)
	{
		snapObj->SetArrayField("properties", ObjArray);
	}

	if (snapshot.Engagements.Num() > 0)
	{
		TArray< TSharedPtr<FJsonValue> > engagements;

		for (auto& Elem : snapshot.Engagements)
		{
			TSharedPtr<FJsonObject>engagement = MakeShareable(new FJsonObject);

			engagement->SetStringField("engagementtype", Elem.EngagementType);
			engagement->SetStringField("engagementparent", Elem.Parent);
			engagement->SetNumberField("engagement_time", Elem.EngagementTime);
			engagement->SetNumberField("engagement_count", Elem.EngagementNumber);

			TSharedPtr< FJsonValueObject > engagementValue = MakeShareable(new FJsonValueObject(engagement));
			engagements.Add(engagementValue);
		}
		snapObj->SetArrayField("engagements", engagements);
	}

	//TSharedPtr< FJsonValueObject > outValue = MakeShareable(new FJsonValueObject(snapObj));

	return MakeShareable(new FJsonValueObject(snapObj));
}

void UDynamicObject::TrySendData()
{
	TSharedPtr<FAnalyticsProviderCognitiveVR> cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider();
	if (cog->GetWorld() != NULL)
	{
		bool withinMinTimer = LastSendTime + MinTimer > cog->GetWorld()->GetRealTimeSeconds();
		bool withinExtremeBatchSize = newManifest.Num() + snapshots.Num() < ExtremeBatchSize;

		if (withinMinTimer && withinExtremeBatchSize)
		{
			return;
		}
		SendData();
	}
}

//static
void UDynamicObject::SendData()
{
	TSharedPtr<FAnalyticsProviderCognitiveVR> cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider();
	if (!cog.IsValid() || !cog->HasStartedSession())
	{
		return;
	}

	TSharedPtr<cognitivevrapi::FSceneData> currentscenedata = cog->GetCurrentSceneData();
	if (!currentscenedata.IsValid())
	{
		GLog->Log("DynamicObject::SendData current scene data is invalid");
		return;
	}

	if (newManifest.Num() + snapshots.Num() == 0)
	{
		cognitivevrapi::CognitiveLog::Info("UDynamicObject::SendData no objects or data to send!");
		cog->GetWorld()->GetGameInstance()->GetTimerManager().SetTimer(CognitiveDynamicAutoSendHandle, FTimerDelegate::CreateStatic(&UDynamicObject::SendData), AutoTimer, false);
		return;
	}	
	
	if (cog->GetWorld() != NULL)
	{
		LastSendTime = cog->GetWorld()->GetRealTimeSeconds();
	}

	cog->GetWorld()->GetGameInstance()->GetTimerManager().SetTimer(CognitiveDynamicAutoSendHandle, FTimerDelegate::CreateStatic(&UDynamicObject::SendData), AutoTimer, false);

	TArray<TSharedPtr<FJsonValueObject>> EventArray = UDynamicObject::DynamicSnapshotsToString();

	TSharedPtr<FJsonObject>wholeObj = MakeShareable(new FJsonObject);

	wholeObj->SetStringField("userid", cog->GetUserID());
	if (!cog->LobbyId.IsEmpty())
	{
		wholeObj->SetStringField("lobbyId", cog->LobbyId);
	}
	wholeObj->SetNumberField("timestamp", (int32)cog->GetSessionTimestamp());
	wholeObj->SetStringField("sessionid", cog->GetSessionID());
	wholeObj->SetStringField("formatversion", "1.0");
	wholeObj->SetNumberField("part", jsonPart);
	jsonPart++;

	

	if (newManifest.Num() > 0)
	{
		TSharedPtr<FJsonObject> ManifestObject = UDynamicObject::DynamicObjectManifestToString();
		wholeObj->SetObjectField("manifest", ManifestObject);
		newManifest.Empty();
	}

	TArray< TSharedPtr<FJsonValue> > ObjArray;
	for (int32 i = 0; i < EventArray.Num(); i++)
	{
		ObjArray.Add(EventArray[i]);
	}

	wholeObj->SetArrayField("data", ObjArray);

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(wholeObj.ToSharedRef(), Writer);
	//cog->SendJson("dynamics", OutputString);
	//FString sceneid = cog->GetCurrentSceneId();
	cog->network->NetworkCall("dynamics", OutputString);

	snapshots.Empty();
}

TSharedPtr<FJsonObject> UDynamicObject::DynamicObjectManifestToString()
{
	TSharedPtr<FJsonObject> manifestObject = MakeShareable(new FJsonObject);

	for (int32 i = 0; i != newManifest.Num(); ++i)
	{
		TSharedPtr<FJsonObject>entry = MakeShareable(new FJsonObject);
		entry->SetStringField("name", newManifest[i].Name);
		entry->SetStringField("mesh", newManifest[i].MeshName);

		manifestObject->SetObjectField(newManifest[i].Id, entry);
	}

	return manifestObject;
}

TArray<TSharedPtr<FJsonValueObject>> UDynamicObject::DynamicSnapshotsToString()
{
	TArray<TSharedPtr<FJsonValueObject>> dataArray;

	for (int32 i = 0; i != snapshots.Num(); ++i)
	{
		dataArray.Add(UDynamicObject::WriteSnapshotToJson(snapshots[i]));
	}
	return dataArray;
}

/*FDynamicObjectSnapshot UDynamicObject::NewSnapshot()
{
	FDynamicObjectSnapshot initSnapshot = MakeSnapshot();
	snapshots.Add(initSnapshot);
	return initSnapshot;
}*/

FDynamicObjectSnapshot UDynamicObject::SnapshotStringProperty(UPARAM(ref)FDynamicObjectSnapshot& snapshot, FString key, FString stringValue)
{
	snapshot.StringProperties.Add(key, stringValue);
	return snapshot;
}

FDynamicObjectSnapshot UDynamicObject::SnapshotBoolProperty(UPARAM(ref) FDynamicObjectSnapshot& snapshot, FString key, bool boolValue)
{
	snapshot.BoolProperties.Add(key, boolValue);
	return snapshot;
}

FDynamicObjectSnapshot UDynamicObject::SnapshotFloatProperty(UPARAM(ref)FDynamicObjectSnapshot& snapshot, FString key, float floatValue)
{
	snapshot.FloatProperties.Add(key, floatValue);
	return snapshot;
}

FDynamicObjectSnapshot UDynamicObject::SnapshotIntegerProperty(UPARAM(ref)FDynamicObjectSnapshot& snapshot, FString key, int32 intValue)
{
	snapshot.IntegerProperties.Add(key, intValue);
	return snapshot;
}

void UDynamicObject::SendDynamicObjectSnapshot(UPARAM(ref)FDynamicObjectSnapshot& snapshot)
{
	snapshots.Add(snapshot);
	if (snapshots.Num() + newManifest.Num() > MaxSnapshots)
	{
		TrySendData();
	}
}

void UDynamicObject::BeginEngagement(UDynamicObject* target, FString engagementType)
{
	if (target != nullptr)
	{
		if (target->ObjectID.IsValid())
		{
			target->BeginEngagementId(engagementType, target->ObjectID->Id);
		}
	}
}

void UDynamicObject::BeginEngagementId(FString engagementName, FString parentObjectId)
{
	bool didFindEvent = false;
	int32 previousEngagementCount = 0;

	for (auto& e : Engagements)
	{
		if (e.EngagementType == engagementName)
		{
			previousEngagementCount++;
			//foundEvent = &e;
		}
	}
	cognitivevrapi::FEngagementEvent newEngagement = cognitivevrapi::FEngagementEvent(engagementName, parentObjectId, previousEngagementCount + 1);
	DirtyEngagements.Add(newEngagement);
	Engagements.Add(newEngagement);
}

void UDynamicObject::EndEngagement(UDynamicObject* target, FString engagementType)
{
	if (target != nullptr)
	{
		if (target->ObjectID.IsValid())
		{
			target->EndEngagementId(engagementType, target->ObjectID->Id);
		}
	}
}

void UDynamicObject::EndEngagementId(FString engagementName, FString parentObjectId)
{
	cognitivevrapi::FEngagementEvent* foundEvent = NULL;
	for (auto& e : DirtyEngagements)
	{
		if (e.EngagementType == engagementName && (e.Parent == parentObjectId || parentObjectId == ""))
		{
			foundEvent = &e;
			break;
		}
	}

	if (foundEvent != nullptr)
	{
		foundEvent->Active = false;
	}
	else
	{
		int32 previousEngagementCount = 0;
		for (auto& e : Engagements)
		{
			if (e.EngagementType == engagementName)
			{
				previousEngagementCount++;
				//foundEvent = &e;
			}
		}

		cognitivevrapi::FEngagementEvent newEngagement = cognitivevrapi::FEngagementEvent(engagementName, parentObjectId, previousEngagementCount + 1);
		newEngagement.Active = false;
		DirtyEngagements.Add(newEngagement);
		Engagements.Add(newEngagement);
	}
}

void UDynamicObject::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (ReleaseIdOnDestroy && !TrackGaze)
	{
		FDynamicObjectSnapshot initSnapshot = MakeSnapshot();
		SnapshotBoolProperty(initSnapshot, "enable", false);
		
		if (initSnapshot.time > 1)
		{
			snapshots.Add(initSnapshot);
		}

		ObjectID->Used = false;
	}

	if (EndPlayReason == EEndPlayReason::EndPlayInEditor)
	{
		snapshots.Empty();
		allObjectIds.Empty();
		manifest.Empty();
		newManifest.Empty();
		jsonPart = 1;
	}
}

cognitivevrapi::FDynamicObjectManifestEntry* cognitivevrapi::FDynamicObjectManifestEntry::SetProperty(FString key, FString value)
{
	this->StringProperties.Add(key, value);
	return this;
}