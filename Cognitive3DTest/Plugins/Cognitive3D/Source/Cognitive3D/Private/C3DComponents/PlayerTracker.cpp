/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/

#include "PlayerTracker.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Cognitive3D/Public/Cognitive3DActor.h"
#include "Cognitive3D/Public/DynamicObject.h"
#include "Cognitive3D/Private/C3DUtil/CognitiveLog.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "Cognitive3D/Private/C3DComponents/Media.h"
#include "Materials/MaterialInterface.h"
#include "Materials/Material.h"
#include "Engine/Texture.h"
#include "MediaTexture.h"
#include "Components/StaticMeshComponent.h"
#include "MediaPlayer.h"
#include "Components/PrimitiveComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "MediaSoundComponent.h"
#include "Materials/MaterialInstance.h"

UPlayerTracker::UPlayerTracker()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UPlayerTracker::BeginPlay()
{
	Super::BeginPlay();

	cog = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();

	auto cognitiveActor = ACognitive3DActor::GetCognitive3DActor(GetWorld());
	if (cognitiveActor != GetOwner())
	{
		UnregisterComponent();
		return;
	}

	if (!cog.IsValid())
	{
		return;
	}

	GEngine->GetAllLocalPlayerControllers(controllers);
}

FVector UPlayerTracker::GetWorldGazeEnd(FVector start)
{
#if defined INCLUDE_TOBII_PLUGIN
	auto eyetracker = ITobiiCore::GetEyeTracker();
	FVector End = start + eyetracker->GetCombinedGazeData().WorldGazeDirection * 100000.0f;
	return End;
#elif defined SRANIPAL_1_2_API
	FVector End = FVector::ZeroVector;
	FVector TempStart = FVector::ZeroVector;
	FVector LocalDirection = FVector::ZeroVector;

	if (USRanipal_FunctionLibrary_Eye::GetGazeRay(GazeIndex::COMBINE, TempStart, LocalDirection))
	{
		FVector WorldDir = controllers[0]->PlayerCameraManager->GetActorTransform().TransformVectorNoScale(LocalDirection);
		End = start + WorldDir * 100000.0f;
		LastDirection = WorldDir;
		return End;
	}
	End = start + LastDirection * 100000.0f;
	return End;
#elif defined SRANIPAL_1_3_API
	FVector End = FVector::ZeroVector;
	FVector TempStart = FVector::ZeroVector;
	FVector LocalDirection = FVector::ZeroVector;
	if (SRanipalEye_Core::Instance()->GetGazeRay(GazeIndex::COMBINE, TempStart, LocalDirection))
	{
		FVector WorldDir = controllers[0]->PlayerCameraManager->GetActorTransform().TransformVectorNoScale(LocalDirection);
		End = start + WorldDir * 100000.0f;
		LastDirection = WorldDir;
		return End;
	}
	End = start + LastDirection * 100000.0f;
	return End;
#elif defined INCLUDE_VARJO_PLUGIN
	FVector Start = start;
	FVector WorldDirection = FVector::ZeroVector;
	FVector End = FVector::ZeroVector;
	float ignored = 0;

	FVarjoEyeTrackingData data;

	if (UVarjoEyeTrackerFunctionLibrary::GetEyeTrackerGazeData(data)) //if the data is valid
	{
		//the gaze transformed into world space
		UVarjoEyeTrackerFunctionLibrary::GetGazeRay(Start, WorldDirection, ignored);

		End = start + WorldDirection * 10000.0f;
		LastDirection = WorldDirection;
		return End;
	}
	End = start + LastDirection * 100000.0f;
	return End;
#elif defined INCLUDE_PICOMOBILE_PLUGIN
	FVector Start = FVector::ZeroVector;
	FVector WorldDirection = FVector::ZeroVector;
	FVector End = FVector::ZeroVector;
	
	if (UPicoBlueprintFunctionLibrary::PicoGetEyeTrackingGazeRay(Start, WorldDirection))
	{
		End = Start + WorldDirection * 10000.0f;
	}
	return End;
#elif defined INCLUDE_HPGLIA_PLUGIN
	FVector End = FVector::ZeroVector;
	FVector TempStart = controllers[0]->PlayerCameraManager->GetCameraLocation();

	FEyeTracking eyeTrackingData;
	if (UHPGliaClient::GetEyeTracking(eyeTrackingData))
	{
		if (eyeTrackingData.CombinedGazeConfidence > 0.4f)
		{
			FVector dir = FVector(eyeTrackingData.CombinedGaze.X, eyeTrackingData.CombinedGaze.Y, eyeTrackingData.CombinedGaze.Z);
			LastDirection = controllers[0]->PlayerCameraManager->GetActorTransform().TransformVectorNoScale(dir);
			End = TempStart + LastDirection * 100000.0f;
			return End;
		}
	}
	End = TempStart + LastDirection * 100000.0f;
	return End;
#elif defined INCLUDE_OCULUS_PLUGIN

	FRotator captureRotation = controllers[0]->PlayerCameraManager->GetCameraRotation();
	FVector End = start + captureRotation.Vector() * 100000.0f;
	IEyeTracker const* const ET = GEngine ? GEngine->EyeTrackingDevice.Get() : nullptr;
	if (ET != nullptr)
	{
		FEyeTrackerGazeData gazeData;
		if (ET->GetEyeTrackerGazeData(gazeData))
		{
			LastDirection = gazeData.GazeDirection;
			End = start + LastDirection * 100000.0f;
		}
	}
	return End;
#elif defined OPENXR_EYETRACKING

	FRotator captureRotation = controllers[0]->PlayerCameraManager->GetCameraRotation();
	FVector End = start + captureRotation.Vector() * 100000.0f;
	IEyeTracker const* const ET = GEngine ? GEngine->EyeTrackingDevice.Get() : nullptr;
	if (ET != nullptr)
	{
		FEyeTrackerGazeData gazeData;
		if (ET->GetEyeTrackerGazeData(gazeData))
		{
			LastDirection = gazeData.GazeDirection;
			End = start + LastDirection * 100000.0f;
		}
	}
	return End;
#elif defined WAVEVR_EYETRACKING
	WaveVREyeManager* pEyeManager = WaveVREyeManager::GetInstance();
	FVector End;
	if (pEyeManager != nullptr)
	{
		//is this world direction or local direction?
		if (pEyeManager->GetCombindedEyeDirectionNormalized(LastDirection))
		{
			LastDirection = controllers[0]->PlayerCameraManager->GetActorTransform().TransformVectorNoScale(LastDirection);
		}
	}
	End = start + LastDirection * 100000.0f;
	return End;
#else
	FRotator captureRotation = controllers[0]->PlayerCameraManager->GetCameraRotation();
	FVector End = start + captureRotation.Vector() * 100000.0f;
	return End;
#endif
}

void UPlayerTracker::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	if (!cog.IsValid()) { return; }
	if (!cog->HasStartedSession())
	{
		//don't record player position data before a session has begun
		return;
	}
	if (cog->CurrentTrackingSceneId.IsEmpty()) { return; }

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	currentTime += DeltaTime;
	if (currentTime < PlayerSnapshotInterval)
	{
		return;
	}

	currentTime -= PlayerSnapshotInterval;

	bool usingMedia = false;

	double timestamp = FUtil::GetTimestamp();
	FString objectid = "";

	if (controllers.Num() == 0)
	{
		FCognitiveLog::Info("UPlayerTracker::TickComponent--------------------------no controllers. skip");
		return;
	}

	FVector captureLocation = controllers[0]->PlayerCameraManager->GetCameraLocation();
	FRotator captureRotation = controllers[0]->PlayerCameraManager->GetCameraRotation();

	bool DidHitFloor = false;
	FVector FloorHitPosition = FVector::ZeroVector;
	if (RecordGazeHit)
	{


		//look at dynamic object
		FCollisionQueryParams Params; // You can use this to customize various properties about the trace
		Params.AddIgnoredActor(GetOwner()); // Ignore the player's pawn


		FHitResult Hit; // The hit result gets populated by the line trace
		FHitResult FloorHit; // The hit result gets populated by the line trace

		FVector Start = captureLocation;
		FVector End = GetWorldGazeEnd(Start);

		FCollisionObjectQueryParams params = FCollisionObjectQueryParams();
		params.AddObjectTypesToQuery(ECC_WorldStatic);
		params.AddObjectTypesToQuery(ECC_WorldDynamic);

		bool bHit = false;
		FCollisionQueryParams gazeparams = FCollisionQueryParams(FName(), true);
		gazeparams.bTraceComplex = true; // This is the key for UV data!
		gazeparams.bReturnFaceIndex = true; // Also helps with UV calculation
		bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECollisionChannel::ECC_Visibility, gazeparams);

		GetWorld()->LineTraceSingleByObjectType(FloorHit, captureLocation, FVector(0, 0, -1000), params);

		DidHitFloor = false;

		if (FloorHit.GetActor() != NULL)
		{
			DidHitFloor = true;
			FloorHitPosition = FloorHit.ImpactPoint;
		}

		if (bHit)
		{
			FVector gaze = Hit.ImpactPoint;

			// Detect media components and calculate UV coordinates using helper functions
			FMediaDetectionResult MediaResult = DetectMediaComponents(Hit);
			FVector2D UVCoordinates = FVector2D::ZeroVector;
			bool bValidUV = false;
			
			// Only calculate UV coordinates for media objects
			if (MediaResult.bIsMediaObject)
			{
				bValidUV = CalculateUVCoordinates(Hit, UVCoordinates);
				
				if (DebugDisplayUVCoordinates)
				{
					// First, let's see what we're hitting
					FString HitDebugInfo = FString::Printf(TEXT("MEDIA HIT: Actor=%s, Component=%s"), 
						Hit.GetActor() ? *Hit.GetActor()->GetName() : TEXT("NULL"),
						Hit.GetComponent() ? *Hit.GetComponent()->GetName() : TEXT("NULL"));
					
					FCognitiveLog::Info(HitDebugInfo);
					if (GEngine)
					{
						GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, HitDebugInfo);
					}
				}
				
			}

			if (Hit.GetActor() != NULL)
			{
				UActorComponent* hitActorComponent = Hit.GetActor()->GetComponentByClass(UDynamicObject::StaticClass());
				if (hitActorComponent != NULL)
				{
					UDynamicObject* hitDynamicObject = Cast<UDynamicObject>(hitActorComponent);
					if (hitDynamicObject != NULL && hitDynamicObject->GetObjectId().IsValid())
					{
						FVector localHitPosition = hitDynamicObject->GetOwner()->GetActorTransform().InverseTransformPosition(Hit.ImpactPoint);

						objectid = hitDynamicObject->GetObjectId()->Id;
						gaze.X = localHitPosition.X;
						gaze.Y = localHitPosition.Y;
						gaze.Z = localHitPosition.Z;
					}
				}
				else
				{
					//hit an actor that is not a dynamic object
				}
			}
			else
			{
				//hit some csg or something that is not an actor
			}
			// Use media snapshot if we have UV coordinates and MediaPlayer data
			if (MediaResult.bIsMediaObject && bValidUV && MediaResult.FoundMediaPlayer)
			{
				// Get media timestamp in milliseconds
				int32 mediaTimeMs = 0;
				if (MediaResult.FoundMediaPlayer && MediaResult.FoundMediaPlayer != nullptr)
				{
					FTimespan CurrentTime = MediaResult.FoundMediaPlayer->GetTime();
					mediaTimeMs = (int32)CurrentTime.GetTotalMilliseconds();
				}

				// Get the actual media ID from the Media component
				FString mediaId = TEXT("");
				if (MediaResult.FoundMediaComponent && !MediaResult.FoundMediaComponent->MediaId.IsEmpty())
				{
					mediaId = MediaResult.FoundMediaComponent->MediaId;
				}

				// Use media BuildSnapshot with UV coordinates
				cog->gazeDataRecorder->BuildSnapshot(
					mediaId,
					mediaTimeMs,
					UVCoordinates,
					captureLocation,
					captureRotation,
					timestamp,
					DidHitFloor,
					FloorHitPosition,
					gaze,
					objectid
				);
				
				if (DebugDisplayUVCoordinates && GEngine)
				{
					GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, 
						FString::Printf(TEXT("MEDIA SNAPSHOT: UV(%.3f,%.3f) Time:%dms"), 
							UVCoordinates.X, UVCoordinates.Y, mediaTimeMs));
				}
			}
			else
			{
				// Use regular snapshot without media data
				cog->gazeDataRecorder->BuildSnapshot(captureLocation, gaze, captureRotation, timestamp, DidHitFloor, FloorHitPosition, objectid);
			}

			if (DebugDisplayGaze)
				DrawDebugSphere(GetWorld(), gaze, 3, 3, FColor::White, false, 0.2);
		}
		else
		{
			//hit nothing. use position and rotation only
			cog->gazeDataRecorder->BuildSnapshot(captureLocation, captureRotation, timestamp, DidHitFloor, FloorHitPosition);
		}
	}
	else //RecordGazeHit is false, so we only record the player's position and rotation
	{
		cog->gazeDataRecorder->BuildSnapshot(captureLocation, captureRotation, timestamp, DidHitFloor, FloorHitPosition);
	}
	cog->OnCognitiveInterval.Broadcast();
}

void UPlayerTracker::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

float UPlayerTracker::GetLastSendTime()
{
	if (!cog.IsValid()) { return 0; }
	if (cog->gazeDataRecorder == nullptr) { return 0; }
	return cog->gazeDataRecorder->GetLastSendTime();
}
int32 UPlayerTracker::GetPartNumber()
{
	if (!cog.IsValid()) { return 0; }
	if (cog->gazeDataRecorder == nullptr) { return 0; }
	return cog->gazeDataRecorder->GetPartNumber();
}
int32 UPlayerTracker::GetDataPoints()
{
	if (!cog.IsValid()) { return 0; }
	if (cog->gazeDataRecorder == nullptr) { return 0; }
	return cog->gazeDataRecorder->GetDataPoints();
}

UPlayerTracker::FMediaDetectionResult UPlayerTracker::DetectMediaComponents(const FHitResult& Hit)
{
	FMediaDetectionResult Result;

	if (Hit.GetActor() == nullptr)
	{
		return Result;
	}

	// Check for Media component
	UActorComponent* mediaComponent = Hit.GetActor()->GetComponentByClass(UMedia::StaticClass());
	if (mediaComponent != nullptr)
	{
		Result.bIsMediaObject = true;
		Result.FoundMediaComponent = Cast<UMedia>(mediaComponent);
	}

	// Check for Media Sound component (which has MediaPlayer reference)
	UMediaSoundComponent* mediaSoundComp = Hit.GetActor()->FindComponentByClass<UMediaSoundComponent>();
	if (mediaSoundComp)
	{
		Result.bIsMediaObject = true;
		Result.FoundMediaPlayer = mediaSoundComp->GetMediaPlayer();

		if (DebugDisplayUVCoordinates && GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange,
				FString::Printf(TEXT("Found MediaSoundComponent, Player: %s"),
					Result.FoundMediaPlayer ? *Result.FoundMediaPlayer->GetName() : TEXT("NULL")));
		}
	}

	// Check for materials with Media Texture
	if (!Result.bIsMediaObject && Hit.GetComponent())
	{
		UStaticMeshComponent* meshComp = Cast<UStaticMeshComponent>(Hit.GetComponent());
		if (meshComp)
		{
			for (int32 i = 0; i < meshComp->GetNumMaterials(); i++)
			{
				UMaterialInterface* material = meshComp->GetMaterial(i);
				if (material)
				{
					// Check if material uses MediaTexture (this is a simplified check)
					FString materialName = material->GetName();
					if (materialName.Contains(TEXT("Media")) || materialName.Contains(TEXT("Video")))
					{
						Result.bIsMediaObject = true;

						if (DebugDisplayUVCoordinates && GEngine)
						{
							GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Cyan,
								FString::Printf(TEXT("Found media material: %s"), *materialName));
						}
					}

					// Also try to find MediaTexture directly in material parameters
					// Try both dynamic and regular material instances
					UMaterialInstanceDynamic* dynMaterial = Cast<UMaterialInstanceDynamic>(material);
					UMaterialInstance* matInstance = Cast<UMaterialInstance>(material);

					if (dynMaterial || matInstance)
					{
						// Try common parameter names for media textures
						TArray<FString> CommonMediaParams = { TEXT("MediaTexture"), TEXT("Video"), TEXT("Media"), TEXT("VideoTexture"), TEXT("Texture"), TEXT("BaseColor") };
						for (const FString& ParamName : CommonMediaParams)
						{
							UTexture* texture = nullptr;
							FName paramFName(*ParamName);

							// Try dynamic material first
							bool foundParam = false;
							if (dynMaterial)
							{
								foundParam = dynMaterial->GetTextureParameterValue(paramFName, texture);
							}
							// Then try regular material instance
							else if (matInstance)
							{
								foundParam = matInstance->GetTextureParameterValue(paramFName, texture);
							}

							if (foundParam && texture)
							{
								UMediaTexture* mediaTexture = Cast<UMediaTexture>(texture);
								if (mediaTexture)
								{
									Result.bIsMediaObject = true;
									Result.FoundMediaTexture = mediaTexture;
									Result.FoundMediaPlayer = mediaTexture->GetMediaPlayer();

									if (DebugDisplayUVCoordinates && GEngine)
									{
										GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Magenta,
											FString::Printf(TEXT("Found MediaTexture in '%s': %s, Player: %s"),
												*ParamName,
												*mediaTexture->GetName(),
												Result.FoundMediaPlayer ? *Result.FoundMediaPlayer->GetName() : TEXT("NULL")));
									}
									break;
								}
							}
						}
					}

					if (Result.bIsMediaObject && Result.FoundMediaPlayer)
					{
						break; // Found what we need
					}
				}
			}
		}
	}

	return Result;
}

bool UPlayerTracker::CalculateUVCoordinates(const FHitResult& Hit, FVector2D& OutUVCoordinates)
{
	OutUVCoordinates = FVector2D::ZeroVector;
	bool bValidUV = false;

	if (DebugDisplayUVCoordinates)
	{
		// First, let's see what we're hitting
		FString HitDebugInfo = FString::Printf(TEXT("MEDIA HIT: Actor=%s, Component=%s"),
			Hit.GetActor() ? *Hit.GetActor()->GetName() : TEXT("NULL"),
			Hit.GetComponent() ? *Hit.GetComponent()->GetName() : TEXT("NULL"));

		FCognitiveLog::Info(HitDebugInfo);
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, HitDebugInfo);
		}
	}

	if (Hit.GetActor() != nullptr && Hit.GetComponent() != nullptr)
	{
		// Try harder to get real UV coordinates with complex trace
		if (DebugDisplayUVCoordinates && GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::White,
				FString::Printf(TEXT("Complex Trace: FaceIndex=%d, Item=%d"),
					Hit.FaceIndex, Hit.Item));
		}

		bValidUV = UGameplayStatics::FindCollisionUV(Hit, 0, OutUVCoordinates);
		if (!bValidUV)
		{
			// Try UV channel 1
			bValidUV = UGameplayStatics::FindCollisionUV(Hit, 1, OutUVCoordinates);
		}
		if (!bValidUV)
		{
			// Try UV channel 2
			bValidUV = UGameplayStatics::FindCollisionUV(Hit, 2, OutUVCoordinates);
		}

		// Flip Y coordinate to match dashboard coordinate system (0,0 = bottom-left)
		if (bValidUV)
		{
			OutUVCoordinates.Y = 1.0f - OutUVCoordinates.Y;
		}

		if (DebugDisplayUVCoordinates)
		{
			if (bValidUV && GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Purple,
					FString::Printf(TEXT("FindCollisionUV succeeded! REAL UV: (%.3f, %.3f)"),
						OutUVCoordinates.X, OutUVCoordinates.Y));
			}
			else if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red,
					TEXT("FindCollisionUV failed - falling back to manual"));
			}
		}

		// If FindCollisionUV still doesn't work, use simple manual calculation
		if (!bValidUV)
		{
			// Convert world hit position to local coordinates
			FVector LocalHitPos = Hit.GetActor()->GetActorTransform().InverseTransformPosition(Hit.ImpactPoint);

			// Simple UV calculation - flip Y to make (0,0) bottom-left, (1,1) top-right
			OutUVCoordinates.X = FMath::Clamp((LocalHitPos.X / 100.0f) + 0.5f, 0.0f, 1.0f);
			OutUVCoordinates.Y = 1.0f - FMath::Clamp((LocalHitPos.Y / 100.0f) + 0.5f, 0.0f, 1.0f);
			bValidUV = true;

			if (DebugDisplayUVCoordinates && GEngine)
			{
				FString ManualCalcInfo = FString::Printf(TEXT("Simple Manual UV: LocalPos(%.1f,%.1f,%.1f)"),
					LocalHitPos.X, LocalHitPos.Y, LocalHitPos.Z);
				GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, ManualCalcInfo);
			}
		}

		if (DebugDisplayUVCoordinates)
		{
			FString UVResultString = FString::Printf(TEXT("UV Result: Valid=%s, UV=(%.3f, %.3f)"),
				bValidUV ? TEXT("TRUE") : TEXT("FALSE"),
				OutUVCoordinates.X, OutUVCoordinates.Y);

			FCognitiveLog::Info(UVResultString);
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, UVResultString);
			}
		}

		if (bValidUV && DebugDisplayUVCoordinates)
		{
			FString UVDebugString = FString::Printf(TEXT("MEDIA UV: (%.3f, %.3f) - Actor: %s"),
				OutUVCoordinates.X, OutUVCoordinates.Y,
				*Hit.GetActor()->GetName());

			// Try to get media timestamp if available
			// Note: This is a simplified approach - finding the actual MediaPlayer requires
			// more complex material graph traversal
			FString TimestampInfo = TEXT("");
			// This debug info doesn't access the media player since it's not passed to this function
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, UVDebugString + TimestampInfo);
			}
		}
	}

	return bValidUV;
}
