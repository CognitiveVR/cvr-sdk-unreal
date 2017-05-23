// Fill out your copyright notice in the Description page of Project Settings.

#include "CognitiveVR.h"
#include "DynamicObject.h"
#include "CognitiveVRSettings.h"
#include "Util.h"

TArray<FDynamicObjectSnapshot> snapshots;
TArray<FDynamicObjectManifestEntry> manifest;
TArray<FDynamicObjectManifestEntry> newManifest;
TArray<FDynamicObjectId> allObjectIds;
int32 jsonPart = 1;
int32 MaxSnapshots = 64;

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
		UStaticMeshComponent* sceneComponent = Cast<UStaticMeshComponent>(actorComponent);
		if (sceneComponent == NULL)
		{
			return;
		}
		if (sceneComponent->StaticMesh == NULL)
		{
			return;
		}
		MeshName = sceneComponent->StaticMesh->GetName();
	}
}

void UDynamicObject::BeginPlay()
{
	s = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider();

	LastPosition = GetOwner()->GetActorLocation();
	LastForward = GetOwner()->GetActorForwardVector();

	if (SnapshotOnEnable)
	{
		FDynamicObjectSnapshot initSnapshot = MakeSnapshot();
		initSnapshot.SnapshotProperty("enabled", true);
		snapshots.Add(initSnapshot);

		if (snapshots.Num() + newManifest.Num() > MaxSnapshots)
		{
			SendData();
		}
	}

	Super::BeginPlay();
}

FDynamicObjectSnapshot* FDynamicObjectSnapshot::SnapshotProperty(FString key, FString value)
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
}

FDynamicObjectId UDynamicObject::GetUniqueId(FString meshName)
{
	FDynamicObjectId freeId;
	static int32 originalId = 1000;
	originalId++;

	freeId = FDynamicObjectId(originalId, meshName);
	return freeId;
}

FDynamicObjectId UDynamicObject::GetObjectId()
{
	return ObjectID;
}

void UDynamicObject::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );

	if (!UpdateOnTick) { return; }

	currentTime += DeltaTime;
	if (currentTime > SnapshotInterval)
	{
		currentTime -= SnapshotInterval;

		//if the object has not moved the minimum amount, return

		//write to json

		FVector currentForward = GetOwner()->GetActorForwardVector();

		/*DrawDebugLine(
			GetWorld(),
			GetOwner()->GetActorLocation(),
			GetOwner()->GetActorLocation() + LastForward*100, //this isn't right. 
			FColor(255, 0, 0),
			false, 3, 0,
			3
		);

		DrawDebugLine(
			GetWorld(),
			GetOwner()->GetActorLocation(),
			GetOwner()->GetActorLocation() + GetOwner()->GetActorForwardVector()*100,
			FColor(0, 255, 0),
			false, 3, 0,
			3
		);*/


		//lastRot.Normalize();
		currentForward.Normalize();
		
		float dotRot = FVector::DotProduct(LastForward, currentForward);

		//GLog->Log("rotations  last " + lastRot.ToString() + " currentRot  " + currentRot.ToString());

		//float dotDegrees = (180.f) / PI * FMath::Acos(dotRot);

		//FRotator newRot((LastRotation - GetOwner()->GetActorRotation()) - (GetOwner()->GetActorRotation()));

		float actualDegrees = FMath::Acos(FMath::Clamp<float>(dotRot, -1.0, 1.0)) * 57.29578;
		
		//GLog->Log("dot " + FString::SanitizeFloat(dotRot) +"       "+FString::SanitizeFloat(actualDegrees) + " degrees");

		if ((LastPosition - GetOwner()->GetActorLocation()).Size() > PositionThreshold)
		{
			//GLog->Log("moved " + GetOwner()->GetName());
			//moved
		}
		else if (actualDegrees > RotationThreshold) //rotator stuff
		{
			//GLog->Log("rotated " + GetOwner()->GetName());
			//GLog->Log("rotated "+ FString::SanitizeFloat(dotDegrees) + "   " + GetOwner()->GetName());
			//rotated
		}
		else
		{
			//hasn't moved enough
			return;
		}
		LastPosition = GetOwner()->GetActorLocation();

		LastForward = GetOwner()->GetActorForwardVector();
		
		FDynamicObjectSnapshot snapObj = MakeSnapshot();

		//TODO add properties to the data, especially enabled = true
		//array of objects

		snapshots.Add(snapObj);

		if (snapshots.Num() + newManifest.Num() > MaxSnapshots)
		{
			SendData();
		}
	}
}

FDynamicObjectSnapshot UDynamicObject::MakeSnapshot()
{
	bool needObjectId = false;
	if (ObjectID.Id == -1)
	{
		needObjectId = true;
	}
	else
	{
		FDynamicObjectManifestEntry entry;
		bool foundEntry = false;
		for (int32 i = 0; i < manifest.Num(); i++)
		{
			if (manifest[i].Id == ObjectID.Id)
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
			FDynamicObjectId recycledId;
			bool foundRecycleId = false;
			for (int32 i = 0; i < allObjectIds.Num(); i++)
			{
				if (!allObjectIds[i].Used && allObjectIds[i].MeshName == MeshName)
				{
					foundRecycleId = true;
					recycledId = allObjectIds[i];
					break;
				}
			}
			if (foundRecycleId)
			{
				ObjectID = recycledId;
				ObjectID.Used = true;
			}
			else
			{
				ObjectID = GetUniqueId(MeshName);

				FDynamicObjectManifestEntry entry = FDynamicObjectManifestEntry(ObjectID.Id, GetOwner()->GetName(), MeshName);
				if (!GroupName.IsEmpty())
				{
					//entry.
					entry.SetProperty("groupname", GroupName);
				}
				manifest.Add(entry);
				newManifest.Add(entry);
			}
		}
		else
		{
			ObjectID = FDynamicObjectId(CustomId, MeshName);
			FDynamicObjectManifestEntry entry = FDynamicObjectManifestEntry(ObjectID.Id, GetOwner()->GetName(), MeshName);
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
			GLog->Log("==================Register send data with manifest");
			s->OnSendData.AddStatic(SendData);
		}
	}

	FDynamicObjectSnapshot snapshot = FDynamicObjectSnapshot();

	double ts = Util::GetTimestamp();

	snapshot.time = ts;
	snapshot.id = ObjectID.Id;
	snapshot.position = FVector(-(int32)GetOwner()->GetActorLocation().X, (int32)GetOwner()->GetActorLocation().Z, (int32)GetOwner()->GetActorLocation().Y);
	

	FQuat quat;
	FRotator rot = GetOwner()->GetActorRotation();
	//rot.Pitch += ExtraPitch;
	//rot.Yaw += ExtraYaw;
	//rot.Roll += ExtraRoll;
	quat = rot.Quaternion();
	/*
	if (XPos == 0)
		snapshot.rotation.X = quat.X;
	if (XPos == 1)
		snapshot.rotation.Y = quat.X;
	if (XPos == 2)
		snapshot.rotation.Z = quat.X;
	if (XPos == 3)
		snapshot.rotation.W = quat.X;

	if (YPos == 0)
		snapshot.rotation.X = quat.Y;
	if (YPos == 1)
		snapshot.rotation.Y = quat.Y;
	if (YPos == 2)
		snapshot.rotation.Z = quat.Y;
	if (YPos == 3)
		snapshot.rotation.W = quat.Y;

	if (ZPos == 0)
		snapshot.rotation.X = quat.Z;
	if (ZPos == 1)
		snapshot.rotation.Y = quat.Z;
	if (ZPos == 2)
		snapshot.rotation.Z = quat.Z;
	if (ZPos == 3)
		snapshot.rotation.W = quat.Z;

	if (WPos == 0)
		snapshot.rotation.X = quat.W;
	if (WPos == 1)
		snapshot.rotation.Y = quat.W;
	if (WPos == 2)
		snapshot.rotation.Z = quat.W;
	if (WPos == 3)
		snapshot.rotation.W = quat.W;
	*/
	snapshot.rotation = FQuat(quat.X, quat.Z, quat.Y, quat.W);

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
	for (auto& Elem : snapshot.DoubleProperties)
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

	//TODO only combine 64 entries, prioritizing the manifest
	FString currentSceneName = myworld->GetMapName();
	currentSceneName.RemoveFromStart(myworld->StreamingLevelsPrefix);
	UDynamicObject::SendData(currentSceneName);
}

void UDynamicObject::SendData(FString sceneName)
{
	if (newManifest.Num() + snapshots.Num() == 0)
	{
		CognitiveLog::Info("UDynamicObject::SendData no objects or data to send!");
		return;
	}

	CognitiveLog::Info("UDynamicObject::SendData for dynamics");

	
	
	TArray<TSharedPtr<FJsonValueObject>> EventArray = UDynamicObject::DynamicSnapshotsToString();

	TSharedPtr<FJsonObject>wholeObj = MakeShareable(new FJsonObject);
	
	FAnalyticsProviderCognitiveVR* cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Get();

	wholeObj->SetStringField("userid", cog->GetDeviceID());
	wholeObj->SetNumberField("timestamp", (int32)cog->GetSessionTimestamp());
	wholeObj->SetStringField("sessionId", cog->GetCognitiveSessionID());
	wholeObj->SetNumberField("part", jsonPart);
	jsonPart++;

	

	if (newManifest.Num() > 0)
	{
		TSharedPtr<FJsonObject> ManifestObject = UDynamicObject::DynamicObjectManifestToString();
		wholeObj->SetObjectField("manifest", ManifestObject);
		newManifest.Empty();

		CognitiveLog::Warning("sending new object manifest");
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

FDynamicObjectSnapshot UDynamicObject::NewSnapshot()
{
	FDynamicObjectSnapshot initSnapshot = MakeSnapshot();
	snapshots.Add(initSnapshot);
	return initSnapshot;
}

FDynamicObjectSnapshot UDynamicObject::SnapshotStringProperty(FDynamicObjectSnapshot snapshot, FString key, FString stringValue)
{
	snapshot.SnapshotProperty(key, stringValue);
	return snapshot;
}

FDynamicObjectSnapshot UDynamicObject::SnapshotBoolProperty(FDynamicObjectSnapshot snapshot, FString key, bool boolValue)
{
	snapshot.SnapshotProperty(key, boolValue);
	return snapshot;
}

FDynamicObjectSnapshot UDynamicObject::SnapshotFloatProperty(FDynamicObjectSnapshot snapshot, FString key, float floatValue)
{
	snapshot.SnapshotProperty(key, floatValue);
	return snapshot;
}

FDynamicObjectSnapshot UDynamicObject::SnapshotIntegerProperty(FDynamicObjectSnapshot snapshot, FString key, int32 intValue)
{
	snapshot.SnapshotProperty(key, intValue);
	return snapshot;
}

void UDynamicObject::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (ReleaseIdOnDestroy && !TrackGaze)
	{
		ObjectID.Used = false;
	}

	if (EndPlayReason == EEndPlayReason::EndPlayInEditor)
	{
		//manifest.Empty();
		snapshots.Empty();
	}
}

FDynamicObjectManifestEntry* FDynamicObjectManifestEntry::SetProperty(FString key, FString value)
{
	this->StringProperties.Add(key, value);
	return this;
}