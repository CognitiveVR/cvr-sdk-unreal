/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/

#include "DynamicObject.h"
#include "DynamicIdPoolAsset.h"
#include "DynamicObjectManager.h"
#include "CustomEvent.h"
//#include "Cognitive3DSettings.h"
//#include "Util.h"

UDynamicObject::UDynamicObject()
{
	PrimaryComponentTick.bCanEverTick = true;
}

#if WITH_EDITOR
bool UDynamicObject::Modify(bool alwaysMarkDirty)
{
	if (GetOwner() != NULL)
	{
		if (GetOwner()->IsUnreachable())
		{
			return Super::Modify(alwaysMarkDirty);
		}
		TryGenerateMeshName();
		TryGenerateCustomId();
		SetUniqueDynamicIds();
	}
	return Super::Modify(true);
}

void UDynamicObject::OnRegister()
{
	Super::OnRegister();

	const UClass* ActorClass = GetOwner()->GetClass();
	check(ActorClass != nullptr);

	UBlueprint* bp = UBlueprint::GetBlueprintFromClass(ActorClass);
	
	if (bp != nullptr) //class has a blueprint - don't overwrite the dynamic object custom id
	{
		return;
	}

	if (GetOwner() != NULL)
	{
		if (GetOwner()->IsUnreachable())
		{
			return;
		}
		TryGenerateMeshName();
		TryGenerateCustomId();
		SetUniqueDynamicIds();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("DynamicObject component does not have a valid parent. This could be caused by incorrectly creating and registering the Dynamic Object component before attaching it to the parent actor. Pleasure ensure that you are calling AttachToParent before RegisterComponent when assigning controller Dynamic Objects with C++"));
	}
}
#endif

void UDynamicObject::SetUniqueDynamicIds()
{
	if (GWorld == NULL) { return; }

	//loop thorugh all dynamics in the scene
	TArray<UDynamicObject*> dynamics;

	//make a list of all the used objectids

	TArray<FDynamicObjectId> usedIds;

	//get all the dynamic objects in the scene
	for (TActorIterator<AActor> ActorItr(GWorld); ActorItr; ++ActorItr)
	{
		// Same as with the Object Iterator, access the subclass instance with the * or -> operators.
		//AStaticMeshActor *Mesh = *ActorItr;

		UActorComponent* actorComponent = (*ActorItr)->GetComponentByClass(UDynamicObject::StaticClass());
		if (actorComponent == NULL)
		{
			continue;
		}
		UDynamicObject* dynamic = Cast<UDynamicObject>(actorComponent);
		if (dynamic == NULL)
		{
			continue;
		}
		dynamics.Add(dynamic);
	}

	int32 changedDynamics = 0;

	//unassigned or invalid numbers
	TArray<UDynamicObject*> UnassignedDynamics;

	//try to put all ids back where they were
	for (auto& dynamic : dynamics)
	{
		//id dynamic custom id is not in usedids - add it

		if (dynamic->MeshName.IsEmpty())
		{
			dynamic->TryGenerateMeshName();
		}
		FString findId = dynamic->CustomId;

		FDynamicObjectId* FoundId = usedIds.FindByPredicate([findId](const FDynamicObjectId& InItem)
			{
				return InItem.Id == findId;
			});

		if (FoundId == NULL && dynamic->CustomId != "")
		{
			usedIds.Add(FDynamicObjectId(dynamic->CustomId, dynamic->MeshName));
		}
		else
		{
			//assign a new and unused id
			//GLog->Log("UDynamicObject::SetUniqueDynamicIds found Duplicate or Invalid Id: " + dynamic->CustomId);
			UnassignedDynamics.Add(dynamic);
		}
	}

	for (auto& dynamic : UnassignedDynamics)
	{
		dynamic->CustomId = FGuid::NewGuid().ToString();
		dynamic->IdSourceType = EIdSourceType::CustomId;
		usedIds.Add(FDynamicObjectId(dynamic->CustomId, dynamic->MeshName));
		changedDynamics++;
	}
}

//utility used in editor
void UDynamicObject::TryGenerateMeshName()
{
	if (GetOwner() == NULL || GetAttachParent() == NULL)
	{
		return;
	}

	if (MeshName.IsEmpty())
	{
		if (GetAttachParent()->IsA<UStaticMeshComponent>())
		{
			USceneComponent* actorComponent = Cast<USceneComponent>(GetAttachParent());
			if (actorComponent != NULL)
			{
				UStaticMeshComponent* staticmeshComponent = Cast<UStaticMeshComponent>(actorComponent);
				if (staticmeshComponent != NULL && staticmeshComponent->GetStaticMesh() != NULL)
				{
					MeshName = staticmeshComponent->GetStaticMesh()->GetName();
					return;
				}
			}
		}
		
		if (GetAttachParent()->IsA<USkeletalMeshComponent>())
		{
			USceneComponent* actorSkeletalComponent = Cast<USceneComponent>(GetAttachParent());
			if (actorSkeletalComponent != NULL)
			{
				USkeletalMeshComponent* skeletalmeshComponent = Cast<USkeletalMeshComponent>(actorSkeletalComponent);
				if (skeletalmeshComponent != NULL && skeletalmeshComponent->SkeletalMesh != NULL)
				{
					MeshName = skeletalmeshComponent->SkeletalMesh->GetName();
					return;
				}
			}
		}
		
	}
}

//utility used in editor
void UDynamicObject::TryGenerateCustomId()
{
	if (GetOwner() == NULL)
	{
		return;
	}
	if (CustomId.IsEmpty() && IdSourceType == EIdSourceType::CustomId)
	{
		CustomId = FGuid::NewGuid().ToString();
	}
}

void UDynamicObject::BeginPlay()
{
	Super::BeginPlay();

	cogProvider = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (cogProvider->HasStartedSession())
	{
		Initialize();
	}

	//listen for session begin delegate. cognitive provider persists, so add/remove delegate on normal unreal begin/end play lifecycle
	//each time OnSessionBegin is broadcast, run through the startup process

	cogProvider->OnSessionBegin.AddDynamic(this, &UDynamicObject::Initialize);
	cogProvider->OnPostSessionEnd.AddDynamic(this, &UDynamicObject::OnPostSessionEnd);
	cogProvider->OnPreSessionEnd.AddDynamic(this, &UDynamicObject::OnPreSessionEnd);
	if (AActor* OwnerActor = GetOwner())
	{
		OwnerActor->OnDestroyed.AddDynamic(this, &UDynamicObject::HandleOwnerDestroyed);
	}

	PreviousRotation = GetOwner()->GetActorRotation();
}

void UDynamicObject::OnPreSessionEnd()
{
	CleanupDynamicObject();
	cogProvider->OnCognitiveInterval.RemoveDynamic(this, &UDynamicObject::UpdateSyncWithPlayer);
}

//reset so this dynamic can be re-registered in the next session (which might happen in the same game instance)
void UDynamicObject::OnPostSessionEnd()
{
	HasInitialized = false;
}

void UDynamicObject::Initialize()
{
	if (HasInitialized)
	{
		return;
	}
	
	if (cogProvider == nullptr) { return; }

	dynamicObjectManager = cogProvider->dynamicObjectManager;

	if (dynamicObjectManager == nullptr) { return; }

	//even if session has not started, still collect data
	//session must be started to send
	//scene id must be valid to send

	//scene component
	if (GetAttachParent() != nullptr)
	{
		LastPosition = GetAttachParent()->GetComponentLocation();
		LastRotation = GetAttachParent()->GetComponentRotation().Quaternion();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("DynamicObject component does not have a valid parent. This could be caused by incorrectly creating and registering the Dynamic Object component before attaching it to the parent actor. Pleasure ensure that you are calling AttachToParent before RegisterComponent when assigning controller Dynamic Objects with C++"));
	}
	LastScale = FVector(1, 1, 1);

	if (IsController)
	{
		switch (ControllerType)
		{
		case EC3DControllerType::Vive:
			ControllerInputImageName = "vivecontroller";
			MeshName = "vivecontroller";
			break;
		case EC3DControllerType::OculusRift:
			if (IsRightController)
			{
				ControllerInputImageName = "oculustouchright";
				MeshName = "oculustouchright";
			}
			else
			{
				ControllerInputImageName = "oculustouchleft";
				MeshName = "oculustouchleft";
			}
			break;
		case EC3DControllerType::Quest2:
			if (IsRightController)
			{
				ControllerInputImageName = "oculusquesttouchright";
				MeshName = "OculusQuestTouchRight";
			}
			else
			{
				ControllerInputImageName = "oculusquesttouchleft";
				MeshName = "OculusQuestTouchLeft";
			}
			break;
		case EC3DControllerType::Quest3:
			if (IsRightController)
			{
				ControllerInputImageName = "quest_plus_touch_right";
				MeshName = "QuestPlusTouchRight";
			}
			else
			{
				ControllerInputImageName = "quest_plus_touch_left";
				MeshName = "QuestPlusTouchLeft";
			}
			break;
		case EC3DControllerType::QuestPro:
			if (IsRightController)
			{
				ControllerInputImageName = "quest_pro_touch_right";
				MeshName = "QuestProTouchRight";
			}
			else
			{
				ControllerInputImageName = "quest_pro_touch_left";
				MeshName = "QuestProTouchLeft";
			}
			break;
		case EC3DControllerType::WindowsMixedReality:
			if (IsRightController)
			{
				ControllerInputImageName = "windows_mixed_reality_controller_right";
				MeshName = "windows_mixed_reality_controller_right";
			}
			else
			{
				ControllerInputImageName = "windows_mixed_reality_controller_left";
				MeshName = "windows_mixed_reality_controller_left";
			}
			break;
		case EC3DControllerType::PicoNeo2:
			if (IsRightController)
			{
				ControllerInputImageName = "pico_neo_2_eye_controller_right";
				MeshName = "pico_neo_2_eye_controller_right";
			}
			else
			{
				ControllerInputImageName = "pico_neo_2_eye_controller_left";
				MeshName = "pico_neo_2_eye_controller_left";
			}
			break;
		case EC3DControllerType::PicoNeo3:
			if (IsRightController)
			{
				ControllerInputImageName = "pico_neo_3_eye_controller_right";
				MeshName = "pico_neo_3_eye_controller_right";
			}
			else
			{
				ControllerInputImageName = "pico_neo_3_eye_controller_left";
				MeshName = "pico_neo_3_eye_controller_left";
			}
			break;
		case EC3DControllerType::PicoNeo4:
			if (IsRightController)
			{
				ControllerInputImageName = "pico_neo_4_enterprise_controller_right";
				MeshName = "pico_neo_4_enterprise_controller_right";
			}
			else
			{
				ControllerInputImageName = "pico_neo_4_enterprise_controller_left";
				MeshName = "pico_neo_4_enterprise_controller_left";
			}
			break;
		default:
			if (IsRightController)
			{
				ControllerInputImageName = "oculusquesttouchright";
				MeshName = "OculusQuestTouchRight";
			}
			else
			{
				ControllerInputImageName = "oculusquesttouchleft";
				MeshName = "OculusQuestTouchLeft";
			}
			break;
		}
	}

	ValidateObjectId();
	dynamicObjectManager->RegisterObjectId(MeshName, ObjectID->Id, GetOwner()->GetName(),IsController,IsRightController, ControllerInputImageName);

	if (IsController)
	{
		dynamicObjectManager->CacheControllerPointer(this, IsRightController);
	}

	bool hasScaleChanged = true;
	if (GetAttachParent() != nullptr)
	{
		if (FMath::Abs(LastScale.Size() - GetAttachParent()->GetComponentTransform().GetScale3D().Size()) > ScaleThreshold)
		{
			hasScaleChanged = true;
		}
	}
	FDynamicObjectSnapshot initSnapshot = MakeSnapshot(hasScaleChanged);
	SnapshotBoolProperty(initSnapshot, "enabled", true);
	dynamicObjectManager->AddSnapshot(initSnapshot);
	HasInitialized = true;
	if (SyncUpdateWithPlayer)
	{
		cogProvider->OnCognitiveInterval.AddDynamic(this, &UDynamicObject::UpdateSyncWithPlayer);
	}
}

TSharedPtr<FDynamicObjectId> UDynamicObject::GetObjectId()
{
	return ObjectID;
}

void UDynamicObject::UpdateSyncWithPlayer()
{
	if (dynamicObjectManager == nullptr) { return; }
	if (cogProvider->CurrentTrackingSceneId.IsEmpty()) { return; }

	FRotator parentRotator;
	//rotation angle to degrees
	if (GetAttachParent() != nullptr)
	{
		parentRotator = GetAttachParent()->GetComponentRotation();
	}
	
	float quatDot = parentRotator.Quaternion() | LastRotation;
	quatDot = FMath::Abs(quatDot);
	quatDot = FMath::Min(quatDot, 1.0f);
	float quatDegrees = FMath::RadiansToDegrees(FMath::Acos(quatDot)) * 2;
	bool hasScaleChanged = false;

	if (GetAttachParent() != nullptr)
	{

		if (FMath::Abs(LastScale.Size() - GetAttachParent()->GetComponentTransform().GetScale3D().Size()) > ScaleThreshold)
		{
			hasScaleChanged = true;
		}
	}

	if (!hasScaleChanged)
	{
		if (GetAttachParent() != nullptr)
		{
			if ((LastPosition - GetAttachParent()->GetComponentLocation()).Size() > PositionThreshold)
			{
				//moved
			}
		}
		else if (quatDegrees > RotationThreshold) //rotator stuff
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
	if (GetAttachParent() != nullptr)
	{
		LastPosition = GetAttachParent()->GetComponentLocation();
		LastRotation = GetAttachParent()->GetComponentRotation().Quaternion();
		if (hasScaleChanged)
		{
			LastScale = GetAttachParent()->GetComponentScale();
		}
	}

	FDynamicObjectSnapshot snapObj = MakeSnapshot(hasScaleChanged);
	dynamicObjectManager->AddSnapshot(snapObj);
}

void UDynamicObject::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );

	if (!HasInitialized) { return; }
	if (dynamicObjectManager == nullptr) { return; }
	if (cogProvider->CurrentTrackingSceneId.IsEmpty()) { return; }

	if (SyncUpdateWithPlayer){return;}

	currentTime += DeltaTime;

	if (currentTime > ControllerUpdateInterval)
	{
		
		if (IsController)
			currentTime -= ControllerUpdateInterval;
		else
			currentTime -= DynamicUpdateInterval;

		//rotation angle to degrees
		FRotator parentRotator;
		if (GetAttachParent() != nullptr)
		{

			parentRotator = GetAttachParent()->GetComponentRotation();
		}
		float quatDot = parentRotator.Quaternion() | LastRotation;
		quatDot = FMath::Abs(quatDot);
		quatDot = FMath::Min(quatDot, 1.0f);
		float quatDegrees = FMath::RadiansToDegrees(FMath::Acos(quatDot)) * 2;
		bool hasScaleChanged = false;

		if (GetAttachParent() != nullptr)
		{
			if (FMath::Abs(LastScale.Size() - GetAttachParent()->GetComponentTransform().GetScale3D().Size()) > ScaleThreshold)
			{
				hasScaleChanged = true;
			}
		}

		if (!hasScaleChanged)
		{
			if (GetAttachParent() != nullptr)
			{
				if ((LastPosition - GetAttachParent()->GetComponentLocation()).Size() > PositionThreshold)
				{
					//moved
				}
			}
			else if (quatDegrees > RotationThreshold) //rotator stuff
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
		if (GetAttachParent() != nullptr)
		{
			LastPosition = GetAttachParent()->GetComponentLocation();
			LastRotation = GetAttachParent()->GetComponentRotation().Quaternion();
			if (hasScaleChanged)
			{
				LastScale = GetAttachParent()->GetComponentScale();
			}
		}

		FDynamicObjectSnapshot snapObj = MakeSnapshot(hasScaleChanged);
		dynamicObjectManager->AddSnapshot(snapObj);
		
	}
}

void UDynamicObject::ValidateObjectId()
{
	//skip if id is already valid
	if (ObjectID.IsValid() && !ObjectID->Id.IsEmpty())
	{
		return;
	}

	//if id pool isn't set up correctly, fall back to generating an id from a guid
	if (IdSourceType == EIdSourceType::PoolId && IDPool == NULL)
	{
		IdSourceType = EIdSourceType::GeneratedId;
	}

	if (IdSourceType == EIdSourceType::PoolId)
	{
		FString id;
		IDPool->GetId(id);
		ObjectID = MakeShareable(new FDynamicObjectId(id, MeshName));
		CustomId = "";
	}
	else if (IdSourceType == EIdSourceType::CustomId)
	{
		ObjectID = MakeShareable(new FDynamicObjectId(CustomId, MeshName));
	}
	else if (IdSourceType == EIdSourceType::GeneratedId)
	{
		//see if there's a free id instance in dynamic manager. otherwise generate a new unique id
		ObjectID = dynamicObjectManager->GetUniqueObjectId(MeshName);
	}
}

FDynamicObjectSnapshot UDynamicObject::MakeSnapshot(bool hasChangedScale)
{
	//decide if the object needs a new entry in the manifest
	bool needObjectId = false;
	if (!ObjectID.IsValid() || ObjectID->Id.IsEmpty())
	{
		needObjectId = true;
	}
	else
	{
		//check that this objectid is in the manifest
		if (dynamicObjectManager->HasRegisteredObjectId(ObjectID->Id) == false)
		{
			needObjectId = true;
		}
	}

	if (needObjectId)
	{
		ValidateObjectId();
		dynamicObjectManager->RegisterObjectId(MeshName, GetObjectId()->Id, GetOwner()->GetName(), IsController, IsRightController, ControllerInputImageName);
	}

	FDynamicObjectSnapshot snapshot = FDynamicObjectSnapshot();

	double ts = FUtil::GetTimestamp();

	snapshot.timestamp = ts;
	snapshot.id = ObjectID->Id;
	if (GetAttachParent() != nullptr)
	{
		snapshot.position = FVector(-GetAttachParent()->GetComponentLocation().X, GetAttachParent()->GetComponentLocation().Z, GetAttachParent()->GetComponentLocation().Y);
	}

	if (hasChangedScale)
	{
		snapshot.hasScaleChanged = true;
		if (GetAttachParent() != nullptr)
		{
			snapshot.scale = GetAttachParent()->GetComponentScale();
		}
	}

	FQuat quat;

	FRotator rot;

	if (GetAttachParent() != nullptr)
	{
		rot = GetAttachParent()->GetComponentRotation();
    //we check if the snapshot if that of a controller
    //that way we can add a small rotational offset to correct how controllers looks on SceneExplorer
    if (IsController)
    {
      if (IsRightController)
      {
        rot.Yaw -= 45.f;
      }
      else
      {
        rot.Yaw += 45.f;
      }
      rot.Normalize();
    }
	}
	
	quat = rot.Quaternion();

	snapshot.rotation = FQuat(quat.X, quat.Z, quat.Y, quat.W);

	return snapshot;
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
	if (dynamicObjectManager == nullptr) { return; }
	dynamicObjectManager->AddSnapshot(snapshot);
}

void UDynamicObject::BeginEngagement(UDynamicObject* target, FString engagementName, FString UniqueEngagementId)
{
	if (target != nullptr)
	{
		target->ValidateObjectId();
		target->BeginEngagementId(target->ObjectID->Id, engagementName, UniqueEngagementId);
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
		if (GetAttachParent() != nullptr)
		{
			Engagements[UniqueEngagementId]->SetPosition(GetAttachParent()->GetComponentLocation());
		}
		Engagements[UniqueEngagementId]->Send();
		Engagements[UniqueEngagementId] = ce;
	}
}

void UDynamicObject::EndEngagement(UDynamicObject* target, FString engagementName, FString UniqueEngagementId)
{
	if (target != nullptr)
	{
		target->ValidateObjectId();
		target->EndEngagementId(target->ObjectID->Id, engagementName, UniqueEngagementId);
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
		if (GetAttachParent() != nullptr)
		{
			Engagements[UniqueEngagementId]->SetPosition(FVector(-GetAttachParent()->GetComponentLocation().X, GetAttachParent()->GetComponentLocation().Z, GetAttachParent()->GetComponentLocation().Y));\
		}
		Engagements[UniqueEngagementId]->Send();
		Engagements.Remove(UniqueEngagementId);
	}
	else
	{
		//start and end event
		UCustomEvent* ce = NewObject<UCustomEvent>(this);
		ce->SetCategory(engagementName);
		ce->SetDynamicObject(parentDynamicObjectId);
		if (GetAttachParent() != nullptr)
		{
			ce->SetPosition(FVector(-GetAttachParent()->GetComponentLocation().X, GetAttachParent()->GetComponentLocation().Z, GetAttachParent()->GetComponentLocation().Y));
		}
		ce->Send();
	}
}

void UDynamicObject::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	//only write ending snapshot if component is destroyed during gameplay
	bool shouldWriteEndSnapshot = false;
	switch (EndPlayReason)
	{
	case EEndPlayReason::Destroyed:
		shouldWriteEndSnapshot = true;
		//this should normally never be destroyed. 4.19 bug - this is called instead of level transition
		//cog->FlushEvents();
		break;
	case EEndPlayReason::EndPlayInEditor:
		break;
	case EEndPlayReason::LevelTransition:
		shouldWriteEndSnapshot = true;
		break;
	case EEndPlayReason::Quit:
		break;
	case EEndPlayReason::RemovedFromWorld:
		//removed from a sublevel unloading
		shouldWriteEndSnapshot = true;
		break;
	default:
		break;
	}

	if (!shouldWriteEndSnapshot) { return; }
	CleanupDynamicObject();
}

//also called from cognitive actor broadcast end session
void UDynamicObject::CleanupDynamicObject()
{
	if (IdSourceType == EIdSourceType::PoolId && HasValidPoolId && IDPool != NULL)
	{
		if (ObjectID.IsValid() && !ObjectID->Id.IsEmpty())
		{
			IDPool->ReturnId(ObjectID->Id);
			HasValidPoolId = false;
		}
	}

	cogProvider->OnSessionBegin.RemoveDynamic(this, &UDynamicObject::Initialize);
	cogProvider->OnPostSessionEnd.RemoveDynamic(this, &UDynamicObject::OnPostSessionEnd);
	cogProvider->OnPreSessionEnd.RemoveDynamic(this, &UDynamicObject::OnPreSessionEnd);
	if (GetOwner() != NULL)
	{
		GetOwner()->OnDestroyed.RemoveDynamic(this, &UDynamicObject::HandleOwnerDestroyed);
	}

	HasInitialized = false;

	if (dynamicObjectManager == nullptr) { return; }

	FDynamicObjectSnapshot initSnapshot = MakeSnapshot(false);
	SnapshotBoolProperty(initSnapshot, "enabled", false);
	dynamicObjectManager->AddSnapshot(initSnapshot);

	//allow reusing generated dynamic ids (controllers, transient objects for SE only)
	if (IdSourceType == EIdSourceType::GeneratedId)
	{
		ObjectID->Used = false;
		dynamicObjectManager->UnregisterId(ObjectID->Id);
	}

	//go through all engagements and send any that match this objectid
	for (auto& Elem : Engagements)
	{
		if (!ObjectID.IsValid() || ObjectID->Id.IsEmpty())
		{
			continue;
		}
		if (Elem.Value->GetDynamicId() == ObjectID->Id)
		{
			if (GetAttachParent() != nullptr)
			{
				Elem.Value->SetPosition(FVector(-GetAttachParent()->GetComponentLocation().X, GetAttachParent()->GetComponentLocation().Z, GetAttachParent()->GetComponentLocation().Y));
			}
			Elem.Value->Send();
		}
	}
}

void UDynamicObject::HandleOwnerDestroyed(AActor* DestroyedActor)
{
	//send snapshot with "enabled" set to false
	CleanupDynamicObject();
}

//called from optional input tracker to serialize button states
void UDynamicObject::FlushButtons(FControllerInputStateCollection& target)
{
	//write an new snapshot and append input state
	if (target.States.Num() == 0)
	{
		return;
	}
	if (dynamicObjectManager == NULL)
	{
		return;
	}
	
	if (cogProvider->CurrentTrackingSceneId.IsEmpty()) { return; }

	FDynamicObjectSnapshot snap = MakeSnapshot(false);
	snap.Buttons = target.States;
	target.States.Empty();

	dynamicObjectManager->AddSnapshot(snap);
}


FDynamicObjectManifestEntry* FDynamicObjectManifestEntry::SetProperty(FString key, FString value)
{
	this->StringProperties.Add(key, value);
	return this;
}