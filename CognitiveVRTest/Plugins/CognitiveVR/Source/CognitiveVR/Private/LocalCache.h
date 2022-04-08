/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/

#pragma once

#include "CoreMinimal.h"
#include "CognitiveVR/Public/CognitiveVR.h"
#include "Json.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManagerGeneric.h"
#if PLATFORM_ANDROID
#include "Android/AndroidPlatformMisc.h"
#else
#include "Windows/WindowsPlatformMisc.h"
#endif

class LocalCache
{
	public:

		LocalCache(FString path);

		void WriteData(FString destination, FString body);

		int32 NumberOfBatches();
		bool HasContent();
		
		//gets content without removing it
		bool PeekContent(FString* url, FString* content);
		//remove content
		void PopContent();

		//close filestreams
		void Close();
		
		//checks for max file size
		bool CanWrite();
	private:
		IFileManager& FileManager = FFileManagerGeneric::Get();
		FArchive* WriterArchive;
		TArray<FString> readContent;

		void MergeDataFiles();
		FString writeFilePath;
		FString readFilePath;
};
