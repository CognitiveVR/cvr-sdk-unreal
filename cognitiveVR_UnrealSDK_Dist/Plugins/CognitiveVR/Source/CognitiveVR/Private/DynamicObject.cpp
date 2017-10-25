// Fill out your copyright notice in the Description page of Project Settings.

#include "CognitiveVR.h"
#include "DynamicObject.h"
#include "CognitiveVRSettings.h"
#include "Util.h"

TArray<FDynamicObjectSnapshot> snapshots;
TArray<FDynamicObjectManifestEntry> manifest;
TArray<FDynamicObjectManifestEntry> newManifest;
TArray<TSharedPtr<FDynamicObjectId>> allObjectIds;
int32 jsonPart = 1;
int32 MaxSnapshots = -1;

// Sets default values for this component's properties
UDynamicObject::UDynamicObject()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	//bWantsBeginPlay = true;
	PrimaryComponentTick.bCanEverTick = true;
}

void UDynamicObject::OnComponentCreated()
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
		MeshName = staticmeshComponent->GetStaticMesh()->GetName();
	}
}

void UDynamicObject::BeginPlay()
{
	Super::BeginPlay();

	s = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider();

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
	}

	if (!s->HasStartedSession())
	{
		//TODO this should set a callback when session becomes started

		//s->OnInitResponse.AddSP(this, &UDynamicObject::BeginPlayCallback); //multicast delegate style

		//s->OnInitResponse().
		s->OnInitResponse().AddUObject(this, &UDynamicObject::BeginPlayCallback);

		FString UE4Str = GetOwner()->GetName();
		std::string MyStdString(TCHAR_TO_UTF8(*UE4Str));

		//s->OnInitResponse.Bind(this, BeginPlayCallback);

		return;
	}

	BeginPlayCallback(true);
	FString UE4Str2 = GetOwner()->GetName();
	std::string MyStdString3(TCHAR_TO_UTF8(*UE4Str2));
}

void UDynamicObject::BeginPlayCallback(bool successful)
{
	LastPosition = GetOwner()->GetActorLocation();
	LastForward = GetOwner()->GetActorForwardVector();

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
			SendData();
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

TSharedPtr<FDynamicObjectId> UDynamicObject::GetUniqueId(FString meshName)
{
	TSharedPtr<FDynamicObjectId> freeId;
	static int32 originalId = 1000;
	originalId++;

	freeId = MakeShareable(new FDynamicObjectId(originalId, meshName));
	return freeId;
}

TSharedPtr<FDynamicObjectId> UDynamicObject::GetObjectId()
{
	return ObjectID;
}

void UDynamicObject::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );

	if (!UpdateOnTick) { return; }

	if (!s->HasStartedSession())
	{
		return;
	}

	currentTime += DeltaTime;
	if (currentTime > SnapshotInterval)
	{
		currentTime -= SnapshotInterval;

		FVector currentForward = GetOwner()->GetActorForwardVector();

		currentForward.Normalize();
		
		float dotRot = FVector::DotProduct(LastForward, currentForward);

		float actualDegrees = FMath::Acos(FMath::Clamp<float>(dotRot, -1.0, 1.0)) * 57.29578;

		if ((LastPosition - GetOwner()->GetActorLocation()).Size() > PositionThreshold)
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

		LastPosition = GetOwner()->GetActorLocation();
		LastForward = GetOwner()->GetActorForwardVector();
		
		FDynamicObjectSnapshot snapObj = MakeSnapshot();

		if (snapObj.time > 1)
		{
			snapshots.Add(snapObj);
		}

		if (snapshots.Num() + newManifest.Num() > MaxSnapshots)
		{
			SendData();
		}
	}
}

FDynamicObjectSnapshot UDynamicObject::MakeSnapshot()
{
	if (!s.IsValid())
	{
		//can't stop snapshots here. at beginning of the game, the manager might not be finished setting up
		CognitiveLog::Error("DynamicObject::MakeSnapshot provider is null. finding provider");
		s = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider();
	}

	if (Util::GetTimestamp() < s->LastSesisonTimestamp)
	{
		FDynamicObjectSnapshot snapshot = FDynamicObjectSnapshot();
		return snapshot;
	}

	//decide if the object needs a new entry in the manifest
	bool needObjectId = false;
	if (!ObjectID.IsValid() || ObjectID->Id == -1)
	{
		needObjectId = true;
	}
	else
	{
		FDynamicObjectManifestEntry entry;
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
		if (!UseCustomId)
		{
			TSharedPtr<FDynamicObjectId> recycledId;
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
				std::string MyStdString(TCHAR_TO_UTF8(*MeshName));
				CognitiveLog::Info("UDynamicObject::Recycle ObjectID! " + MyStdString);
			}
			else
			{
				std::string MyStdString(TCHAR_TO_UTF8(*MeshName));
				CognitiveLog::Info("UDynamicObject::Get new ObjectID! " + MyStdString);
				ObjectID = GetUniqueId(MeshName);

				allObjectIds.Add(ObjectID);
			}

			FDynamicObjectManifestEntry entry = FDynamicObjectManifestEntry(ObjectID->Id, GetOwner()->GetName(), MeshName);
			if (!GroupName.IsEmpty())
			{
				entry.SetProperty("groupname", GroupName);
			}
			manifest.Add(entry);
			newManifest.Add(entry);
		}
		else
		{
			ObjectID = MakeShareable(new FDynamicObjectId(CustomId, MeshName));
			FDynamicObjectManifestEntry entry = FDynamicObjectManifestEntry(ObjectID->Id, GetOwner()->GetName(), MeshName);
			if (!GroupName.IsEmpty())
			{
				//entry.
				entry.SetProperty("groupname", GroupName);
			}
			manifest.Add(entry);
			newManifest.Add(entry);
		}

		if (manifest.Num() == 1)
		{
			CognitiveLog::Info("DynamicObject::MakeSnapshot Register Provider->OnSendData for Dynamics");
			s->OnSendData.AddStatic(SendData);
		}
	}

	FDynamicObjectSnapshot snapshot = FDynamicObjectSnapshot();

	double ts = Util::GetTimestamp();

	snapshot.time = ts;
	snapshot.id = ObjectID->Id;
	snapshot.position = FVector(-(int32)GetOwner()->GetActorLocation().X, (int32)GetOwner()->GetActorLocation().Z, (int32)GetOwner()->GetActorLocation().Y);
	

	FQuat quat;
	FRotator rot = GetOwner()->GetActorRotation();
	quat = rot.Quaternion();

	snapshot.rotation = FQuat(quat.X, quat.Z, quat.Y, quat.W);

	//TODO snapshot properties. eg size, color, texture

	for (auto& element : DirtyEngagements)
	{
		//copying event because it could be removed below if inactive
		auto engage = FEngagementEvent(element.EngagementType, element.Parent, element.EngagementNumber);
		engage.EngagementTime = element.EngagementTime;

		snapshot.Engagements.Add(engage);
	}

	DirtyEngagements.RemoveAll([=](const FEngagementEvent& engage) { return engage.Active == false; });

	return snapshot;
}

TSharedPtr<FJsonValueObject> UDynamicObject::WriteSnapshotToJson(FDynamicObjectSnapshot snapshot)
{
	TSharedPtr<FJsonObject>snapObj = MakeShareable(new FJsonObject);

	//id
	snapObj->SetNumberField("id", snapshot.id);

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

			engagement->SetNumberField("engagementparent", Elem.Parent);
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

void UDynamicObject::SendData()
{
	UWorld* myworld = GWorld;
	if (myworld == NULL)
	{
		snapshots.Empty();
		return;
	}

	FString currentSceneName = myworld->GetMapName();
	currentSceneName.RemoveFromStart(myworld->StreamingLevelsPrefix);
	UDynamicObject::SendData(currentSceneName);
}

//TODO only combine 64 entries, prioritizing the manifest
void UDynamicObject::SendData(FString sceneName)
{
	if (newManifest.Num() + snapshots.Num() == 0)
	{
		CognitiveLog::Info("UDynamicObject::SendData no objects or data to send!");
		return;
	}	
	
	TArray<TSharedPtr<FJsonValueObject>> EventArray = UDynamicObject::DynamicSnapshotsToString();

	TSharedPtr<FJsonObject>wholeObj = MakeShareable(new FJsonObject);
	
	FAnalyticsProviderCognitiveVR* cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Get();

	wholeObj->SetStringField("userid", cog->GetDeviceID());
	wholeObj->SetNumberField("timestamp", (int32)cog->GetSessionTimestamp());
	wholeObj->SetStringField("sessionid", cog->GetCognitiveSessionID());
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
	cog->SendJson("dynamics", OutputString);

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

		manifestObject->SetObjectField(FString::FromInt(newManifest[i].Id), entry);
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
		SendData();
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

void UDynamicObject::BeginEngagementId(FString engagementName, int32 parentObjectId)
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
	FEngagementEvent newEngagement = FEngagementEvent(engagementName, parentObjectId, previousEngagementCount + 1);
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

void UDynamicObject::EndEngagementId(FString engagementName, int32 parentObjectId)
{
	FEngagementEvent* foundEvent = NULL;
	for (auto& e : DirtyEngagements)
	{
		if (e.EngagementType == engagementName && (e.Parent == parentObjectId || parentObjectId == -1))
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

		FEngagementEvent newEngagement = FEngagementEvent(engagementName, parentObjectId, previousEngagementCount + 1);
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

FDynamicObjectManifestEntry* FDynamicObjectManifestEntry::SetProperty(FString key, FString value)
{
	this->StringProperties.Add(key, value);
	return this;
}