/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/

#include "DynamicObjectManager.h"
#include "Cognitive3D/Private/C3DNetwork/Network.h"
#include "Cognitive3D/Public/Cognitive3DBlueprints.h"
#include "Cognitive3D/Public/Cognitive3DActor.h"
#include "Cognitive3D/Public/DynamicObject.h"

FDynamicObjectManager::FDynamicObjectManager()
{
	cogProvider = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
}

void FDynamicObjectManager::OnSessionBegin()
{
	//should all these be reset? will dynamic object components correctly re-add themselves?
	//do they manage their registration outside of sessions?

	snapshots.Empty();
	manifest.Empty();
	newManifest.Empty();
	allObjectIds.Empty();
	jsonPart = 1;

	MaxSnapshots = 64;
	FString ValueReceived;

	ValueReceived = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/Cognitive3D.Cognitive3DSettings", "DynamicDataLimit", false);
	if (ValueReceived.Len() > 0)
	{
		int32 dynamicLimit = FCString::Atoi(*ValueReceived);
		if (dynamicLimit > 0)
		{
			MaxSnapshots = dynamicLimit;
		}
	}

	ValueReceived = FAnalytics::Get().GetConfigValueFromIni(GEngineIni, "/Script/Cognitive3D.Cognitive3DSettings", "DynamicAutoTimer", false);
	if (ValueReceived.Len() > 0)
	{
		int32 parsedValue = FCString::Atoi(*ValueReceived);
		if (parsedValue > 0)
		{
			AutoTimer = parsedValue;
		}
	}

	auto world = ACognitive3DActor::GetCognitiveSessionWorld();
	if (world == nullptr)
	{
		GLog->Log("FDynamicObjectManager::StartSession world from ACognitive3DActor is null!");
		return;
	}
	world->GetTimerManager().SetTimer(AutoSendHandle, FTimerDelegate::CreateRaw(this, &FDynamicObjectManager::SendData, false), AutoTimer, true);
}

//this is used when generating a simple unique id at runtime
TSharedPtr<FDynamicObjectId> FDynamicObjectManager::GetUniqueId(FString meshName)
{
	TSharedPtr<FDynamicObjectId> freeId;
	static int32 originalId = 1000;
	originalId++;

	freeId = MakeShareable(new FDynamicObjectId(FString::FromInt(originalId), meshName));
	return freeId;
}

void FDynamicObjectManager::UnregisterId(const FString id)
{
	for (int32 i = 0; i < allObjectIds.Num(); i++)
	{
		if (allObjectIds[i].IsValid() == false) { continue; }
		if (allObjectIds[i]->Id != id) { continue; }

		allObjectIds[i]->Used = false;
		break;
	}
}

TSharedPtr<FDynamicObjectId> FDynamicObjectManager::GetUniqueObjectId(const FString meshName)
{
	//ObjectId from DynamicObject and ObjectId in allObjectIds aren't pointing to the same object
	//look for an unused dynamic object id instance
	for (int32 i = 0; i < allObjectIds.Num(); i++)
	{
		if (allObjectIds[i].IsValid() == false) { continue; }
		if (allObjectIds[i]->Used == true) { continue; }
		if (allObjectIds[i]->MeshName != meshName) { continue; }

		allObjectIds[i]->Used = true;
		return allObjectIds[i];
	}

	//create a new dynamic object id instance. it will be registered into allObjectIds when the dynamic calls RegisterObjectId
	TSharedPtr<FDynamicObjectId> freeId;
	static int32 originalId = 1000;
	originalId++;
	freeId = MakeShareable(new FDynamicObjectId(FString::FromInt(originalId), meshName));
	return freeId;
}


bool FDynamicObjectManager::HasRegisteredObjectId(FString id)
{
	FDynamicObjectManifestEntry entry;
	bool foundEntry = false;
	for (int32 i = 0; i < manifest.Num(); i++)
	{
		if (manifest[i].Id == id)
		{
			foundEntry = true;
			break;
		}
	}
	return foundEntry;
}

//should be rewritten to return an objectid and take some arguments
void FDynamicObjectManager::RegisterObjectId(FString MeshName, FString Id, FString ActorName, bool IsController, bool IsRightController, FString ControllerType)
{
	if (Id.IsEmpty())
	{
		return;
	}

	//check if object with Id already existst in allObjectIds. if so, don't add it to the list again
	bool containsId = false;
	for (int32 i = 0; i < allObjectIds.Num(); i++)
	{
		if (!allObjectIds[i].IsValid())
		{
			//temporary fix. see T-8820 for details
			continue;
		}
		if (allObjectIds[i]->Id == Id)
		{
			containsId = true;
			break;
		}
	}
	if (containsId == false)
	{
		TSharedPtr<FDynamicObjectId> ObjectID = MakeShareable(new FDynamicObjectId(Id, MeshName));
		allObjectIds.Add(ObjectID);
	}

	FDynamicObjectManifestEntry entry = FDynamicObjectManifestEntry(Id, ActorName, MeshName);
	if (IsController)
	{
		entry.ControllerType = ControllerType;
		entry.IsRight = IsRightController;
	}
	manifest.Add(entry);
	newManifest.Add(entry);
}

void FDynamicObjectManager::CacheControllerPointer(UDynamicObject* object, bool isRight)
{
	if (isRight)
	{
		RightHandController = object;
	}
	else
	{
		LeftHandController = object;
	}
}

TSharedPtr<FJsonValueObject> FDynamicObjectManager::WriteSnapshotToJson(FDynamicObjectSnapshot snapshot)
{
	TSharedPtr<FJsonObject>snapObj = MakeShareable(new FJsonObject);

	//id
	snapObj->SetStringField("id", snapshot.id);

	//time
	snapObj->SetNumberField("time", snapshot.timestamp);

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
	return MakeShareable(new FJsonValueObject(snapObj));
}

void FDynamicObjectManager::SendData(bool copyDataToCache)
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
		return;
	}

	LastSendTime = UCognitive3DBlueprints::GetSessionDuration();

	TArray<TSharedPtr<FJsonValueObject>> EventArray = FDynamicObjectManager::DynamicSnapshotsToString();

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
		TSharedPtr<FJsonObject> ManifestObject = FDynamicObjectManager::DynamicObjectManifestToString();
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
	UE_LOG(LogTemp, Warning, TEXT("calling dynamics network call"));
	cogProvider->network->NetworkCall("dynamics", OutputString, copyDataToCache);

	snapshots.Empty();
}

TArray<FDynamicObjectManifestEntry> FDynamicObjectManager::GetDynamicsManifest()
{
	return manifest;
}

TSharedPtr<FJsonObject> FDynamicObjectManager::DynamicObjectManifestToString()
{
	TSharedPtr<FJsonObject> manifestObject = MakeShareable(new FJsonObject);

	for (int32 i = 0; i != newManifest.Num(); ++i)
	{
		TSharedPtr<FJsonObject>entry = MakeShareable(new FJsonObject);
		entry->SetStringField("name", newManifest[i].Name);
		entry->SetStringField("mesh", newManifest[i].MeshName);
		entry->SetStringField("fileType", "gltf");
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

TArray<TSharedPtr<FJsonValueObject>> FDynamicObjectManager::DynamicSnapshotsToString()
{
	TArray<TSharedPtr<FJsonValueObject>> dataArray;

	for (int32 i = 0; i != snapshots.Num(); ++i)
	{
		dataArray.Add(FDynamicObjectManager::WriteSnapshotToJson(snapshots[i]));
	}
	return dataArray;
}

void FDynamicObjectManager::OnPreSessionEnd()
{
	//Q: should this delete the session manifest of dynamic objects? or just the outstanding snapshots?
	//A: clean up everything. this uobject will be destroyed as part of the Cognitive3Dprovider sessionEnd process

	//record snapshots for all dynamic object components
	auto world = ACognitive3DActor::GetCognitiveSessionWorld();
	if (world == nullptr) { return; }

	for (TObjectIterator<UDynamicObject> Itr; Itr; ++Itr)
	{
		if (Itr->GetWorld() != world) { continue; }
		Itr->MakeSnapshot(false);
	}
	world->GetTimerManager().ClearTimer(AutoSendHandle);
}

void FDynamicObjectManager::OnPostSessionEnd()
{
	//clean up variables
	allObjectIds.Empty();
	manifest.Empty();
	newManifest.Empty();
	jsonPart = 1;
}

void FDynamicObjectManager::AddSnapshot(FDynamicObjectSnapshot snapshot)
{
	snapshots.Add(snapshot);
	if (snapshots.Num() + newManifest.Num() > MaxSnapshots)
	{
		SendData(false);
	}
}