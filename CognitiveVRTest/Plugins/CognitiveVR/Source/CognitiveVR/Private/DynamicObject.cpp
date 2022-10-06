// Fill out your copyright notice in the Description page of Project Settings.

#include "DynamicObject.h"
//#include "CognitiveVRSettings.h"
//#include "Util.h"

bool UDynamicObject::callbackInitialized = false;
int32 UDynamicObject::jsonPart = 1;
int32 UDynamicObject::MaxSnapshots = -1;
int32 UDynamicObject::MinTimer = 5;
int32 UDynamicObject::AutoTimer = 10;
int32 UDynamicObject::ExtremeBatchSize = 128;
float UDynamicObject::NextSendTime = 0;
float UDynamicObject::LastSendTime = -60;
FString UDynamicObject::DynamicObjectFileType = "gltf";
TArray<FDynamicObjectSnapshot> UDynamicObject::snapshots;
TArray<FDynamicObjectManifestEntry> UDynamicObject::manifest;
TArray<FDynamicObjectManifestEntry> UDynamicObject::newManifest;
TArray<TSharedPtr<FDynamicObjectId>> UDynamicObject::allObjectIds;
FTimerHandle UDynamicObject::CognitiveDynamicAutoSendHandle;
TSharedPtr<FAnalyticsProviderCognitiveVR> UDynamicObject::cogProvider;

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
		if (actorComponent != NULL)
		{
			UStaticMeshComponent* staticmeshComponent = Cast<UStaticMeshComponent>(actorComponent);
			if (staticmeshComponent != NULL && staticmeshComponent->GetStaticMesh() != NULL)
			{
				UseCustomMeshName = true;
				MeshName = staticmeshComponent->GetStaticMesh()->GetName();
			}
		}

		UActorComponent* actorSkeletalComponent = GetOwner()->GetComponentByClass(USkeletalMeshComponent::StaticClass());
		if (actorSkeletalComponent != NULL)
		{
			USkeletalMeshComponent* staticmeshComponent = Cast<USkeletalMeshComponent>(actorSkeletalComponent);
			if (staticmeshComponent != NULL && staticmeshComponent->SkeletalMesh != NULL)
			{
				UseCustomMeshName = true;
				MeshName = staticmeshComponent->GetName();
			}
		}
		
	}
}

void UDynamicObject::GenerateCustomId()
{
	IdSourceType = EIdSourceType::CustomId;
	CustomId = FGuid::NewGuid().ToString();
}

void UDynamicObject::TryGenerateCustomIdAndMesh()
{
	if (GetOwner() == NULL)
	{
		return;
	}

	if (MeshName.IsEmpty())
	{
		bool foundAnyMesh = false;
		UActorComponent* actorComponent = GetOwner()->GetComponentByClass(UStaticMeshComponent::StaticClass());
		if (actorComponent != NULL)
		{
			UStaticMeshComponent* staticmeshComponent = Cast<UStaticMeshComponent>(actorComponent);
			if (staticmeshComponent != NULL && staticmeshComponent->GetStaticMesh() != NULL)
			{
				MeshName = staticmeshComponent->GetStaticMesh()->GetName();
				foundAnyMesh = true;
			}
		}

		UActorComponent* actorSkeletalComponent = GetOwner()->GetComponentByClass(USkeletalMeshComponent::StaticClass());
		if (actorSkeletalComponent != NULL)
		{
			USkeletalMeshComponent* staticmeshComponent = Cast<USkeletalMeshComponent>(actorSkeletalComponent);
			if (staticmeshComponent != NULL && staticmeshComponent->SkeletalMesh != NULL)
			{
				MeshName = staticmeshComponent->SkeletalMesh->GetName();
				foundAnyMesh = true;
			}
		}
		if (foundAnyMesh)
		{
			UseCustomMeshName = true;
			GWorld->MarkPackageDirty();
		}
	}
	if (CustomId.IsEmpty())
	{
		CustomId = FGuid::NewGuid().ToString();
		GWorld->MarkPackageDirty();
	}
}

//static
void UDynamicObject::ClearSnapshots()
{
	//when playing in editor, sometimes snapshots will persist in this list
	snapshots.Empty();
}

//static
void UDynamicObject::OnSessionBegin()
{
	if (!cogProvider.IsValid())
	{
		cogProvider = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	}
	UWorld* sessionWorld = cogProvider->EnsureGetWorld();

	if (sessionWorld == NULL)
	{
		CognitiveLog::Error("UDynamicObject::OnSessionBegin SessionWorld is null! Need to make playertracker initialize first");
		return;
	}

	for (TObjectIterator<UDynamicObject> Itr; Itr; ++Itr)
	{
		//itr will also return editor world objects
		//sessionWorld is either game or PIE type
		if (Itr->GetWorld() != sessionWorld) { continue; }

		if (Itr->SnapshotOnBeginPlay)
		{
			Itr->Initialize();
		}
	}
}

void UDynamicObject::BeginPlay()
{
	Super::BeginPlay();
	Initialize();
}

void UDynamicObject::Initialize()
{
	if (HasInitialized)
	{
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
			}
		}
	}

	if (!callbackInitialized)
	{
		callbackInitialized = true;
		GetWorld()->GetGameInstance()->GetTimerManager().SetTimer(CognitiveDynamicAutoSendHandle, FTimerDelegate::CreateStatic(&UDynamicObject::SendData, false), AutoTimer, true);
	}

	//even if session has not started, still collect data
	//session must be started to send
	//scene id must be valid to send

	//scene component
	LastPosition = GetComponentLocation();
	LastForward = GetComponentTransform().TransformVector(FVector::ForwardVector);
	LastScale = FVector(1, 1, 1);

	if (!UseCustomMeshName)
	{
		UseCustomMeshName = true;
		if (CommonMeshName == ECommonMeshName::OculusRiftTouchLeft)
		{
			MeshName = "oculustouchleft";
		}
		else if (CommonMeshName == ECommonMeshName::OculusRiftTouchRight)
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
		else if (CommonMeshName == ECommonMeshName::WindowsMixedRealityLeft)
		{
			MeshName = "windows_mixed_reality_controller_left";
		}
		else if (CommonMeshName == ECommonMeshName::WindowsMixedRealityRight)
		{
			MeshName = "windows_mixed_reality_controller_right";
		}
		else if (CommonMeshName == ECommonMeshName::PicoNeo2EyeControllerLeft)
		{
			MeshName = "pico_neo_2_eye_controller_left";
		}
		else if (CommonMeshName == ECommonMeshName::PicoNeo2EyeControllerRight)
		{
			MeshName = "pico_neo_2_eye_controller_right";
		}
	}

	if (MeshName == "")
	{
		//hasn't been initialized correctly
		return;
	}

	if (!cogProvider.IsValid())
		cogProvider = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();

	if (!cogProvider.IsValid())
	{
		CognitiveLog::Warning("UDynamicObject::BeginPlay cannot find CognitiveVRProvider!");
		return;
	}

	if (SnapshotOnBeginPlay)
	{
		if (!cogProvider->HasStartedSession())
		{
			return;
		}

		bool hasScaleChanged = true;
		if (FMath::Abs(LastScale.Size() - GetComponentTransform().GetScale3D().Size()) > ScaleThreshold)
		{
			hasScaleChanged = true;
		}

		FDynamicObjectSnapshot initSnapshot = MakeSnapshot(hasScaleChanged);
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
	HasInitialized = true;
}

TSharedPtr<FDynamicObjectId> UDynamicObject::GetUniqueId(FString meshName)
{
	TSharedPtr<FDynamicObjectId> freeId;
	static int32 originalId = 1000;
	originalId++;

	freeId = MakeShareable(new FDynamicObjectId(FString::FromInt(originalId), meshName));
	return freeId;
}

TSharedPtr<FDynamicObjectId> UDynamicObject::GetObjectId()
{
	if (!ObjectID.IsValid() || ObjectID->Id == "")
	{
		GenerateObjectId();
	}
	return ObjectID;
}

void UDynamicObject::GenerateObjectId()
{
	if (IdSourceType == EIdSourceType::PoolId)
	{
		FString poolId;
		if (IDPool == NULL)
		{
			ObjectID = GetUniqueId(MeshName);
			allObjectIds.Add(ObjectID);
			FDynamicObjectManifestEntry entry = FDynamicObjectManifestEntry(ObjectID->Id, GetOwner()->GetName(), MeshName);
			if (IsController)
			{
				entry.ControllerType = ControllerType;
				entry.IsRight = IsRightController;
			}
			manifest.Add(entry);
			newManifest.Add(entry);
			return;
		}
		else
		{
			HasValidPoolId = IDPool->GetId(poolId);
			ObjectID = MakeShareable(new FDynamicObjectId(poolId, MeshName));
			FDynamicObjectManifestEntry entry = FDynamicObjectManifestEntry(ObjectID->Id, GetOwner()->GetName(), MeshName);
			manifest.Add(entry);
			newManifest.Add(entry);
			return;
		}
	}

	if (IdSourceType != EIdSourceType::CustomId || CustomId.IsEmpty())
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
			CognitiveLog::Info("UDynamicObject::Recycle ObjectID! " + MeshName);
		}
		else
		{
			CognitiveLog::Info("UDynamicObject::Get new ObjectID! " + MeshName);
			ObjectID = GetUniqueId(MeshName);

			allObjectIds.Add(ObjectID);
		}

		FDynamicObjectManifestEntry entry = FDynamicObjectManifestEntry(ObjectID->Id, GetOwner()->GetName(), MeshName);
		if (IsController)
		{
			entry.ControllerType = ControllerType;
			entry.IsRight = IsRightController;
		}
		manifest.Add(entry);
		newManifest.Add(entry);
	}
	else
	{
		ObjectID = MakeShareable(new FDynamicObjectId(CustomId, MeshName));
		FDynamicObjectManifestEntry entry = FDynamicObjectManifestEntry(ObjectID->Id, GetOwner()->GetName(), MeshName);
		if (IsController)
		{
			entry.ControllerType = ControllerType;
			entry.IsRight = IsRightController;
		}
		manifest.Add(entry);
		newManifest.Add(entry);
	}
}

void UDynamicObject::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );

	if (!cogProvider.IsValid()) { return; }
	if (!cogProvider->HasStartedSession()) { return; }
	if (!SnapshotOnInterval) { return; }

	currentTime += DeltaTime;
	if (currentTime > SnapshotInterval)
	{
		currentTime -= SnapshotInterval;

		FVector currentForward = GetForwardVector();
		
		float dotRot = FVector::DotProduct(LastForward, currentForward);

		float actualDegrees = FMath::Acos(FMath::Clamp<float>(dotRot, -1.0, 1.0)) * 57.29578;

		bool hasScaleChanged = false;

		if (FMath::Abs(LastScale.Size() - GetComponentTransform().GetScale3D().Size()) > ScaleThreshold)
		{
			hasScaleChanged = true;
		}

		if (!hasScaleChanged)
		{
			if ((LastPosition - GetComponentLocation()).Size() > PositionThreshold)
			{
				//moved
			}
			else if (actualDegrees > RotationThreshold) //rotator stuff
			{
				//rotated
			}
			else
			{
				//hasn't moved enough
				return;
			}
		}

		//scene component
		LastPosition = GetComponentLocation();
		LastForward = currentForward;
		if (hasScaleChanged)
		{
			LastScale = GetComponentTransform().GetScale3D();
		}

		FDynamicObjectSnapshot snapObj = MakeSnapshot(hasScaleChanged);

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

FDynamicObjectSnapshot UDynamicObject::MakeSnapshot(bool hasChangedScale)
{
	//decide if the object needs a new entry in the manifest
	bool needObjectId = false;
	if (!ObjectID.IsValid() || ObjectID->Id == "")
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
		GenerateObjectId();
	}

	FDynamicObjectSnapshot snapshot = FDynamicObjectSnapshot();

	double ts = Util::GetTimestamp();

	snapshot.time = ts;
	snapshot.id = ObjectID->Id;
	snapshot.position = FVector(-GetComponentLocation().X, GetComponentLocation().Z, GetComponentLocation().Y);

	if (hasChangedScale)
	{
		snapshot.hasScaleChanged = true;
		snapshot.scale = GetComponentTransform().GetScale3D();
	}

	FQuat quat;
	FRotator rot = GetComponentRotation();
	quat = rot.Quaternion();

	snapshot.rotation = FQuat(quat.X, quat.Z, quat.Y, quat.W);

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
	TArray<TSharedPtr<FJsonValue>> scaleArray;
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

	if (snapshot.hasScaleChanged)
	{
		//scale
		JsonValue = MakeShareable(new FJsonValueNumber(snapshot.scale.X));
		scaleArray.Add(JsonValue);
		JsonValue = MakeShareable(new FJsonValueNumber(snapshot.scale.Z));
		scaleArray.Add(JsonValue);
		JsonValue = MakeShareable(new FJsonValueNumber(snapshot.scale.Y));
		scaleArray.Add(JsonValue);

		snapObj->SetArrayField("s", scaleArray);
	}

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

	if (snapshot.Buttons.Num() > 0)
	{
		//write button properties

		TSharedPtr<FJsonObject> buttons = MakeShareable(new FJsonObject);
		
		for (auto& Elem : snapshot.Buttons)
		{
			TSharedPtr<FJsonObject> button = MakeShareable(new FJsonObject);

			if (Elem.Value.IsVector)
			{
				button->SetNumberField("buttonPercent", Elem.Value.AxisValue);
				button->SetNumberField("x", Elem.Value.X);
				button->SetNumberField("y", Elem.Value.Y);
			}
			else
			{
				button->SetNumberField("buttonPercent", Elem.Value.AxisValue);
			}
			buttons->SetObjectField(Elem.Key, button);
		}

		snapObj->SetObjectField("buttons", buttons);
	}

	//TSharedPtr< FJsonValueObject > outValue = MakeShareable(new FJsonValueObject(snapObj));

	return MakeShareable(new FJsonValueObject(snapObj));
}

void UDynamicObject::TrySendData()
{
	if (!cogProvider.IsValid()) { return; }

	if (cogProvider->GetWorld() != NULL)
	{
		bool withinMinTimer = LastSendTime + MinTimer > UCognitiveVRBlueprints::GetSessionDuration();
		bool withinExtremeBatchSize = newManifest.Num() + snapshots.Num() < ExtremeBatchSize;

		if (withinMinTimer && withinExtremeBatchSize)
		{
			return;
		}
		SendData();
	}
}

//static
void UDynamicObject::SendData(bool copyDataToCache)
{
	if (!cogProvider.IsValid() || !cogProvider->HasStartedSession())
	{
		return;
	}

	TSharedPtr<FSceneData> currentscenedata = cogProvider->GetCurrentSceneData();
	if (!currentscenedata.IsValid())
	{
		return;
	}

	if (newManifest.Num() + snapshots.Num() == 0)
	{
		CognitiveLog::Info("UDynamicObject::SendData no objects or data to send!");
		return;
	}

	LastSendTime = UCognitiveVRBlueprints::GetSessionDuration();

	TArray<TSharedPtr<FJsonValueObject>> EventArray = UDynamicObject::DynamicSnapshotsToString();

	TSharedPtr<FJsonObject>wholeObj = MakeShareable(new FJsonObject);

	wholeObj->SetStringField("userid", cogProvider->GetUserID());
	if (!cogProvider->LobbyId.IsEmpty())
	{
		wholeObj->SetStringField("lobbyId", cogProvider->LobbyId);
	}
	wholeObj->SetNumberField("timestamp", cogProvider->GetSessionTimestamp());
	wholeObj->SetStringField("sessionid", cogProvider->GetSessionID());
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

	if (ObjArray.Num() > 0)
	{
		wholeObj->SetArrayField("data", ObjArray);
	}


	FString OutputString;
	auto Writer = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&OutputString);
	FJsonSerializer::Serialize(wholeObj.ToSharedRef(), Writer);
	cogProvider->network->NetworkCall("dynamics", OutputString, copyDataToCache);

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
		entry->SetStringField("fileType", DynamicObjectFileType);
		if (!newManifest[i].ControllerType.IsEmpty())
		{
			entry->SetStringField("controllerType", newManifest[i].ControllerType);
			TSharedPtr<FJsonObject>controllerProps = MakeShareable(new FJsonObject);
			controllerProps->SetStringField("controller", newManifest[i].IsRight ? "right" : "left");
			entry->SetObjectField("properties", controllerProps);
		}

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

void UDynamicObject::BeginEngagement(UDynamicObject* target, FString engagementName, FString UniqueEngagementId)
{
	if (target != nullptr)
	{
		if (target->ObjectID.IsValid())
		{
			target->BeginEngagementId(target->ObjectID->Id, engagementName, UniqueEngagementId);
		}
	}
}

void UDynamicObject::BeginEngagementId(FString parentDynamicObjectId, FString engagementName, FString UniqueEngagementId)
{
	if (UniqueEngagementId.IsEmpty())
	{
		UniqueEngagementId = parentDynamicObjectId + " " + engagementName;
	}

	UCustomEvent* ce = NewObject<UCustomEvent>(this);
	ce->SetCategory(engagementName);
	ce->SetDynamicObject(parentDynamicObjectId);
	if (!Engagements.Contains(UniqueEngagementId))
	{
		Engagements.Add(UniqueEngagementId, ce);
	}
	else
	{
		Engagements[UniqueEngagementId]->SetPosition(FVector(-GetComponentLocation().X, GetComponentLocation().Z, GetComponentLocation().Y));
		Engagements[UniqueEngagementId]->Send();
		Engagements[UniqueEngagementId] = ce;
	}
}

void UDynamicObject::EndEngagement(UDynamicObject* target, FString engagementName, FString UniqueEngagementId)
{
	if (target != nullptr)
	{
		if (target->ObjectID.IsValid())
		{
			target->EndEngagementId(target->ObjectID->Id, engagementName, UniqueEngagementId);
		}
	}
}

void UDynamicObject::EndEngagementId(FString parentDynamicObjectId, FString engagementName, FString UniqueEngagementId)
{
	if (UniqueEngagementId.IsEmpty())
	{
		UniqueEngagementId = parentDynamicObjectId + " " + engagementName;
	}

	if (Engagements.Contains(UniqueEngagementId))
	{
		Engagements[UniqueEngagementId]->SetPosition(FVector(-GetComponentLocation().X, GetComponentLocation().Z, GetComponentLocation().Y));
		Engagements[UniqueEngagementId]->Send();
		Engagements.Remove(UniqueEngagementId);
	}
	else
	{
		//start and end event
		UCustomEvent* ce = NewObject<UCustomEvent>(this);
		ce->SetCategory(engagementName);
		ce->SetDynamicObject(parentDynamicObjectId);
		ce->SetPosition(FVector(-GetComponentLocation().X, GetComponentLocation().Z, GetComponentLocation().Y));
		ce->Send();
	}
}

//instance
void UDynamicObject::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IdSourceType == EIdSourceType::PoolId && HasValidPoolId && IDPool != NULL)
	{
		if (ObjectID.IsValid() && !ObjectID->Id.IsEmpty())
		{
			IDPool->ReturnId(ObjectID->Id);
			HasValidPoolId = false;
		}
	}

	if (!cogProvider.IsValid())
	{
		//will get an 'EndPlay' when PIE closes
		return;
	}
	if (ReleaseIdOnDestroy && cogProvider->HasStartedSession())
	{
		FDynamicObjectSnapshot initSnapshot = MakeSnapshot(false);
		SnapshotBoolProperty(initSnapshot, "enable", false);
		
		if (initSnapshot.time > 1)
		{
			snapshots.Add(initSnapshot);
		}

		ObjectID->Used = false;
	}
	HasInitialized = false;

	if (EndPlayReason == EEndPlayReason::EndPlayInEditor)
	{
		//go through all engagements and send any that match this objectid
		for (auto &Elem : Engagements)
		{
			if (Elem.Value->GetDynamicId() == ObjectID->Id)
			{
				Elem.Value->SetPosition(FVector(-GetComponentLocation().X, GetComponentLocation().Z, GetComponentLocation().Y));
				Elem.Value->Send();
			}
		}
	}
	else if (EndPlayReason == EEndPlayReason::Destroyed || EndPlayReason == EEndPlayReason::LevelTransition || EndPlayReason == EEndPlayReason::RemovedFromWorld)
	{
		//go through all engagements and send any that match this objectid
		for (auto &Elem : Engagements)
		{
			if (Elem.Value->GetDynamicId() == ObjectID->Id)
			{
				Elem.Value->SetPosition(FVector(-GetComponentLocation().X, GetComponentLocation().Z, GetComponentLocation().Y));
				Elem.Value->Send();
			}
		}
	}
}

//static
void UDynamicObject::OnSessionEnd()
{
	snapshots.Empty();
	allObjectIds.Empty();
	manifest.Empty();
	newManifest.Empty();
	jsonPart = 1;
	callbackInitialized = false;

	for (TObjectIterator<UDynamicObject> Itr; Itr; ++Itr)
	{
		if (Itr->SnapshotOnBeginPlay)
		{
			Itr->HasInitialized = false;
		}
	}

	cogProvider = NULL;
}

//static
UDynamicObject* UDynamicObject::SetupControllerActor(AActor* target, bool IsRight, EC3DControllerType controllerType)
{
	UDynamicObject* dyn = target->FindComponentByClass<UDynamicObject>();
	if (dyn == NULL)
	{
		auto mcc = target->FindComponentByClass< UMotionControllerComponent>();
		dyn = NewObject<UDynamicObject>(target, UDynamicObject::StaticClass());
		dyn->SetupAttachment(mcc);
		dyn->RegisterComponent();
	}

	dyn->IsRightController = IsRight;

	switch (controllerType)
	{
	case EC3DControllerType::Vive:
		dyn->ControllerType = "vivecontroller";
		dyn->CommonMeshName = ECommonMeshName::ViveController;
		break;
	case EC3DControllerType::Oculus:
		if (IsRight)
		{
			dyn->ControllerType = "oculustouchright";
			dyn->CommonMeshName = ECommonMeshName::OculusRiftTouchRight;
		}
		else
		{
			dyn->ControllerType = "oculustouchleft";
			dyn->CommonMeshName = ECommonMeshName::OculusRiftTouchLeft;
		}
		break;
	case EC3DControllerType::WindowsMixedReality:
		if (IsRight)
		{
			dyn->ControllerType = "windows_mixed_reality_controller_right";
			dyn->CommonMeshName = ECommonMeshName::WindowsMixedRealityRight;
		}
		else
		{
			dyn->ControllerType = "windows_mixed_reality_controller_left";
			dyn->CommonMeshName = ECommonMeshName::WindowsMixedRealityLeft;
		}
		break;
	case EC3DControllerType::PicoNeo2Eye:
		if (IsRight)
		{
			dyn->ControllerType = "pico_neo_2_eye_controller_right";
			dyn->CommonMeshName = ECommonMeshName::PicoNeo2EyeControllerRight;
		}
		else
		{
			dyn->ControllerType = "pico_neo_2_eye_controller_left";
			dyn->CommonMeshName = ECommonMeshName::PicoNeo2EyeControllerLeft;
		}
		break;
	default:
		break;
	}

	dyn->UseCustomMeshName = false;
	dyn->IsController = true;

	dyn->IdSourceType = EIdSourceType::CustomId;
	dyn->CustomId = FGuid::NewGuid().ToString();
	dyn->Initialize();
	return dyn;
}

//static
UDynamicObject* UDynamicObject::SetupControllerDynamic(UDynamicObject* dyn, bool IsRight, EC3DControllerType controllerType)
{
	if (dyn == NULL)
	{
		return NULL;
	}

	dyn->IsRightController = IsRight;

	switch (controllerType)
	{
	case EC3DControllerType::Vive:
		dyn->ControllerType = "vivecontroller";
		dyn->CommonMeshName = ECommonMeshName::ViveController;
		break;
	case EC3DControllerType::Oculus:
		if (IsRight)
		{
			dyn->ControllerType = "oculustouchright";
			dyn->CommonMeshName = ECommonMeshName::OculusRiftTouchRight;
		}
		else
		{
			dyn->ControllerType = "oculustouchleft";
			dyn->CommonMeshName = ECommonMeshName::OculusRiftTouchLeft;
		}
		break;
	case EC3DControllerType::PicoNeo2Eye:
		if (IsRight)
		{
			dyn->ControllerType = "pico_neo_2_eye_controller_right";
			dyn->CommonMeshName = ECommonMeshName::PicoNeo2EyeControllerRight;
		}
		else
		{
			dyn->ControllerType = "pico_neo_2_eye_controller_left";
			dyn->CommonMeshName = ECommonMeshName::PicoNeo2EyeControllerLeft;
		}
		break;
	case EC3DControllerType::WindowsMixedReality:
		if (IsRight)
		{
			dyn->ControllerType = "windows_mixed_reality_controller_right";
			dyn->CommonMeshName = ECommonMeshName::WindowsMixedRealityRight;
		}
		else
		{
			dyn->ControllerType = "windows_mixed_reality_controller_left";
			dyn->CommonMeshName = ECommonMeshName::WindowsMixedRealityLeft;
		}
		break;
	default:
		break;
	}

	dyn->UseCustomMeshName = false;
	dyn->IsController = true;

	dyn->IdSourceType = EIdSourceType::CustomId;
	dyn->CustomId = FGuid::NewGuid().ToString();
	dyn->Initialize();
	return dyn;
}

UDynamicObject* UDynamicObject::SetupControllerMotionController(UMotionControllerComponent* mc, bool IsRight, EC3DControllerType controllerType)
{
	TArray<USceneComponent*> childComponents;
	mc->GetChildrenComponents(true, childComponents);

	UDynamicObject* dyn = NULL;

	for (auto& Elem : childComponents)
	{
		dyn = Cast<UDynamicObject>(Elem);
		if (dyn != NULL)
		{
			break;
		}
	}

	if (dyn == NULL)
	{
		dyn = NewObject<UDynamicObject>(mc->GetOwner(), UDynamicObject::StaticClass());
		dyn->SetupAttachment(mc);
		dyn->RegisterComponent();
	}

	dyn->IsRightController = IsRight;

	switch (controllerType)
	{
	case EC3DControllerType::Vive:
		dyn->ControllerType = "vivecontroller";
		dyn->CommonMeshName = ECommonMeshName::ViveController;
		break;
	case EC3DControllerType::Oculus:
		if (IsRight)
		{
			dyn->ControllerType = "oculustouchright";
			dyn->CommonMeshName = ECommonMeshName::OculusRiftTouchRight;
		}
		else
		{
			dyn->ControllerType = "oculustouchleft";
			dyn->CommonMeshName = ECommonMeshName::OculusRiftTouchLeft;
		}
		break;
	case EC3DControllerType::PicoNeo2Eye:
		if (IsRight)
		{
			dyn->ControllerType = "pico_neo_2_eye_controller_right";
			dyn->CommonMeshName = ECommonMeshName::PicoNeo2EyeControllerRight;
		}
		else
		{
			dyn->ControllerType = "pico_neo_2_eye_controller_left";
			dyn->CommonMeshName = ECommonMeshName::PicoNeo2EyeControllerLeft;
		}
		break;
	case EC3DControllerType::WindowsMixedReality:
		if (IsRight)
		{
			dyn->ControllerType = "windows_mixed_reality_controller_right";
			dyn->CommonMeshName = ECommonMeshName::WindowsMixedRealityRight;
		}
		else
		{
			dyn->ControllerType = "windows_mixed_reality_controller_left";
			dyn->CommonMeshName = ECommonMeshName::WindowsMixedRealityLeft;
		}
		break;
	default:
		break;
	}

	dyn->UseCustomMeshName = false;
	dyn->IsController = true;

	dyn->IdSourceType = EIdSourceType::CustomId;
	dyn->CustomId = FGuid::NewGuid().ToString();
	dyn->Initialize();
	return dyn;
}

void UDynamicObject::FlushButtons(FControllerInputStateCollection& target)
{
	//write an new snapshot and append input state
	if (target.States.Num() == 0)
	{
		return;
	}

	FDynamicObjectSnapshot snap = MakeSnapshot(false);
	snap.Buttons = target.States;
	target.States.Empty();

	snapshots.Add(snap);
	if (snapshots.Num() + newManifest.Num() > MaxSnapshots)
	{
		TrySendData();
	}
}


FDynamicObjectManifestEntry* FDynamicObjectManifestEntry::SetProperty(FString key, FString value)
{
	this->StringProperties.Add(key, value);
	return this;
}