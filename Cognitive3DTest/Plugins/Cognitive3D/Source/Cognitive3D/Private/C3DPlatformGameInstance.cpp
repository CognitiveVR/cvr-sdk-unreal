// Fill out your copyright notice in the Description page of Project Settings.


#include "C3DPlatformGameInstance.h"
#include "C3DCommonTypes.h"
#include "Cognitive3D/Public/Cognitive3D.h"
#ifdef __ANDROID__
#include "Android/AndroidApplication.h"
#endif
#ifdef INCLUDE_OCULUS_PLATFORM
#include "OVR_Platform.h"
#endif

void UC3DPlatformGameInstance::Init()
{
    Super::Init();
    // Additional checks for success or logging can be added here
#ifdef INCLUDE_OCULUS_PLATFORM
#ifdef __ANDROID__
    JNIEnv* Env = FAndroidApplication::GetJavaEnv();
    jobject Activity = FAndroidApplication::GetGameActivityThis();

    FString EngineIni = FPaths::Combine(*(FPaths::ProjectDir()), TEXT("Config/DefaultEngine.ini"));
    FString OculusAppId = FAnalytics::Get().GetConfigValueFromIni(EngineIni, "OnlineSubsystemOculus", "OculusAppId", false);
    auto tempAnsi = StringCast<ANSICHAR>(*OculusAppId);
    const char* AppID = tempAnsi.Get();

    if (Env != nullptr && Activity != nullptr)
    {
        ovrPlatformInitializeResult result = ovr_PlatformInitializeAndroid(AppID, Activity, Env);
        if (result != ovrPlatformInitialize_Success)
        {
            // Handle initialization failure
            UE_LOG(LogTemp, Error, TEXT("Failed to initialize Oculus Platform, Error code: %d"), result);
        }
        else
        {
            // Initialization successful
            UE_LOG(LogTemp, Log, TEXT("Oculus Platform initialized successfully"));
        }
    }
    else
    {
        // Failed to obtain JNI Environment or Activity
        UE_LOG(LogTemp, Error, TEXT("Failed to obtain JNI Environment or Android Activity"));
    }
#endif
#endif
}