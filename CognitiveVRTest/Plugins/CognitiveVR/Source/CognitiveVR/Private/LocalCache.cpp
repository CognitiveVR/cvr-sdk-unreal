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
	GLog->Log("LocalCache::Close");
	if (WriterArchive != nullptr)
	{
		GLog->Log("LocalCache::Close WriterArchive");
		WriterArchive->Flush();
		WriterArchive->Close();
		delete WriterArchive;
		WriterArchive = nullptr;
	}
	bool result = FFileHelper::SaveStringArrayToFile(readContent, *readFilePath, FFileHelper::EEncodingOptions::ForceUTF8);
	if (result)
	{
		
	}
	else
	{
		GLog->Log("LocalCache::Close readContent to file");
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
	WriterArchive->Serialize("\n", 1);
	WriterArchive->Serialize(TCHAR_TO_ANSI(*body), body.Len());
	WriterArchive->Serialize("\n", 1);

	numberWriteBatches++;
	GLog->Log("LOCALCACHE Write data");
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
		GLog->Log("LocalCache::MergeDataFiles ERROR readContent not empty");
	}

	//close writer stream
	if (WriterArchive != nullptr)
	{
		GLog->Log("LocalCache::MergeDataFiles WriterArchive flush and close");
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
		GLog->Log("LocalCache::MergeDataFiles write file error");
	}

	//read data from 'read' file
	bool readResult = FFileHelper::LoadFileToStringArray(readContent, *readFilePath);
	if (readResult)
	{
	}
	else
	{
		GLog->Log("LocalCache::MergeDataFiles read file error");
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
		GLog->Log("LocalCache::MergeDataFiles Error serialize readContents to file");
	}

	//clear write file
	WriterArchive = FileManager.CreateFileWriter(*writeFilePath);
	GLog->Log("MERGE " + FString::FromInt(readContent.Num()));
	numberWriteBatches = 0;
}