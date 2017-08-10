// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CognitiveVR.h"
#include "CognitiveVRProvider.h"
#include "Private/CognitiveVRPrivatePCH.h"
#include "GameFramework/Actor.h"
#include "rtaudio/RtAudio.h"
#include "Base64.h"
#include "MicrophoneCaptureActor.generated.h"

UCLASS()
class AMicrophoneCaptureActor : public AActor
{
	GENERATED_BODY()
	
private:
	//uint8* RecordedData;
	
	//TArray<uint8> QueuedAudio;
	//FThreadSafeCounter AvailableByteCount;
	//void QueueAudio(const uint8* AudioData, const int32 BufferSize);
	//uint32 TotalRecordedBytes;
	void EncodeToWav(TArray<uint8>& OutWaveFileData, const uint8* InPCMData, const int32 NumBytes);
	
	// RtAudio ADC object -- used to interact with low-level audio device.
	RtAudio ADC;

	// Saves raw PCM data recorded to a wave file format
	void SerializeWaveFile(TArray<uint8>& OutWaveFileData, const uint8* InPCMData, const int32 NumBytes);

	// functional quality default sample rate - 16000
	const int32 WAVE_FILE_SAMPLERATE = 16000;

	// Critical section used to stop and retrieve finished audio buffers.
	FCriticalSection CriticalSection;

	// Whether or not the manager is actively recording.
	FThreadSafeBool bRecording;

	// Whether or not we have an error
	//uint32 bError : 1;

	float GainDB = 21;
	float InputGain;

	// Number of overflows detected while recording
	int32 NumOverflowsDetected;

	// The number of frames to record if recording a set duration
	int32 NumFramesToRecord;

	// The data which is currently being recorded to, if the manager is actively recording. This is not safe to access while recording.
	TArray<int16> CurrentRecordedPCMData;

	// Buffer to store sample rate converted PCM data
	TArray<int16> ConvertedPCMData;

	// Reusable raw wave data buffer to generate .wav file formats
	TArray<uint8> RawWaveData;

	// Recording block size (number of frames per callback block)
	int32 RecordingBlockSize;

	// The number of frames that have been recorded
	int32 NumRecordedSamples;

	// Stream parameters to initialize the ADC
	RtAudio::StreamParameters StreamParams;

	// The sample rate used in the recording
	float RecordingSampleRate = 16000;

	// Num input channels
	int32 NumInputChannels = 1;

public:

	int32 OnAudioCapture(void* InBuffer, uint32 InBufferFrames, double StreamTime, bool bOverflow);

	// Sets default values for this actor's properties
	AMicrophoneCaptureActor();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;
	
	//void PlayVoiceCapture();
	
	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Microphone")
		bool BeginRecording(float RecordingDurationSec);

	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Microphone")
		void EndRecording();

	FString wav64string;
	UFUNCTION(BlueprintCallable, Category = "CognitiveVR Analytics|Microphone")
		FString& GetMicrophoneRecording();
};
