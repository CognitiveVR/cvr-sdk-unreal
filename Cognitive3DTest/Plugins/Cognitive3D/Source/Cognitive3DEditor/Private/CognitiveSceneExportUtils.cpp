#include "CognitiveSceneExportUtils.h"

#include "Misc/ConfigCacheIni.h"
#include "Misc/Paths.h"

bool CognitiveSceneExportUtils::UseMetricSceneUnits()
{
	const FString SettingsPath = FPaths::Combine(FPaths::ProjectConfigDir(), TEXT("c3dlocal"), TEXT("Cognitive3DSettings.ini"));
	FConfigFile Settings;
	Settings.Read(SettingsPath);
	FString Value;
	return Settings.GetString(TEXT("/Script/Cognitive3D.Cognitive3DSettings"), TEXT("MetricSceneUnits"), Value) && Value.ToBool();
}

float CognitiveSceneExportUtils::GetExportUniformScale()
{
	return UseMetricSceneUnits() ? 0.01f : 1.0f;
}

int32 CognitiveSceneExportUtils::GetSceneSettingsScale()
{
	return UseMetricSceneUnits() ? 1 : 100;
}
