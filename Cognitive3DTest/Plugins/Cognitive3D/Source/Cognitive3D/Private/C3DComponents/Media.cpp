// Fill out your copyright notice in the Description page of Project Settings.


#include "C3DComponents/Media.h"
#include "Cognitive3D/Private/C3DApi/CustomEventRecorder.h"

// Sets default values for this component's properties
UMedia::UMedia()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	AssociatedMediaPlayer = nullptr;
}


// Called when the game starts
void UMedia::BeginPlay()
{
	Super::BeginPlay();

	cognitive = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	// Find and bind to the associated media player
	FindAndBindToMediaPlayer();
}


// Called every frame
void UMedia::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

#if WITH_EDITOR
void UMedia::OnRegister()
{
	Super::OnRegister();

	// Check if the owner actor has a Dynamic Object component
	if (AActor* Owner = GetOwner())
	{
		UDynamicObject* ExistingDynamicObject = Owner->FindComponentByClass<UDynamicObject>();
		if (!ExistingDynamicObject)
		{
			// Add a Dynamic Object component if it doesn't exist
			UDynamicObject* NewDynamicObject = NewObject<UDynamicObject>(Owner, UDynamicObject::StaticClass(), TEXT("DynamicObject"));
			if (NewDynamicObject)
			{
				Owner->AddInstanceComponent(NewDynamicObject);

				// Find a mesh component to attach to, otherwise use root
				USceneComponent* AttachParent = Owner->GetRootComponent();

				// Look for any mesh component (StaticMesh, SkeletalMesh, etc.)
				UMeshComponent* MeshComp = Owner->FindComponentByClass<UMeshComponent>();
				if (MeshComp)
				{
					AttachParent = MeshComp;
				}

				if (AttachParent)
				{
					NewDynamicObject->AttachToComponent(AttachParent, FAttachmentTransformRules::KeepRelativeTransform);
				}

				NewDynamicObject->RegisterComponent();
			}
		}
	}
}
#endif

void UMedia::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindFromMediaPlayer();
	Super::EndPlay(EndPlayReason);
}

void UMedia::FindAndBindToMediaPlayer()
{
	if (!GetOwner())
	{
		return;
	}

	// Check for Media Sound component (which has MediaPlayer reference)
	UMediaSoundComponent* mediaSoundComp = GetOwner()->FindComponentByClass<UMediaSoundComponent>();
	if (mediaSoundComp)
	{
		AssociatedMediaPlayer = mediaSoundComp->GetMediaPlayer();
	}

	// If not found in MediaSoundComponent, check for materials with Media Texture
	if (!AssociatedMediaPlayer)
	{
		TArray<UMeshComponent*> MeshComponents;
		GetOwner()->GetComponents<UMeshComponent>(MeshComponents);

		for (UMeshComponent* MeshComp : MeshComponents)
		{
			if (MeshComp)
			{
				TArray<UMaterialInterface*> materials = MeshComp->GetMaterials();
				for (UMaterialInterface* material : materials)
				{
					if (material)
					{
						// Get texture parameters from the material
						TArray<FMaterialParameterInfo> textureParams;
						TArray<FGuid> guids;
						material->GetAllTextureParameterInfo(textureParams, guids);

						for (const FMaterialParameterInfo& param : textureParams)
						{
							UTexture* texture = nullptr;
							if (material->GetTextureParameterValue(param, texture))
							{
								UMediaTexture* mediaTexture = Cast<UMediaTexture>(texture);
								if (mediaTexture)
								{
									AssociatedMediaPlayer = mediaTexture->GetMediaPlayer();
									if (AssociatedMediaPlayer)
									{
										break;
									}
								}
							}
						}

						if (AssociatedMediaPlayer)
						{
							break;
						}
					}
				}

				if (AssociatedMediaPlayer)
				{
					break;
				}
			}
		}
	}

	// Bind to media player events if found
	if (AssociatedMediaPlayer)
	{
		UE_LOG(LogTemp, Log, TEXT("Media component found associated MediaPlayer: %s"), *AssociatedMediaPlayer->GetName());

		AssociatedMediaPlayer->OnMediaOpened.AddDynamic(this, &UMedia::OnMediaPlayerOpened);
		AssociatedMediaPlayer->OnPlaybackResumed.AddDynamic(this, &UMedia::OnMediaPlayerPlaybackResumed);
		AssociatedMediaPlayer->OnPlaybackSuspended.AddDynamic(this, &UMedia::OnMediaPlayerPlaybackSuspended);
		AssociatedMediaPlayer->OnMediaClosed.AddDynamic(this, &UMedia::OnMediaPlayerClosed);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Media component could not find associated MediaPlayer"));
	}
}

void UMedia::UnbindFromMediaPlayer()
{
	if (AssociatedMediaPlayer)
	{
		AssociatedMediaPlayer->OnMediaOpened.RemoveDynamic(this, &UMedia::OnMediaPlayerOpened);
		AssociatedMediaPlayer->OnPlaybackResumed.RemoveDynamic(this, &UMedia::OnMediaPlayerPlaybackResumed);
		AssociatedMediaPlayer->OnPlaybackSuspended.RemoveDynamic(this, &UMedia::OnMediaPlayerPlaybackSuspended);
		AssociatedMediaPlayer->OnMediaClosed.RemoveDynamic(this, &UMedia::OnMediaPlayerClosed);
		AssociatedMediaPlayer = nullptr;
	}
}

void UMedia::OnMediaPlayerOpened(FString OpenedUrl)
{
	UE_LOG(LogTemp, Log, TEXT("Media Player OPENED: %s (Media ID: %s)"), *OpenedUrl, *MediaId);
}

void UMedia::OnMediaPlayerPlaybackResumed()
{
	UE_LOG(LogTemp, Log, TEXT("Media Player RESUMED/PLAYED (Media ID: %s)"), *MediaId);

	// Get video playback timestamp in milliseconds
	int32 videoTimeMs = 0;
	if (AssociatedMediaPlayer)
	{
		FTimespan CurrentTime = AssociatedMediaPlayer->GetTime();
		videoTimeMs = (int32)CurrentTime.GetTotalMilliseconds();
	}

	TSharedPtr<FJsonObject> properties = MakeShareable(new FJsonObject);
	properties->SetStringField("mediaId", MediaId);
	properties->SetNumberField("videoTime", videoTimeMs);

	cognitive->customEventRecorder->Send("cvr.media.play", properties);
}

void UMedia::OnMediaPlayerPlaybackSuspended()
{
	UE_LOG(LogTemp, Log, TEXT("Media Player PAUSED/SUSPENDED (Media ID: %s)"), *MediaId);

	// Get video playback timestamp in milliseconds
	int32 videoTimeMs = 0;
	if (AssociatedMediaPlayer)
	{
		FTimespan CurrentTime = AssociatedMediaPlayer->GetTime();
		videoTimeMs = (int32)CurrentTime.GetTotalMilliseconds();
	}

	TSharedPtr<FJsonObject> properties = MakeShareable(new FJsonObject);
	properties->SetStringField("mediaId", MediaId);
	properties->SetNumberField("videoTime", videoTimeMs);

	cognitive->customEventRecorder->Send("cvr.media.pause", properties);
}

void UMedia::OnMediaPlayerClosed()
{
	UE_LOG(LogTemp, Log, TEXT("Media Player CLOSED/STOPPED (Media ID: %s)"), *MediaId);

	// Get video playback timestamp in milliseconds
	int32 videoTimeMs = 0;
	if (AssociatedMediaPlayer)
	{
		FTimespan CurrentTime = AssociatedMediaPlayer->GetTime();
		videoTimeMs = (int32)CurrentTime.GetTotalMilliseconds();
	}

	TSharedPtr<FJsonObject> properties = MakeShareable(new FJsonObject);
	properties->SetStringField("mediaId", MediaId);
	properties->SetNumberField("videoTime", videoTimeMs);

	cognitive->customEventRecorder->Send("cvr.media.stop", properties);
}

