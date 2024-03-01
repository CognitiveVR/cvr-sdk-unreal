/*
** Copyright (c) 2016 Cognitive3D, Inc. All rights reserved.
*/

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IAnalyticsProviderModule.h"
#include "Cognitive3D/Private/util/cognitive_log.h"
//#include "Cognitive3D/Public/Cognitive3D.h"
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
		bool PeekContent(FString& url, FString& content);
		//remove content
		void PopContent();

		//close filestreams
		void Close();
		
		//checks for max file size
		bool IsEnabled();
		bool CanWrite(int32 byteCount);
		//
		void SerializeToFile();
	private:
		IFileManager& FileManager = FFileManagerGeneric::Get();
		FArchive* WriterArchive = nullptr;
		TArray<FString> readContent;

		void MergeDataFiles();
		FString writeFilePath;
		FString readFilePath;
		//simple tracker for how many entires (url + content) are in the write file without doing IO stuff
		int32 numberWriteBatches = 0;
		bool localCacheEnabled = true;
		int32 MaxCacheSize = 0;
};
