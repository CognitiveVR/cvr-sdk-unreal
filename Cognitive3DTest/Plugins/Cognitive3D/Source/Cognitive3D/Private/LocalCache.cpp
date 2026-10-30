/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/

#include "LocalCache.h"
#include "Cognitive3D/Public/Cognitive3DProvider.h"
#include "Analytics.h"
#include "Cognitive3D/Public/Cognitive3D.h"
#include "Misc/FileHelper.h"
#include "Containers/StringConv.h"

FLocalCache::FLocalCache(FString path)
{
	FString ValueReceived;
	localCacheEnabled = false;

	auto cognitive = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (!cognitive.IsValid()) {
		return;
	}

	FString C3DSettingsPath = cognitive->GetSettingsFilePathRuntime();
	GConfig->LoadFile(C3DSettingsPath);

	ValueReceived = FAnalytics::Get().GetConfigValueFromIni(C3DSettingsPath, "/Script/Cognitive3D.Cognitive3DSettings", "EnableLocalCache", false);
	if (ValueReceived.Len() > 0)
	{
		if (ValueReceived == "True")
		{
			localCacheEnabled = true;
		}
	}

	if (localCacheEnabled == false)
	{
		return;
	}

	int64 targetCacheSize = 100LL * 1024 * 1024;
	ValueReceived = FAnalytics::Get().GetConfigValueFromIni(C3DSettingsPath, "/Script/Cognitive3D.Cognitive3DSettings", "LocalCacheSize", false);
	if (ValueReceived.Len() > 0)
	{
		int32 tempSize = FCString::Atoi(*ValueReceived);
		if (tempSize > 0)
		{
			targetCacheSize = static_cast<int64>(tempSize) * 1024 * 1024;
		}
	}
	MaxCacheSize = targetCacheSize;

	//path to project config dir
	writeFilePath = path + "write.txt";
	readFilePath = path + "read.txt";

	MergeDataFiles();
}

int32 FLocalCache::NumberOfBatches()
{
	return readContent.Num() / 2;
}
bool FLocalCache::HasContent()
{
	if (readContent.Num() > 0)
	{
		return true;
	}

	if (numberWriteBatches > 0)
	{
		MergeDataFiles();
	}
	return readContent.Num() > 0;
}

//called on session end. close filestream, serialize any outstanding data batches to disk
void FLocalCache::Close()
{
	if (!localCacheEnabled) { return; }
	SerializeToFile();
	if (WriterArchive != nullptr)
	{
		WriterArchive->Close();
		delete WriterArchive;
		WriterArchive = nullptr;
	}
}

bool FLocalCache::IsEnabled()
{
	return localCacheEnabled;
}

bool FLocalCache::Reopen()
{
	if (!localCacheEnabled) { return false; }
	if (WriterArchive != nullptr && !WriterArchive->IsError()) { return true; }
	if (WriterArchive != nullptr)
	{
		WriterArchive->Close();
		delete WriterArchive;
		WriterArchive = nullptr;
	}
	WriterArchive = FileManager.CreateFileWriter(*writeFilePath, FILEWRITE_Append | FILEWRITE_AllowRead);
	if (WriterArchive == nullptr)
	{
		FCognitiveLog::Error("Cognitive3D local cache writer could not be opened");
		return false;
	}
	return true;
}

bool FLocalCache::CanWrite(int64 byteCount)
{
	if (!localCacheEnabled || byteCount < 0) { return false; }
	const int64 ReadBytes = FMath::Max<int64>(0, FileManager.FileSize(*readFilePath));
	int64 WriteBytes = FMath::Max<int64>(0, FileManager.FileSize(*writeFilePath));
	if (WriterArchive != nullptr)
	{
		WriteBytes = FMath::Max(WriteBytes, WriterArchive->Tell());
	}
	return ReadBytes + WriteBytes <= MaxCacheSize && byteCount <= MaxCacheSize - ReadBytes - WriteBytes;
}

bool FLocalCache::WriteData(FString destination, FString body)
{
	if (!localCacheEnabled || !Reopen()) { return false; }
	FTCHARToUTF8 DestinationUtf8(*destination);
	body.ReplaceInline(TEXT("\r"), TEXT(""));
	body.ReplaceInline(TEXT("\n"), TEXT(""));
	FTCHARToUTF8 BodyUtf8(*body);
	const int64 RequiredBytes = static_cast<int64>(DestinationUtf8.Length()) + BodyUtf8.Length() + 2;
	if (!CanWrite(RequiredBytes))
	{
		FCognitiveLog::Warning("Cognitive3D local cache is full; request could not be retained");
		return false;
	}
	WriterArchive->Serialize(const_cast<ANSICHAR*>(DestinationUtf8.Get()), DestinationUtf8.Length());
	uint8 Newline = '\n';
	WriterArchive->Serialize(&Newline, 1);
	WriterArchive->Serialize(const_cast<ANSICHAR*>(BodyUtf8.Get()), BodyUtf8.Length());
	WriterArchive->Serialize(&Newline, 1);
	WriterArchive->Flush();
	if (WriterArchive->IsError())
	{
		FCognitiveLog::Error("Cognitive3D local cache write failed");
		return false;
	}

	numberWriteBatches++;
	return true;
}

bool FLocalCache::PeekContent(FString& url, FString& content)
{
	if (NumberOfBatches() < 1) //nothing in 'read' file
	{
		if (numberWriteBatches > 0) //there is content in the write file
		{
			MergeDataFiles(); //merged write file into read file
			if (NumberOfBatches() >= 1)
			{
				//returns the url and content
				url = readContent[readContent.Num() - 2];
				content = readContent[readContent.Num() - 1];

				return true;
			}
			else //merge resulted in no new data in read file
			{
				FCognitiveLog::Error("Merging Local Cache files failed?");
				return false;
			}
		}
		else //nothing in write file either
		{
			return false;
		}
	}
	else
	{
		//returns the url and content
		url = readContent[readContent.Num() - 2];
		content = readContent[readContent.Num() - 1];
		return true;
	}
}

void FLocalCache::PopContent()
{
	if (readContent.Num() < 2) { return; }
	readContent.RemoveAt(readContent.Num() - 1);
	readContent.RemoveAt(readContent.Num() - 1);
	SerializeToFile();
}


void FLocalCache::MergeDataFiles()
{
	if (readContent.Num() > 0)
	{
		FCognitiveLog::Error("FLocalCache::MergeDataFiles ERROR readContent not empty");
		return;
	}

	//close writer stream
	if (WriterArchive != nullptr)
	{
		WriterArchive->Flush();
		WriterArchive->Close();
		delete WriterArchive;
		WriterArchive = nullptr;
	}

	//read data from 'write' file to memory
	TArray<FString> writeContent;
	bool writeResult = FFileHelper::LoadFileToStringArray(writeContent, *writeFilePath);
	if (writeResult)
	{
	}
	else
	{
		FCognitiveLog::Error("FLocalCache::MergeDataFiles write file error");
	}

	//read data from 'read' file
	bool readResult = FFileHelper::LoadFileToStringArray(readContent, *readFilePath);
	if (readResult)
	{
	}
	else
	{
		FCognitiveLog::Error("FLocalCache::MergeDataFiles read file error");
	}

	//combine
	readContent.Append(writeContent);

	//serialize read to file
	const FString ReadText = readContent.Num() > 0 ? FString::Join(readContent, TEXT("\n")) + TEXT("\n") : FString();
	bool result = FFileHelper::SaveStringToFile(ReadText, *readFilePath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
	if (result)
	{
	}
	else
	{
		FCognitiveLog::Error("FLocalCache::MergeDataFiles Error serialize readContents to file");
	}

	if (!result) { return; }
	//clear write file
	WriterArchive = FileManager.CreateFileWriter(*writeFilePath, FILEWRITE_AllowRead);
	if (WriterArchive == nullptr)
	{
		FCognitiveLog::Error("Cognitive3D local cache writer could not be opened after merge");
	}
	numberWriteBatches = 0;
}

void FLocalCache::SerializeToFile()
{
	if (WriterArchive != nullptr)
	{
		WriterArchive->Flush();
	}
	const FString ReadText = readContent.Num() > 0 ? FString::Join(readContent, TEXT("\n")) + TEXT("\n") : FString();
	bool result = FFileHelper::SaveStringToFile(ReadText, *readFilePath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
	if (result)
	{

	}
	else
	{
		FCognitiveLog::Error("FLocalCache::SerializeToFile ERROR ReadArchive failed");
	}
}
