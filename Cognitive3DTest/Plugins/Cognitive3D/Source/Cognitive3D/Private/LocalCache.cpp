/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/

#include "LocalCache.h"
#include "Analytics.h"

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

	int32 targetCacheSize = 100 * 1024 * 1024; //convert to MB
	ValueReceived = FAnalytics::Get().GetConfigValueFromIni(C3DSettingsPath, "/Script/Cognitive3D.Cognitive3DSettings", "LocalCacheSize", false);
	if (ValueReceived.Len() > 0)
	{
		int32 tempSize = FCString::Atoi(*ValueReceived);
		if (tempSize > 0)
		{
			targetCacheSize = tempSize * 1024 * 1024; //convert to MB
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

bool FLocalCache::CanWrite(int32 byteCount)
{
	if (!localCacheEnabled) { return false; }

	int32 currentCacheSize = 0;
	if (byteCount + currentCacheSize < MaxCacheSize)
	{
		return true;
	}
	return false;
}

void FLocalCache::WriteData(FString destination, FString body)
{
	if (WriterArchive == nullptr) { return; }
	if (!localCacheEnabled) { return; }
	WriterArchive->Serialize(TCHAR_TO_ANSI(*destination), destination.Len());
	WriterArchive->Serialize(TCHAR_TO_ANSI(*FString("\n")), 1);
	WriterArchive->Serialize(TCHAR_TO_ANSI(*body), body.Len());
	WriterArchive->Serialize(TCHAR_TO_ANSI(*FString("\n")), 1);

	numberWriteBatches++;
}

bool FLocalCache::PeekContent(FString& url, FString& content)
{
	if (NumberOfBatches() < 1) //nothing in 'read' file
	{
		if (numberWriteBatches > 0) //there is content in the write file
		{
			MergeDataFiles(); //merged write file into read file
			if (NumberOfBatches() > 1) //checks read file again
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
	readContent.RemoveAt(readContent.Num() - 1);
	readContent.RemoveAt(readContent.Num() - 1);
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
	bool result = FFileHelper::SaveStringArrayToFile(readContent, *readFilePath, FFileHelper::EEncodingOptions::ForceUTF8);
	if (result)
	{
	}
	else
	{
		FCognitiveLog::Error("FLocalCache::MergeDataFiles Error serialize readContents to file");
	}

	//clear write file
	WriterArchive = FileManager.CreateFileWriter(*writeFilePath);
	numberWriteBatches = 0;
}

void FLocalCache::SerializeToFile()
{
	if (WriterArchive != nullptr)
	{
		WriterArchive->Flush();
	}
	bool result = FFileHelper::SaveStringArrayToFile(readContent, *readFilePath, FFileHelper::EEncodingOptions::ForceUTF8);
	if (result)
	{

	}
	else
	{
		FCognitiveLog::Error("FLocalCache::SerializeToFile ERROR ReadArchive failed");
	}
}