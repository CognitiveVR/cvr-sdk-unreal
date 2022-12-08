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
			GWorld->MarkPackageDirty();
		}
	}
	if (CustomId.IsEmpty())
	{
		CustomId = FGuid::NewGuid().ToString();
		GWorld->MarkPackageDirty();
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

	//TODO IMPROVEMENT write a cached getter function to get the dynamic manager in the CognitiveActor class, instead of casting here
	auto cognitiveActor = ACognitiveVRActor::GetCognitiveVRActor();
	if (cognitiveActor == nullptr) { return; }
	dynamicObjectManager = Cast<UDynamicObjectManager>(cognitiveActor->GetComponentByClass(UDynamicObjectManager::StaticClass()));

	if (dynamicObjectManager == nullptr) { return; }

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

	dynamicObjectManager->RegisterObjectId(MeshName, ObjectID->Id, GetOwner()->GetName(),IsController,IsRightController,ControllerType);

	if (SnapshotOnBeginPlay)
	{
		bool hasScaleChanged = true;
		if (FMath::Abs(LastScale.Size() - GetComponentTransform().GetScale3D().Size()) > ScaleThreshold)
		{
			hasScaleChanged = true;
		}

		FDynamicObjectSnapshot initSnapshot = MakeSnapshot(hasScaleChanged);
		SnapshotBoolProperty(initSnapshot, "enable", true);
		dynamicObjectManager->AddSnapshot(initSnapshot);
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
	return ObjectID;
}

void UDynamicObject::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );

	if (dynamicObjectManager == nullptr) { return; }
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
		dynamicObjectManager->AddSnapshot(snapObj);
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
		//check that this objectid is in the manifest
		if (dynamicObjectManager->HasRegisteredObjectId(ObjectID->Id) == false)
		{
			needObjectId = true;
		}
	}

	if (needObjectId)
	{
		//GenerateObjectId();
		dynamicObjectManager->RegisterObjectId(MeshName, GetObjectId()->Id, GetOwner()->GetName(), IsController, IsRightController, ControllerType);
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

	HasInitialized = false;

	if (dynamicObjectManager == nullptr) { return; }

	if (ReleaseIdOnDestroy)
	{
		FDynamicObjectSnapshot initSnapshot = MakeSnapshot(false);
		SnapshotBoolProperty(initSnapshot, "enable", false);
		dynamicObjectManager->AddSnapshot(initSnapshot);
		ObjectID->Used = false;
	}

	if (EndPlayReason == EEndPlayReason::EndPlayInEditor || EndPlayReason == EEndPlayReason::Destroyed || EndPlayReason == EEndPlayReason::LevelTransition || EndPlayReason == EEndPlayReason::RemovedFromWorld)
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

//static uility
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

//static utility
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

//static utility
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