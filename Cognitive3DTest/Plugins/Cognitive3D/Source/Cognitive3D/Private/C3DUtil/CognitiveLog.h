/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/
#pragma once

#include "Cognitive3D/Public/Cognitive3D.h"

    class FCognitiveLog
    {
	private:
		static bool ShowDebugLogs; //basic info/warning/errors
		static bool ShowDevLogs; //development specific logs
		static bool HasInitialized;

        public:
			static void Init();
			static void DevLog(FString s);
			static void Info(FString s);
            static void Warning(FString s);
            static void Error(FString s);

			static bool DevLogEnabled();
    };
