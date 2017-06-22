// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CognitiveVR.h"
#include "CognitiveVRProvider.h"
#include "Private/CognitiveVRPrivatePCH.h"
#include "GameFramework/Actor.h"
#include "Voice.h"
#include "OnlineSubsystemUtils.h"
#include "Sound/SoundWaveProcedural.h"
#include "Base64.h"
#include "MicrophoneCaptureActor.generated.h"

UCLASS()
class AMicrophoneCaptureActor : public AActor
{
	GENERATED_BODY()
	
private:
	//uint8* RecordedData;
	TArray<uint8> QueuedAudio;
	FThreadSafeCounter AvailableByteCount;
	void QueueAudio(const uint8* AudioData, const int32 BufferSize);
	//uint32 TotalRecordedBytes;
	TArray<uint8> EncodeToWav(TArray<uint8> rawData);

public:

	//UPROPERTY()
	//float VoiceCaptureVolume;
	
	//UPROPERTY()
	//bool VoiceCaptureTest;
	//UPROPERTY()
	//bool PlayVoiceCaptureFlag;
	
	//UPROPERTY()
	//FTimerHandle VoiceCaptureTickTimer;
	//UPROPERTY()
	//FTimerHandle PlayVoiceCaptureTimer;
	
	TSharedPtr<class IVoiceCapture> VoiceCapture;
	
	UPROPERTY()
	USoundWaveProcedural* VoiceCaptureSoundWaveProcedural;
	UPROPERTY()
	UAudioComponent* VoiceCaptureAudioComponent;
	
	UPROPERTY()
	TArray<uint8> VoiceCaptureBuffer;
	//uint32 VoiceCaptureReadBytes;
	
	//UFUNCTION()
	//void VoiceCaptureTick();
	
	//UFUNCTION()
	//void PlayVoiceCapture();

	// Sets default values for this actor's properties
	AMicrophoneCaptureActor();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;
	
	//void PlayVoiceCapture();
	
	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Microphone")
		bool BeginRecording();

	//TODO get volume, etc

	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Microphone")
		void EndRecording();

	FString wav64string;
	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Microphone")
		FString GetMicrophoneRecording();
};
