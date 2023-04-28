// Fill out your copyright notice in the Description page of Project Settings.

#include "DynamicObjectManager.h"

UDynamicObjectManager::UDynamicObjectManager()
{

}

void UDynamicObjectManager::Initialize()
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

	cogProvider = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
}

void UDynamicObjectManager::ClearSnapshots()
{
	//when playing in editor, sometimes snapshots will persist in this list
	snapshots.Empty();
}

void UDynamicObjectManager::OnSessionBegin()
{
	auto world = ACognitiveVRActor::GetCognitiveSessionWorld();
	if (world == nullptr)
	{
		GLog->Log("UDynamicObjectManager::StartSession world from ACognitiveVRActor is null!");
		return;
	}
	world->GetTimerManager().SetTimer(AutoSendHandle, FTimerDelegate::CreateRaw(this, &UDynamicObjectManager::SendData, false), AutoTimer, true);
}

//this is used when generating a simple unique id at runtime
TSharedPtr<FDynamicObjectId> UDynamicObjectManager::GetUniqueId(FString meshName)
{
	TSharedPtr<FDynamicObjectId> freeId;
	static int32 originalId = 1000;
	originalId++;

	freeId = MakeShareable(new FDynamicObjectId(FString::FromInt(originalId), meshName));
	return freeId;
}

bool UDynamicObjectManager::HasRegisteredObjectId(FString id)
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
void UDynamicObjectManager::RegisterObjectId(FString MeshName, FString Id, FString ActorName, bool IsController, bool IsRightController, FString ControllerType)
{
	if (Id.IsEmpty())
	{
		return;
	}
	TSharedPtr<FDynamicObjectId> ObjectID = MakeShareable(new FDynamicObjectId(Id, MeshName));
	allObjectIds.Add(ObjectID);
	FDynamicObjectManifestEntry entry = FDynamicObjectManifestEntry(Id, ActorName, MeshName);
	if (IsController)
	{
		entry.ControllerType = ControllerType;
		entry.IsRight = IsRightController;
	}
	manifest.Add(entry);
	newManifest.Add(entry);
}

void UDynamicObjectManager::CacheControllerPointer(UDynamicObject* object, bool isRight)
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

TSharedPtr<FJsonValueObject> UDynamicObjectManager::WriteSnapshotToJson(FDynamicObjectSnapshot snapshot)
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

void UDynamicObjectManager::TrySendData()
{
	bool withinMinTimer = LastSendTime + MinTimer > UCognitiveVRBlueprints::GetSessionDuration();
	bool withinExtremeBatchSize = newManifest.Num() + snapshots.Num() < ExtremeBatchSize;

	if (withinMinTimer && withinExtremeBatchSize)
	{
		return;
	}
	SendData(false);
}

void UDynamicObjectManager::SendData(bool copyDataToCache)
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
		CognitiveLog::Info("UDynamicObjectManager::SendData no objects or data to send!");
		return;
	}

	LastSendTime = UCognitiveVRBlueprints::GetSessionDuration();

	TArray<TSharedPtr<FJsonValueObject>> EventArray = UDynamicObjectManager::DynamicSnapshotsToString();

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
		TSharedPtr<FJsonObject> ManifestObject = UDynamicObjectManager::DynamicObjectManifestToString();
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

TSharedPtr<FJsonObject> UDynamicObjectManager::DynamicObjectManifestToString()
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

TArray<TSharedPtr<FJsonValueObject>> UDynamicObjectManager::DynamicSnapshotsToString()
{
	TArray<TSharedPtr<FJsonValueObject>> dataArray;

	for (int32 i = 0; i != snapshots.Num(); ++i)
	{
		dataArray.Add(UDynamicObjectManager::WriteSnapshotToJson(snapshots[i]));
	}
	return dataArray;
}

void UDynamicObjectManager::OnPreSessionEnd()
{
	//Q: should this delete the session manifest of dynamic objects? or just the outstanding snapshots?
	//A: clean up everything. this uobject will be destroyed as part of the cognitivevrprovider sessionEnd process

	//record snapshots for all dynamic object components
	auto world = ACognitiveVRActor::GetCognitiveSessionWorld();
	if (world == nullptr) { return; }

	for (TObjectIterator<UDynamicObject> Itr; Itr; ++Itr)
	{
		if (Itr->GetWorld() != world) { continue; }
		Itr->MakeSnapshot(false);
	}
	world->GetTimerManager().ClearTimer(AutoSendHandle);
}

//this uobject is getting deleted - should only happen after OnPreSessionEnd
void UDynamicObjectManager::OnPostSessionEnd()
{
	//clean up variables
	allObjectIds.Empty();
	manifest.Empty();
	newManifest.Empty();
	jsonPart = 1;
	callbackInitialized = false;
	cogProvider.Reset();
}

void UDynamicObjectManager::AddSnapshot(FDynamicObjectSnapshot snapshot)
{
	snapshots.Add(snapshot);
	if (snapshots.Num() + newManifest.Num() > MaxSnapshots)
	{
		TrySendData();
	}
}