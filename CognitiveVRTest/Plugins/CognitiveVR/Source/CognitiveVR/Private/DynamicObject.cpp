// Fill out your copyright notice in the Description page of Project Settings.

#include "DynamicObject.h"
#include "DynamicIdPoolAsset.h"
#include "DynamicObjectManager.h"
#include "CustomEvent.h"
//#include "CognitiveVRSettings.h"
//#include "Util.h"

UDynamicObject::UDynamicObject()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UDynamicObject::OnComponentCreated()
{
	Super::OnComponentCreated();
	TryGenerateCustomIdAndMesh();
}

//utility used in editor
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

//utility used in editor
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
		}
	}
	if (CustomId.IsEmpty())
	{
		CustomId = FGuid::NewGuid().ToString();
	}
}

void UDynamicObject::BeginPlay()
{
	Super::BeginPlay();

	TSharedPtr<FAnalyticsProviderCognitiveVR> cogProvider = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	if (cogProvider->HasStartedSession())
	{
		Initialize();
	}

	//listen for session begin delegate. cognitive provider persists, so add/remove delegate on normal unreal begin/end play lifecycle
	//each time OnSessionBegin is broadcast, run through the startup process

	cogProvider->OnSessionBegin.AddDynamic(this, &UDynamicObject::Initialize);
	cogProvider->OnPostSessionEnd.AddDynamic(this, &UDynamicObject::OnPostSessionEnd);
	cogProvider->OnPreSessionEnd.AddDynamic(this, &UDynamicObject::OnPreSessionEnd);
}

void UDynamicObject::OnPreSessionEnd()
{
	CleanupDynamicObject();
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

	TSharedPtr<FAnalyticsProviderCognitiveVR> cogProvider = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	dynamicObjectManager = cogProvider->dynamicObjectManager;

	if (dynamicObjectManager == nullptr) { return; }

	//even if session has not started, still collect data
	//session must be started to send
	//scene id must be valid to send

	//scene component
	LastPosition = GetComponentLocation();
	LastForward = GetComponentTransform().TransformVector(FVector::ForwardVector);
	LastScale = FVector(1, 1, 1);

	if (IsController)
	{
		switch (ControllerType)
		{
		case EC3DControllerType::Vive:
			ControllerInputImageName = "vivecontroller";
			MeshName = "vivecontroller";
			break;
		case EC3DControllerType::Oculus:
			if (IsRightController)
			{
				ControllerInputImageName = "oculustouchright";
				MeshName = "oculustouchleft";
			}
			else
			{
				ControllerInputImageName = "oculustouchleft";
				MeshName = "oculustouchright";
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
		case EC3DControllerType::PicoNeo2Eye:
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
		default:
			break;
		}
	}

	if (IdSourceType == EIdSourceType::PoolId)
	{
		FString id;
		IDPool->GetId(id);
		ObjectID = MakeShareable(new FDynamicObjectId(id, MeshName));
	}
	else if (IdSourceType == EIdSourceType::CustomId)
	{
		ObjectID = MakeShareable(new FDynamicObjectId(CustomId, MeshName));
	}
	else if (IdSourceType == EIdSourceType::GeneratedId)
	{
		FString generatedId = FGuid::NewGuid().ToString();
		ObjectID = MakeShareable(new FDynamicObjectId(generatedId, MeshName));
	}

	dynamicObjectManager->RegisterObjectId(MeshName, ObjectID->Id, GetOwner()->GetName(),IsController,IsRightController,ControllerInputImageName);

	if (IsController)
	{
		dynamicObjectManager->CacheControllerPointer(this, IsRightController);
	}

	bool hasScaleChanged = true;
	if (FMath::Abs(LastScale.Size() - GetComponentTransform().GetScale3D().Size()) > ScaleThreshold)
	{
		hasScaleChanged = true;
	}

	FDynamicObjectSnapshot initSnapshot = MakeSnapshot(hasScaleChanged);
	SnapshotBoolProperty(initSnapshot, "enabled", true);
	dynamicObjectManager->AddSnapshot(initSnapshot);
	HasInitialized = true;
}

TSharedPtr<FDynamicObjectId> UDynamicObject::GetObjectId()
{
	return ObjectID;
}

void UDynamicObject::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );

	if (!HasInitialized) { return; }
	if (dynamicObjectManager == nullptr) { return; }

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
		dynamicObjectManager->AddSnapshot(snapObj);
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
		if (!GetObjectId().IsValid())
		{
			FString generatedId = FGuid::NewGuid().ToString();
			ObjectID = MakeShareable(new FDynamicObjectId(generatedId, MeshName));
		}
		else if (GetObjectId()->Id.IsEmpty())
		{
			FString generatedId = FGuid::NewGuid().ToString();
			ObjectID = MakeShareable(new FDynamicObjectId(generatedId, MeshName));
		}
		dynamicObjectManager->RegisterObjectId(MeshName, GetObjectId()->Id, GetOwner()->GetName(), IsController, IsRightController, ControllerInputImageName);
	}

	FDynamicObjectSnapshot snapshot = FDynamicObjectSnapshot();

	double ts = Util::GetTimestamp();

	snapshot.timestamp = ts;
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
	dynamicObjectManager->AddSnapshot(snapshot);
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

	TSharedPtr<FAnalyticsProviderCognitiveVR> cogProvider = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Pin();
	cogProvider->OnSessionBegin.RemoveDynamic(this, &UDynamicObject::Initialize);
	cogProvider->OnPostSessionEnd.RemoveDynamic(this, &UDynamicObject::OnPostSessionEnd);
	cogProvider->OnPreSessionEnd.RemoveDynamic(this, &UDynamicObject::OnPreSessionEnd);

	HasInitialized = false;

	if (dynamicObjectManager == nullptr) { return; }

	FDynamicObjectSnapshot initSnapshot = MakeSnapshot(false);
	SnapshotBoolProperty(initSnapshot, "enabled", false);
	dynamicObjectManager->AddSnapshot(initSnapshot);
	ObjectID->Used = false;

	//go through all engagements and send any that match this objectid
	for (auto& Elem : Engagements)
	{
		if (Elem.Value->GetDynamicId() == ObjectID->Id)
		{
			Elem.Value->SetPosition(FVector(-GetComponentLocation().X, GetComponentLocation().Z, GetComponentLocation().Y));
			Elem.Value->Send();
		}
	}
}

//called from optional input tracker to serialize button states
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

	dynamicObjectManager->AddSnapshot(snap);
}


FDynamicObjectManifestEntry* FDynamicObjectManifestEntry::SetProperty(FString key, FString value)
{
	this->StringProperties.Add(key, value);
	return this;
}