// Fill out your copyright notice in the Description page of Project Settings.

#include "CognitiveVR.h"
#include "CognitiveVRProvider.h"
#include "Private/CognitiveVRPrivatePCH.h"
#include "MicrophoneCaptureActor.h"

static int32 OnAudioCaptureCallback(void *OutBuffer, void* InBuffer, uint32 InBufferFrames, double StreamTime, CRtAudioStreamStatus AudioStreamStatus, void* InUserData)
{
	// Cast the user data to the mic recorder
	AMicrophoneCaptureActor* MicrophoneCaptureActor = (AMicrophoneCaptureActor*)InUserData;

	// Call the OnAudioCapture function
	return MicrophoneCaptureActor->OnAudioCapture(InBuffer, InBufferFrames, StreamTime, AudioStreamStatus == RTAUDIO_INPUT_OVERFLOW);
}

AMicrophoneCaptureActor::AMicrophoneCaptureActor()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AMicrophoneCaptureActor::BeginPlay()
{
	Super::BeginPlay();	
}

void AMicrophoneCaptureActor::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );
}

bool AMicrophoneCaptureActor::BeginRecording(float RecordingDurationSec)
{
	// If we have a stream open close it (reusing streams can cause a blip of previous recordings audio)
	if (ADC.isStreamOpen())
	{

		ADC.stopStream();
		ADC.closeStream();
		//normally wrapped in try/catch, but ue4 doesnt like exceptions
	}

	// Convert input gain decibels into linear scale
	InputGain = FMath::Pow(10.0f, GainDB / 20.0f);

	if (!ADC.getDeviceInfo((int32)ADC.getDefaultInputDevice()).isDefaultInput)
	{
		cognitivevrapi::CognitiveLog::Warning("MicrophoneCaptureActor BeginRecording no default input source!");
		return false;
	}

	if (RecordingDurationSec == 0)
	{
		cognitivevrapi::CognitiveLog::Warning("MicrophoneCaptureActor BeginRecording duration is 0 seconds!");
		return false;
	}

	CRtAudio::DeviceInfo Info = ADC.getDeviceInfo(StreamParams.deviceId);
	RecordingSampleRate = 16000;// Info.preferredSampleRate;
	NumInputChannels = 1;// Info.inputChannels;

	int32 NumSamplesToReserve = RecordingDurationSec * RecordingSampleRate * NumInputChannels;
	CurrentRecordedPCMData.Reset(NumSamplesToReserve);

	NumRecordedSamples = 0;
	NumOverflowsDetected = 0;

	NumFramesToRecord = RecordingSampleRate * RecordingDurationSec;

	// Publish to the mic input thread that we're ready to record...
	bRecording = true;

	StreamParams.deviceId = ADC.getDefaultInputDevice(); // Only use the default input device for now

	StreamParams.nChannels = NumInputChannels;
	StreamParams.firstChannel = 0;

	uint32 BufferFrames = FMath::Max(RecordingBlockSize, 256);

	//UE_LOG(LogTemp, Log, TEXT("AMyActor::BeginRecording Initialized mic recording manager at %d hz sample rate, %d channels, and %d Recording Block Size"), (int32)RecordingSampleRate, StreamParams.nChannels, BufferFrames);

	// RtAudio uses exceptions for error handling
	ADC.openStream(nullptr, &StreamParams, RTAUDIO_SINT16, RecordingSampleRate, &BufferFrames, &OnAudioCaptureCallback, this);
	//try catch exceptions
	ADC.startStream();

	return true;
}

//from OnAudioCaptureCallback, used in BeginRecording ADC.openStream
int32 AMicrophoneCaptureActor::OnAudioCapture(void* InBuffer, uint32 InBufferFrames, double StreamTime, bool bOverflow)
{
	FScopeLock Lock(&CriticalSection);

	if (bRecording)
	{
		if (bOverflow)
			++NumOverflowsDetected;

		CurrentRecordedPCMData.Append((int16*)InBuffer, InBufferFrames * NumInputChannels);
		return 0;
	}

	return 1;
}


//static void SampleRateConvert(float CurrentSR, float TargetSR, int32 NumChannels, const TArray<int16>& CurrentRecordedPCMData, int32 NumSamplesToConvert, TArray<int16>& OutConverted){}

void AMicrophoneCaptureActor::EndRecording()
{
	FScopeLock Lock(&CriticalSection);

	// If we're currently recording, stop the recording
	if (bRecording)
	{
		bRecording = false;

		if (CurrentRecordedPCMData.Num() > 0)
		{
			NumRecordedSamples = FMath::Min(NumFramesToRecord * NumInputChannels, CurrentRecordedPCMData.Num());

			/*UE_LOG(LogTemp, Log, TEXT("Stopping mic recording. Recorded %d frames of audio (%.4f seconds). Detected %d buffer overflows."),
				NumRecordedSamples,
				(float)NumRecordedSamples / RecordingSampleRate,
				NumOverflowsDetected);*/

			// Get a ptr to the buffer we're actually going to serialize
			TArray<int16>* PCMDataToSerialize = nullptr;

			// If our sample rate isn't 44100, then we need to do a SampleRateConvert
			PCMDataToSerialize = &CurrentRecordedPCMData;

			// Scale by the linear gain if it's been set to something (0.0f is ctor default and impossible to set by dB)
			if (InputGain != 0.0f)
			{
				for (int32 i = 0; i < NumRecordedSamples; ++i)
				{
					// Scale by input gain, clamp to prevent integer overflow when casting back to int16. Will still clip.
					(*PCMDataToSerialize)[i] = (int16)FMath::Clamp(InputGain * (float)(*PCMDataToSerialize)[i], -32767.0f, 32767.0f);
				}
			}

			// Get the raw data
			const uint8* RawData = (const uint8*)PCMDataToSerialize->GetData();
			int32 NumBytes = NumRecordedSamples * sizeof(int16);

			// Create a raw .wav file to stuff the raw PCM data in so when we create the sound wave asset it's identical to a normal imported asset
			//SerializeWaveFile(RawWaveData, RawData, NumBytes);
			EncodeToWav(RawWaveData, RawData, NumBytes);


			//DEBUG save wav to disk
			/*
			FString SaveDirectory = FString("C:/Users/calder/Desktop");
			FString FileName = FString("recordedAudio" + FString::FromInt(NumBytes) + ".wav");

			IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

			if (PlatformFile.CreateDirectoryTree(*SaveDirectory))
			{
				// Get absolute file path
				FString AbsoluteFilePath = SaveDirectory + "/" + FileName;

				//FFileHelper::SaveStringToFile(Content, *AbsoluteFilePath);
				FFileHelper::SaveArrayToFile(RawWaveData, *AbsoluteFilePath);
				//UE_LOG(LogTemp, Warning, TEXT("AMicrophoneCaptureActor::EndRecording saved wav file to desktop"));
			}*/
		}
	}
}

//static void WriteUInt32ToByteArrayLE(TArray<uint8>& InByteArray, int32& Index, const uint32 Value){}

//static void WriteUInt16ToByteArrayLE(TArray<uint8>& InByteArray, int32& Index, const uint16 Value) {}

//void AMicrophoneCaptureActor::SerializeWaveFile(TArray<uint8>& OutWaveFileData, const uint8* InPCMData, const int32 NumBytes){}

void AMicrophoneCaptureActor::EncodeToWav(TArray<uint8>& OutWaveFileData, const uint8* InPCMData, const int32 NumBytes)
{
	uint8 bytes[4];
	uint8 twobytes[2];
	unsigned long n = 0;

	OutWaveFileData.Empty(NumBytes + 44);
	//OutWaveFileData.AddZeroed(NumBytes + 44);

	///RIFF

	//riff (big)
	FString riff = "RIFF";
	FTCHARToUTF8 Converter1(*riff);
	auto riffdata = (const uint8*)Converter1.Get();
	OutWaveFileData.Append(riffdata, 4);

	//chunk size (little)
	n = NumBytes + 36;
	bytes[3] = (n >> 24) & 0xFF;
	bytes[2] = (n >> 16) & 0xFF;
	bytes[1] = (n >> 8) & 0xFF;
	bytes[0] = n & 0xFF;
	OutWaveFileData.Append(bytes, 4);

	//format (big)
	FString wave = "WAVE";
	FTCHARToUTF8 Converter2(*wave);
	auto wavedata = (const uint8*)Converter2.Get();
	OutWaveFileData.Append(wavedata, 4);

	///fmt sub-chunk

	//subchunk1 id (big)
	FString fmt = "fmt ";
	FTCHARToUTF8 Converter3(*fmt);
	auto fmtdata = (const uint8*)Converter3.Get();
	OutWaveFileData.Append(fmtdata, 4);

	//subchunk 1 size (little)
	n = 16;
	bytes[3] = (n >> 24) & 0xFF;
	bytes[2] = (n >> 16) & 0xFF;
	bytes[1] = (n >> 8) & 0xFF;
	bytes[0] = n & 0xFF;
	OutWaveFileData.Append(bytes, 4);

	//audio format (little)
	n = 1;
	twobytes[1] = (n >> 8) & 0xFF;
	twobytes[0] = n & 0xFF;
	OutWaveFileData.Append(twobytes, 2);

	//num channels (little)
	n = NumInputChannels;
	twobytes[1] = (n >> 8) & 0xFF;
	twobytes[0] = n & 0xFF;
	OutWaveFileData.Append(twobytes, 2);

	//sample rate (little)
	n = WAVE_FILE_SAMPLERATE;
	bytes[3] = (n >> 24) & 0xFF;
	bytes[2] = (n >> 16) & 0xFF;
	bytes[1] = (n >> 8) & 0xFF;
	bytes[0] = n & 0xFF;
	OutWaveFileData.Append(bytes, 4);

	//byte rate (little)
	n = WAVE_FILE_SAMPLERATE * NumInputChannels * 2;
	bytes[3] = (n >> 24) & 0xFF;
	bytes[2] = (n >> 16) & 0xFF;
	bytes[1] = (n >> 8) & 0xFF;
	bytes[0] = n & 0xFF;
	OutWaveFileData.Append(bytes, 4);

	//block align (little)
	n = 2;
	twobytes[1] = (n >> 8) & 0xFF;
	twobytes[0] = n & 0xFF;
	OutWaveFileData.Append(twobytes, 2);

	//bits per sample (little)
	n = 16;
	twobytes[1] = (n >> 8) & 0xFF;
	twobytes[0] = n & 0xFF;
	OutWaveFileData.Append(twobytes, 2);

	/// data subchunk

	//subchunk2 id (big)
	FString data = "data";
	FTCHARToUTF8 Converter4(*data);
	auto datadata = (const uint8*)Converter4.Get();
	OutWaveFileData.Append(datadata, 4);

	//subchunk2 size (little)
	n = NumBytes;
	bytes[3] = (n >> 24) & 0xFF;
	bytes[2] = (n >> 16) & 0xFF;
	bytes[1] = (n >> 8) & 0xFF;
	bytes[0] = n & 0xFF;
	OutWaveFileData.Append(bytes, 4);

	OutWaveFileData.Append(InPCMData, NumBytes);

	wav64string = FBase64::Encode(OutWaveFileData);
}

FString& AMicrophoneCaptureActor::GetMicrophoneRecording()
{
	return wav64string;
}