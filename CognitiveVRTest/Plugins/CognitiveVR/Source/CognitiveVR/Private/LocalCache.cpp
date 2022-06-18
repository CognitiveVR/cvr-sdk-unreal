/*
** Copyright (c) 2016 CognitiveVR, Inc. All rights reserved.
*/

#include "LocalCache.h"

LocalCache::LocalCache(FString path)
{
	//path to project config dir
	writeFilePath = path + "write.txt";
	readFilePath = path + "read.txt";

	MergeDataFiles();


#if PLATFORM_ANDROID

#elif PLATFORM_WINDOWS

#endif
}

int32 LocalCache::NumberOfBatches()
{
	return readContent.Num() / 2;
}
bool LocalCache::HasContent()
{
	return readContent.Num() > 0;
}

//called on session end. close filestream, serialize any outstanding data batches to disk
void LocalCache::Close()
{
	SerializeToFile();
	if (WriterArchive != nullptr)
	{
		WriterArchive->Close();
		delete WriterArchive;
		WriterArchive = nullptr;
	}
}

//TODO check for max file size
bool LocalCache::CanWrite()
{
	return true;
}

void LocalCache::WriteData(FString destination, FString body)
{
	if (WriterArchive == nullptr) { return; }
	WriterArchive->Serialize(TCHAR_TO_ANSI(*destination), destination.Len());
	WriterArchive->Serialize(TCHAR_TO_ANSI(*FString("\n")), 1);
	WriterArchive->Serialize(TCHAR_TO_ANSI(*body), body.Len());
	WriterArchive->Serialize(TCHAR_TO_ANSI(*FString("\n")), 1);

	numberWriteBatches++;
}

bool LocalCache::PeekContent(FString& url, FString& content)
{
	if (NumberOfBatches() < 1) //nothing in 'read' file
	{
		if (numberWriteBatches > 0) //there is content in the write file
		{
			MergeDataFiles();
			if (NumberOfBatches() > 1)
			{
				url = readContent[readContent.Num() - 2];
				content = readContent[readContent.Num() - 1];

				return true;
			}
			else //merge resulted in no new data in read file
			{
				CognitiveLog::Error("Merging Local Cache files failed?");
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
		url = readContent[readContent.Num() - 2];
		content = readContent[readContent.Num() - 1];
		return true;
	}
}

void LocalCache::PopContent()
{
	readContent.RemoveAt(readContent.Num() - 1);
	readContent.RemoveAt(readContent.Num() - 1);
}


void LocalCache::MergeDataFiles()
{
	if (readContent.Num() > 0)
	{
		CognitiveLog::Error("LocalCache::MergeDataFiles ERROR readContent not empty");
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
		CognitiveLog::Error("LocalCache::MergeDataFiles write file error");
	}

	//read data from 'read' file
	bool readResult = FFileHelper::LoadFileToStringArray(readContent, *readFilePath);
	if (readResult)
	{
	}
	else
	{
		CognitiveLog::Error("LocalCache::MergeDataFiles read file error");
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
		CognitiveLog::Error("LocalCache::MergeDataFiles Error serialize readContents to file");
	}

	//clear write file
	WriterArchive = FileManager.CreateFileWriter(*writeFilePath);
	numberWriteBatches = 0;
}

void LocalCache::SerializeToFile()
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
		CognitiveLog::Error("LocalCache::SerializeToFile ERROR ReadArchive failed");
	}
}