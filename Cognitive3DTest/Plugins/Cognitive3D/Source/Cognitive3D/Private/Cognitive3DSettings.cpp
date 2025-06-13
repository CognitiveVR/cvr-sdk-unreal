/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/

#include "Cognitive3DSettings.h"

#define LOCTEXT_NAMESPACE "Cognitive3DLoc"

UCognitive3DSettings::UCognitive3DSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SettingsDisplayName = LOCTEXT("SettingsDisplayName", "Cognitive 3D");
	SettingsTooltip = LOCTEXT("SettingsTooltip", "Cognitive 3D analytics configuration settings");
}

void UCognitive3DSettings::ReadConfigSettings()
{
	const FString CustomConfigFilePath = GetIniName();

	GConfig->GetBool(TEXT("/Script/Cognitive3D.Cognitive3DSettings"), TEXT("EnableLogging"), EnableLogging, CustomConfigFilePath);
	GConfig->GetString(TEXT("/Script/Cognitive3D.Cognitive3DSettings"), TEXT("Gateway"), Gateway, CustomConfigFilePath);
	GConfig->GetBool(TEXT("/Script/Cognitive3D.Cognitive3DSettings"), TEXT("EnableLocalCache"), EnableLocalCache, CustomConfigFilePath);
	GConfig->GetInt(TEXT("/Script/Cognitive3D.Cognitive3DSettings"), TEXT("LocalCacheSize"), LocalCacheSize, CustomConfigFilePath);
	GConfig->GetInt(TEXT("/Script/Cognitive3D.Cognitive3DSettings"), TEXT("GazeBatchSize"), GazeBatchSize, CustomConfigFilePath);
	GConfig->GetInt(TEXT("/Script/Cognitive3D.Cognitive3DSettings"), TEXT("CustomEventBatchSize"), CustomEventBatchSize, CustomConfigFilePath);
	GConfig->GetInt(TEXT("/Script/Cognitive3D.Cognitive3DSettings"), TEXT("CustomEventAutoTimer"), CustomEventAutoTimer, CustomConfigFilePath);
	GConfig->GetInt(TEXT("/Script/Cognitive3D.Cognitive3DSettings"), TEXT("DynamicDataLimit"), DynamicDataLimit, CustomConfigFilePath);
	GConfig->GetInt(TEXT("/Script/Cognitive3D.Cognitive3DSettings"), TEXT("DynamicAutoTimer"), DynamicAutoTimer, CustomConfigFilePath);
	GConfig->GetInt(TEXT("/Script/Cognitive3D.Cognitive3DSettings"), TEXT("SensorDataLimit"), SensorDataLimit, CustomConfigFilePath);
	GConfig->GetInt(TEXT("/Script/Cognitive3D.Cognitive3DSettings"), TEXT("SensorAutoTimer"), SensorAutoTimer, CustomConfigFilePath);
}

void UCognitive3DSettings::WriteConfigSettings()
{
	const FString CustomConfigFilePath = GetIniName();

	GConfig->SetBool(TEXT("/Script/Cognitive3D.Cognitive3DSettings"), TEXT("EnableLogging"), EnableLogging, CustomConfigFilePath);
	GConfig->SetString(TEXT("/Script/Cognitive3D.Cognitive3DSettings"), TEXT("Gateway"), *Gateway, CustomConfigFilePath);
	GConfig->SetBool(TEXT("/Script/Cognitive3D.Cognitive3DSettings"), TEXT("EnableLocalCache"), EnableLocalCache, CustomConfigFilePath);
	GConfig->SetInt(TEXT("/Script/Cognitive3D.Cognitive3DSettings"), TEXT("LocalCacheSize"), LocalCacheSize, CustomConfigFilePath);
	GConfig->SetInt(TEXT("/Script/Cognitive3D.Cognitive3DSettings"), TEXT("GazeBatchSize"), GazeBatchSize, CustomConfigFilePath);
	GConfig->SetInt(TEXT("/Script/Cognitive3D.Cognitive3DSettings"), TEXT("CustomEventBatchSize"), CustomEventBatchSize, CustomConfigFilePath);
	GConfig->SetInt(TEXT("/Script/Cognitive3D.Cognitive3DSettings"), TEXT("CustomEventAutoTimer"), CustomEventAutoTimer, CustomConfigFilePath);
	GConfig->SetInt(TEXT("/Script/Cognitive3D.Cognitive3DSettings"), TEXT("DynamicDataLimit"), DynamicDataLimit, CustomConfigFilePath);
	GConfig->SetInt(TEXT("/Script/Cognitive3D.Cognitive3DSettings"), TEXT("DynamicAutoTimer"), DynamicAutoTimer, CustomConfigFilePath);
	GConfig->SetInt(TEXT("/Script/Cognitive3D.Cognitive3DSettings"), TEXT("SensorDataLimit"), SensorDataLimit, CustomConfigFilePath);
	GConfig->SetInt(TEXT("/Script/Cognitive3D.Cognitive3DSettings"), TEXT("SensorAutoTimer"), SensorAutoTimer, CustomConfigFilePath);
	// ... repeat for each setting

	GConfig->Flush(false, CustomConfigFilePath);
}

FString UCognitive3DSettings::GetIniName() const
{
	return FPaths::Combine(FPaths::ProjectConfigDir(), TEXT("c3dlocal/Cognitive3DSettings.ini"));
}


#undef LOCTEXT_NAMESPACE