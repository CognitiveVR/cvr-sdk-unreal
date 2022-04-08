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

	//create write file
	WriterArchive = FileManager.CreateFileWriter(*writeFilePath);

	//load read file
	if (FileManager.FileExists(*readFilePath))
	{
		GLog->Log("read file exists");
		bool loadResult = FFileHelper::LoadFileToStringArray(readContent, *readFilePath);
		if (loadResult)
		{
			for (auto& line : readContent)
			{
				GLog->Log(line);
			}
		}
		else
		{
			GLog->Log("load error");
		}
	}
	else
	{
		GLog->Log("file does not exist");
	}


#if PLATFORM_ANDROID

#elif PLATFORM_WINDOWS

#endif
}

int32 LocalCache::NumberOfBatches()
{
	//read content lines / 2
	//+ write content lines / 2
	return 0;
}
bool LocalCache::HasContent()
{
	return readContent.Num() > 0;
}


//called on session end. close filestream, serialize any outstanding data batches to disk
void LocalCache::Close()
{
	WriterArchive->Flush();
	WriterArchive->Close();
	delete WriterArchive;
}

//check for max file size
bool LocalCache::CanWrite()
{
	return true;
}

void LocalCache::WriteData(FString destination, FString body)
{
	WriterArchive->Serialize(TCHAR_TO_ANSI(*destination), destination.Len());
	WriterArchive->Serialize("\n", 1);
	WriterArchive->Serialize(TCHAR_TO_ANSI(*body), body.Len());
	WriterArchive->Serialize("\n", 1);
}

bool LocalCache::PeekContent(FString* url, FString* content)
{
	if (NumberOfBatches() < 2)
	{
		MergeDataFiles();
		if (NumberOfBatches() < 2)
		{
			return false;
		}
	}

	url = &readContent[readContent.Num() - 2];
	content = &readContent[readContent.Num() - 1];
	return true;
}

void LocalCache::PopContent()
{
	readContent.RemoveAt(readContent.Num() - 1);
	readContent.RemoveAt(readContent.Num() - 1);
}


void LocalCache::MergeDataFiles()
{
	//read from 'writable', write into 'readable' file
	FString content;
	WriterArchive->Flush();
	WriterArchive->Close();
	delete WriterArchive;


	//read data from 'write' file to memory
	TArray<FString> writeContent;
	bool writeResult = FFileHelper::LoadFileToStringArray(writeContent, *writeFilePath);
	if (writeResult)
	{
		for (auto& line : writeContent)
		{
			GLog->Log(line);
		}
	}
	else
	{
		GLog->Log("load error");
	}

	//read data from 'read' file
	bool readResult = FFileHelper::LoadFileToStringArray(readContent, *readFilePath);
	if (readResult)
	{
		for (auto& line : readContent)
		{
			GLog->Log(line);
		}
	}
	else
	{
		GLog->Log("load error");
	}

	//combine
	readContent.Append(writeContent);

	//serialize read to file
	bool result = FFileHelper::SaveStringToFile(content, *readFilePath, FFileHelper::EEncodingOptions::ForceUTF8);
	if (result)
	{
		GLog->Log("serialize successful");
	}
	else
	{
		GLog->Log("serialize error");
	}

	//clear write file
	WriterArchive = FileManager.CreateFileWriter(*writeFilePath);
	//TODO test that default write flag clears file contents
}