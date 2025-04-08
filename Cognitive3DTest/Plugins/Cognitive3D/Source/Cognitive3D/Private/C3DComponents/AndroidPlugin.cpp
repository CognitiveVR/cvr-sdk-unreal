// Fill out your copyright notice in the Description page of Project Settings.


#include "C3DComponents/AndroidPlugin.h"
#include "Cognitive3D/Private/C3DUtil/Util.h"
#include "Cognitive3D/Public/Cognitive3D.h"
#include "Regex.h"
#include "Analytics.h"
#if PLATFORM_ANDROID
#include "Android/AndroidJNI.h"
#include "Android/AndroidApplication.h"
#endif

// Sets default values for this component's properties
UAndroidPlugin::UAndroidPlugin()
{
    // Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
    // off to improve performance if you don't need them.
    PrimaryComponentTick.bCanEverTick = true;

    // ...
}


// Called when the game starts
void UAndroidPlugin::BeginPlay()
{
    Super::BeginPlay();

    auto cognitive = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
    if (cognitive.IsValid())
    {
        cognitive->OnSessionBegin.AddDynamic(this, &UAndroidPlugin::OnSessionBegin);
        cognitive->OnPreSessionEnd.AddDynamic(this, &UAndroidPlugin::OnSessionEnd);
        if (cognitive->HasStartedSession())
        {
            OnSessionBegin();
        }
    }
}




// Called every frame
void UAndroidPlugin::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // ...
}

void UAndroidPlugin::OnSessionBegin()
{
    auto cognitive = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
    if (!cognitive.IsValid()) {
        return;
    }

    C3DSettingsPath = cognitive->GetSettingsFilePathRuntime();
	C3DKeysPath = cognitive->GetKeysFilePathRuntime();
    GConfig->LoadFile(C3DSettingsPath);
	GConfig->LoadFile(C3DKeysPath);

    FString AppKey = "APIKEY:DATA " + cognitive->ApplicationKey;
    FString DeviceId = cognitive->GetDeviceID();
    double timeStamp = FUtil::GetTimestamp();
    FString sessionID = cognitive->GetSessionID();
    FString trackingSceneID = cognitive->CurrentTrackingSceneId;
    FString trackingSceneVersionStr = cognitive->GetCurrentSceneVersionNumber();
    int trackingSceneVersion = FCString::Atoi(*trackingSceneVersionStr);
    FString Gateway = FAnalytics::Get().GetConfigValueFromIni(C3DKeysPath, "/Script/Cognitive3D.Cognitive3DSettings", "Gateway", false);
    FString gazeURL = "https://" + Gateway + "/v" + FString::FromInt(0) + "/gaze/" + cognitive->GetCurrentSceneId() + "?version=" + cognitive->GetCurrentSceneVersionNumber();
    FString eventsURL = "https://" + Gateway + "/v" + FString::FromInt(0) + "/events/" + cognitive->GetCurrentSceneId() + "?version=" + cognitive->GetCurrentSceneVersionNumber();

    // Set the Android plugin version info as a session property    
#if PLATFORM_ANDROID
    FString PluginVersionInfo = "Unknown";

    if (JNIEnv* Env = FAndroidApplication::GetJavaEnv()) {
        jobject GameActivity = FAndroidApplication::GetGameActivityThis();
        if (!GameActivity) {
            UE_LOG(LogTemp, Error, TEXT("Failed to get GameActivity instance"));
            return;
        }

        // Find the Plugin class
        jclass PluginClass = FAndroidApplication::FindJavaClass("com/c3d/androidjavaplugin/Plugin");
        if (!PluginClass) {
            UE_LOG(LogTemp, Error, TEXT("Failed to find class: com/c3d/androidjavaplugin.Plugin"));
            Env->ExceptionDescribe(); // Logs the error details
            Env->ExceptionClear();    // Clears the exception
            return;
        }

        // Locate the static method
        jmethodID GetVersionMethod = Env->GetStaticMethodID(PluginClass, "getAndroidPluginVersionInfo", "()Ljava/lang/String;");
        if (!GetVersionMethod) {
            UE_LOG(LogTemp, Error, TEXT("Failed to find getAndroidPluginVersionInfo method"));
            Env->ExceptionDescribe();
            Env->ExceptionClear();
            return;
        }

        // Call the static method
        jstring VersionInfo = (jstring)Env->CallStaticObjectMethod(PluginClass, GetVersionMethod);
        if (VersionInfo) {
            const char* VersionInfoChars = Env->GetStringUTFChars(VersionInfo, nullptr);
            PluginVersionInfo = FString(UTF8_TO_TCHAR(VersionInfoChars));
            Env->ReleaseStringUTFChars(VersionInfo, VersionInfoChars);
            UE_LOG(LogTemp, Warning, TEXT("Android Plugin Version: %s"), *PluginVersionInfo);
            cognitive->SetSessionProperty("AndroidPluginVersion", PluginVersionInfo);
        }
        else {
            UE_LOG(LogTemp, Error, TEXT("Failed to get Android Plugin Version"));
        }

        // Locate the getInstance method
        jmethodID GetInstanceMethod = Env->GetStaticMethodID(PluginClass, "getInstance", "(Landroid/app/Activity;)Lcom/c3d/androidjavaplugin/Plugin;");
        if (!GetInstanceMethod) {
            UE_LOG(LogTemp, Error, TEXT("Failed to find getInstance method"));
            Env->ExceptionDescribe();
            Env->ExceptionClear();
            return;
        }

        //

        jobject PluginInstance = Env->CallStaticObjectMethod(PluginClass, GetInstanceMethod, GameActivity);
        if (!PluginInstance) {
            UE_LOG(LogTemp, Error, TEXT("Failed to get Plugin instance"));
            return;
        }

        // Locate initSessionData method
        jmethodID InitSessionDataMethod = Env->GetMethodID(PluginClass, "initSessionData",
            "(Ljava/lang/String;Ljava/lang/String;DLjava/lang/String;Ljava/lang/String;ILjava/lang/String;Ljava/lang/String;)V");
        if (!InitSessionDataMethod) {
            UE_LOG(LogTemp, Error, TEXT("Failed to find initSessionData method"));
            Env->ExceptionDescribe();
            Env->ExceptionClear();
            return;
        }

        jstring AppKeyStr = Env->NewStringUTF(TCHAR_TO_UTF8(*AppKey));
        jstring DeviceIdStr = Env->NewStringUTF(TCHAR_TO_UTF8(*DeviceId));
        jstring SessionIdStr = Env->NewStringUTF(TCHAR_TO_UTF8(*sessionID));
        jstring TrackingSceneIdStr = Env->NewStringUTF(TCHAR_TO_UTF8(*trackingSceneID));
        jstring GazeURLStr = Env->NewStringUTF(TCHAR_TO_UTF8(*gazeURL));
        jstring EventsURLStr = Env->NewStringUTF(TCHAR_TO_UTF8(*eventsURL));

        Env->CallVoidMethod(PluginInstance, InitSessionDataMethod, AppKeyStr, DeviceIdStr, timeStamp, SessionIdStr, TrackingSceneIdStr, trackingSceneVersion, GazeURLStr, EventsURLStr);

        Env->DeleteLocalRef(AppKeyStr);
        Env->DeleteLocalRef(DeviceIdStr);
        Env->DeleteLocalRef(SessionIdStr);
        Env->DeleteLocalRef(TrackingSceneIdStr);
        Env->DeleteLocalRef(GazeURLStr);
        Env->DeleteLocalRef(EventsURLStr);

        UE_LOG(LogTemp, Warning, TEXT("Android Session Data Initialized"));


        // Construct file paths
        FolderPath = FPaths::Combine(FPaths::ProjectConfigDir(), TEXT("c3dlocal"));
        FolderPathCrashLog = FPaths::Combine(*FolderPath, TEXT("CrashLogs"));
        CurrentFilePath = FPaths::Combine(*FolderPathCrashLog, FString::Printf(TEXT("BackupCrashLog-%d.log"), FDateTime::Now().ToUnixTimestamp()));
        PreviousSessionFilePath = FPaths::Combine(*FolderPath, TEXT("PreviousSessionData.log"));
        WriteDataFilePath = FPaths::Combine(*FolderPath, TEXT("write.txt"));

        // Ensure the directory exists
        if (!FPaths::DirectoryExists(FolderPath)) {
            FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*FolderPath);
        }
        if (!FPaths::DirectoryExists(FolderPathCrashLog)) {
            FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*FolderPathCrashLog);
        }


        jmethodID InitializeMethod = Env->GetStaticMethodID(PluginClass, "initialize", "(Landroid/app/Activity;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");

        if (InitializeMethod) {
            // Convert file paths to jstrings
            jstring JCurrentFilePath = Env->NewStringUTF(TCHAR_TO_UTF8(*IFileManager::Get().ConvertToAbsolutePathForExternalAppForWrite(*CurrentFilePath)));
            jstring JPreviousSessionFilePath = Env->NewStringUTF(TCHAR_TO_UTF8(*IFileManager::Get().ConvertToAbsolutePathForExternalAppForWrite(*PreviousSessionFilePath)));
            jstring JWriteDataFilePath = Env->NewStringUTF(TCHAR_TO_UTF8(*IFileManager::Get().ConvertToAbsolutePathForExternalAppForWrite(*WriteDataFilePath)));

            Env->CallStaticVoidMethod(PluginClass, InitializeMethod, GameActivity, JCurrentFilePath, JPreviousSessionFilePath, JWriteDataFilePath);

            // Clean up
            Env->DeleteLocalRef(JCurrentFilePath);
            Env->DeleteLocalRef(JPreviousSessionFilePath);
            Env->DeleteLocalRef(JWriteDataFilePath);
        }
        else {
            UE_LOG(LogTemp, Error, TEXT("Failed to find initialize method in Plugin"));
        }

        UE_LOG(LogTemp, Warning, TEXT("Android Plugin Initialized"));


        Env->DeleteLocalRef(PluginClass);
    }
#endif


    //call LogFileHasContent
    LogFileHasContent();

    //bind function to level load process
    FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UAndroidPlugin::OnLevelLoad);

}

void UAndroidPlugin::OnSessionEnd()
{
}

void UAndroidPlugin::OnLevelLoad(UWorld* world)
{
    auto cognitive = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
    if (!cognitive.IsValid()) {
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("UAndroidPlugin: Level loaded"));

    auto level = world->GetCurrentLevel();
    if (level == nullptr)
    {
        return;
    }

    FString levelName = level->GetFullGroupName(true);
    auto currentSceneData = cognitive->GetCurrentSceneData();


    TSharedPtr<FSceneData> data = cognitive->GetSceneData(levelName);

    //if the new scene is the same as the current scene, DONT return
    if (cognitive->LastSceneData.IsValid() && data.IsValid() && cognitive->LastSceneData->Id == data->Id)
    {
        UE_LOG(LogTemp, Warning, TEXT("UAndroidPlugin: Scene is the same as the current scene"));
        //return;
    }

    if (cognitive->LastSceneData.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("UAndroidPlugin: Scene changed"));
        FString trackingSceneID = cognitive->CurrentTrackingSceneId;
        FString trackingSceneVersionStr = cognitive->GetCurrentSceneVersionNumber();
        int trackingSceneVersion = FCString::Atoi(*trackingSceneVersionStr);
        FString Gateway = FAnalytics::Get().GetConfigValueFromIni(C3DKeysPath, "/Script/Cognitive3D.Cognitive3DSettings", "Gateway", false);
        FString gazeURL = "https://" + Gateway + "/v" + FString::FromInt(0) + "/gaze/" + cognitive->GetCurrentSceneId() + "?version=" + cognitive->GetCurrentSceneVersionNumber();
        FString eventsURL = "https://" + Gateway + "/v" + FString::FromInt(0) + "/events/" + cognitive->GetCurrentSceneId() + "?version=" + cognitive->GetCurrentSceneVersionNumber();

#if PLATFORM_ANDROID

        if (JNIEnv* Env = FAndroidApplication::GetJavaEnv()) {
            jobject GameActivity = FAndroidApplication::GetGameActivityThis();
            if (!GameActivity) {
                UE_LOG(LogTemp, Error, TEXT("Failed to get GameActivity instance"));
                return;
            }

            UE_LOG(LogTemp, Warning, TEXT("UAndroidPlugin: Calling onTrackingSceneChanged"));

            // Find the Plugin class
            jclass PluginClass = FAndroidApplication::FindJavaClass("com/c3d/androidjavaplugin/Plugin");
            if (!PluginClass) {
                UE_LOG(LogTemp, Error, TEXT("Failed to find class: com/c3d/androidjavaplugin.Plugin"));
                Env->ExceptionDescribe();
                Env->ExceptionClear();
                return;
            }

            // Locate the getInstance method
            jmethodID GetInstanceMethod = Env->GetStaticMethodID(PluginClass, "getInstance", "(Landroid/app/Activity;)Lcom/c3d/androidjavaplugin/Plugin;");
            if (!GetInstanceMethod) {
                UE_LOG(LogTemp, Error, TEXT("Failed to find getInstance method"));
                Env->ExceptionDescribe();
                Env->ExceptionClear();
                return;
            }

            // Get the Plugin instance
            jobject PluginInstance = Env->CallStaticObjectMethod(PluginClass, GetInstanceMethod, GameActivity);
            if (!PluginInstance) {
                UE_LOG(LogTemp, Error, TEXT("Failed to get Plugin instance"));
                return;
            }

            // Locate the onTrackingSceneChanged method
            jmethodID OnTrackingSceneChangedMethod = Env->GetMethodID(PluginClass, "onTrackingSceneChanged", "(Ljava/lang/String;ILjava/lang/String;Ljava/lang/String;)V");
            if (!OnTrackingSceneChangedMethod) {
                UE_LOG(LogTemp, Error, TEXT("Failed to find onTrackingSceneChanged method"));
                Env->ExceptionDescribe();
                Env->ExceptionClear();
                return;
            }

            // Prepare method parameters
            jstring TrackingSceneIdStr = Env->NewStringUTF(TCHAR_TO_UTF8(*trackingSceneID));
            jstring GazeURLStr = Env->NewStringUTF(TCHAR_TO_UTF8(*gazeURL));
            jstring EventsURLStr = Env->NewStringUTF(TCHAR_TO_UTF8(*eventsURL));

            // Call the method
            Env->CallVoidMethod(PluginInstance, OnTrackingSceneChangedMethod, TrackingSceneIdStr, trackingSceneVersion, EventsURLStr, GazeURLStr);

            // Clean up local references
            Env->DeleteLocalRef(TrackingSceneIdStr);
            Env->DeleteLocalRef(EventsURLStr);
            Env->DeleteLocalRef(GazeURLStr);
            Env->DeleteLocalRef(PluginInstance);
            Env->DeleteLocalRef(PluginClass);

            UE_LOG(LogTemp, Warning, TEXT("Successfully called onTrackingSceneChanged"));
        }

#endif

    }
}

void UAndroidPlugin::LogFileHasContent()
{
    auto cognitive = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
    if (!cognitive.IsValid()) {
        return;
    }

#if PLATFORM_ANDROID

    if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
    {
        jobject GameActivity = FAndroidApplication::GetGameActivityThis();
        if (!GameActivity)
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to get GameActivity instance"));
            return;
        }

        // Find the Plugin class
        jclass PluginClass = FAndroidApplication::FindJavaClass("com/c3d/androidjavaplugin/Plugin");
        if (!PluginClass)
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to find class: com/c3d/androidjavaplugin.Plugin"));
            Env->ExceptionDescribe();
            Env->ExceptionClear();
            return;
        }

        // Get the Plugin instance
        jmethodID GetInstanceMethod = Env->GetStaticMethodID(PluginClass, "getInstance", "(Landroid/app/Activity;)Lcom/c3d/androidjavaplugin/Plugin;");
        if (!GetInstanceMethod)
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to find getInstance method"));
            Env->ExceptionDescribe();
            Env->ExceptionClear();
            return;
        }

        jobject PluginInstance = Env->CallStaticObjectMethod(PluginClass, GetInstanceMethod, GameActivity);
        if (!PluginInstance)
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to get Plugin instance"));
            return;
        }

        // Check if CrashLogs folder exists
        if (FPaths::DirectoryExists(FolderPathCrashLog))
        {
            TArray<FString> Files;
            IFileManager::Get().FindFiles(Files, *FolderPathCrashLog, TEXT("*.log"));

            for (const FString& File : Files)
            {
                FString FilePath = FPaths::Combine(FolderPathCrashLog, File);

                TArray<FString> Lines;
                if (FFileHelper::LoadFileToStringArray(Lines, *FilePath))
                {
                    // If crash log contains required data
                    if (Lines.Num() >= 6 && !Lines[5].IsEmpty())
                    {
                        FString CrashTimestamp;
                        if (Lines.Num() >= 7)
                        {
                            CrashTimestamp = ExtractTimestampFromLine(Lines[6]);
                        }
                        else
                        {
                            CrashTimestamp = ExtractTimestampFromLine(Lines[5]);
                        }

                        if (!CrashTimestamp.IsEmpty())
                        {
                            // Process the extracted CrashTimestamp
                            UE_LOG(LogTemp, Log, TEXT("Extracted CrashTimestamp: %s"), *CrashTimestamp);
                        }
                        else
                        {
                            UE_LOG(LogTemp, Error, TEXT("Failed to extract a valid timestamp from the log file: %s"), *FilePath);
                            continue;
                        }

                        // Generate Event and Gaze URLs
                        FString Gateway = FAnalytics::Get().GetConfigValueFromIni(C3DKeysPath, "/Script/Cognitive3D.Cognitive3DSettings", "Gateway", false);

                        FString SceneID = Lines[3];
                        FString VersionNumber = Lines[4];

                        FString GazeURL = FString::Printf(TEXT("https://%s/v0/gaze/%s?version=%s"), *Gateway, *SceneID, *VersionNumber);
                        FString EventsURL = FString::Printf(TEXT("https://%s/v0/events/%s?version=%s"), *Gateway, *SceneID, *VersionNumber);

                        // Call sendCrashEvents
                        jmethodID SendCrashEventsMethod = Env->GetMethodID(
                            PluginClass,
                            "sendCrashEvents",
                            "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V"
                        );

                        if (SendCrashEventsMethod)
                        {
                            jstring JSessionID = Env->NewStringUTF(TCHAR_TO_UTF8(*Lines[0]));
                            jstring JUserID = Env->NewStringUTF(TCHAR_TO_UTF8(*Lines[1]));
                            jstring JTimestamp = Env->NewStringUTF(TCHAR_TO_UTF8(*Lines[2]));
                            jstring JCrashTimestamp = Env->NewStringUTF(TCHAR_TO_UTF8(*CrashTimestamp));
                            jstring JEventURL = Env->NewStringUTF(TCHAR_TO_UTF8(*EventsURL));
                            // Concatenate lines starting from index 5 into a single string with newlines
                            FString StackTrace;
                            if (Lines.Num() > 5)
                            {
                                for (int32 i = 5; i < Lines.Num(); ++i)
                                {
                                    if (!StackTrace.IsEmpty())
                                    {
                                        StackTrace += TEXT("\n");
                                    }
                                    StackTrace += Lines[i];
                                }
                            }
                            jstring JStackTrace = Env->NewStringUTF(TCHAR_TO_UTF8(*StackTrace));


                            FString AbsoluteFilePath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForWrite(*FilePath);
                            jstring JFilePath = Env->NewStringUTF(TCHAR_TO_UTF8(*AbsoluteFilePath));

                            Env->CallVoidMethod(PluginInstance, SendCrashEventsMethod, JSessionID, JUserID, JTimestamp, JCrashTimestamp, JEventURL, JStackTrace, JFilePath);

                            Env->DeleteLocalRef(JSessionID);
                            Env->DeleteLocalRef(JUserID);
                            Env->DeleteLocalRef(JTimestamp);
                            Env->DeleteLocalRef(JCrashTimestamp);
                            Env->DeleteLocalRef(JEventURL);
                            Env->DeleteLocalRef(JStackTrace);
                            Env->DeleteLocalRef(JFilePath);

                            UE_LOG(LogTemp, Log, TEXT("sendCrashEvents called for file: %s"), *FilePath);
                        }

                        // Call sendCrashGaze
                        jmethodID SendCrashGazeMethod = Env->GetMethodID(
                            PluginClass,
                            "sendCrashGaze",
                            "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V"
                        );

                        if (SendCrashGazeMethod)
                        {
                            jstring JSessionID = Env->NewStringUTF(TCHAR_TO_UTF8(*Lines[0]));
                            jstring JUserID = Env->NewStringUTF(TCHAR_TO_UTF8(*Lines[1]));
                            jstring JTimestamp = Env->NewStringUTF(TCHAR_TO_UTF8(*Lines[2]));
                            jstring JGazeURL = Env->NewStringUTF(TCHAR_TO_UTF8(*GazeURL));

                            Env->CallVoidMethod(PluginInstance, SendCrashGazeMethod, JSessionID, JUserID, JTimestamp, JGazeURL);

                            Env->DeleteLocalRef(JSessionID);
                            Env->DeleteLocalRef(JUserID);
                            Env->DeleteLocalRef(JTimestamp);
                            Env->DeleteLocalRef(JGazeURL);

                            UE_LOG(LogTemp, Log, TEXT("sendCrashGaze called for file: %s"), *FilePath);
                        }
                    }
                    else
                    {
                        // If it's not current session crash log file and has no crash logs, delete it
                        if (FilePath != CurrentFilePath)
                        {
                            jmethodID DeleteLogFileMethod = Env->GetMethodID(PluginClass, "deleteLogFile", "(Ljava/lang/String;)V");

                            if (DeleteLogFileMethod)
                            {
                                FString AbsoluteFilePath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForWrite(*FilePath);
                                jstring JFilePath = Env->NewStringUTF(TCHAR_TO_UTF8(*AbsoluteFilePath));
                                Env->CallVoidMethod(PluginInstance, DeleteLogFileMethod, JFilePath);
                                Env->DeleteLocalRef(JFilePath);

                                UE_LOG(LogTemp, Log, TEXT("deleteLogFile called for file: %s"), *FilePath);
                            }
                        }
                    }
                }
            }
        }

        // Check previous session file
        TArray<FString> PreviousSessionLines;
        if (FFileHelper::LoadFileToStringArray(PreviousSessionLines, *PreviousSessionFilePath))
        {
            if (PreviousSessionLines.Num() > 0)
            {
                FString SceneID = PreviousSessionLines[3];
                FString VersionNumber = PreviousSessionLines[4];
                FString Gateway = FAnalytics::Get().GetConfigValueFromIni(C3DKeysPath, "/Script/Cognitive3D.Cognitive3DSettings", "Gateway", false);

                FString EventsURL = FString::Printf(TEXT("https://%s/v0/events/%s?version=%s"), *Gateway, *SceneID, *VersionNumber);

                jmethodID SendEndSessionEventsMethod = Env->GetMethodID(
                    PluginClass,
                    "sendEndSessionEvents",
                    "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V"
                );

                if (SendEndSessionEventsMethod)
                {
                    jstring JSessionID = Env->NewStringUTF(TCHAR_TO_UTF8(*PreviousSessionLines[0]));
                    jstring JUserID = Env->NewStringUTF(TCHAR_TO_UTF8(*PreviousSessionLines[1]));
                    jstring JTimestamp = Env->NewStringUTF(TCHAR_TO_UTF8(*PreviousSessionLines[2]));
                    jstring JEventURL = Env->NewStringUTF(TCHAR_TO_UTF8(*EventsURL));

                    Env->CallVoidMethod(PluginInstance, SendEndSessionEventsMethod, JSessionID, JUserID, JTimestamp, JEventURL);

                    Env->DeleteLocalRef(JSessionID);
                    Env->DeleteLocalRef(JUserID);
                    Env->DeleteLocalRef(JTimestamp);
                    Env->DeleteLocalRef(JEventURL);

                    UE_LOG(LogTemp, Log, TEXT("sendEndSessionEvents called for previous session file."));
                }
            }
        }

        // Call writeSessionDataIntoLogFile for the current session
        jmethodID WriteSessionDataMethod = Env->GetMethodID(PluginClass, "writeSessionDataIntoLogFile", "(Ljava/lang/String;Z)V");
        if (WriteSessionDataMethod)
        {
            jstring JPreviousSessionFilePath = Env->NewStringUTF(TCHAR_TO_UTF8(*IFileManager::Get().ConvertToAbsolutePathForExternalAppForWrite(*PreviousSessionFilePath)));
            jboolean Append = false;

            Env->CallVoidMethod(PluginInstance, WriteSessionDataMethod, JPreviousSessionFilePath, Append);
            Env->DeleteLocalRef(JPreviousSessionFilePath);

            UE_LOG(LogTemp, Log, TEXT("writeSessionDataIntoLogFile called for current session."));
        }

        // Clean up
        Env->DeleteLocalRef(PluginInstance);
        Env->DeleteLocalRef(PluginClass);
    }

#endif

}

FString UAndroidPlugin::ExtractTimestampFromLine(const FString& Line)
{
    // Define a regex pattern to extract the timestamp at the start of the line
    const FRegexPattern TimestampPattern(TEXT(R"(^\s*(\d+\.\d+))"));
    FRegexMatcher Matcher(TimestampPattern, Line);

    if (Matcher.FindNext())
    {
        // Return the first match (timestamp)
        return Matcher.GetCaptureGroup(1);
    }

    return FString(); // Return empty string if no match found
}

