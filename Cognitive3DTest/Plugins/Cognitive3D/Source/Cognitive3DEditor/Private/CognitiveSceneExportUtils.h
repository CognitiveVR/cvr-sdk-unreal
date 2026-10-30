#pragma once

#include "CoreMinimal.h"

namespace CognitiveSceneExportUtils
{
	COGNITIVE3DEDITOR_API bool NormalizeGLTFForUpload(const FString& FilePath, FString& OutError);

	COGNITIVE3DEDITOR_API bool UseMetricSceneUnits();
	COGNITIVE3DEDITOR_API float GetExportUniformScale();
	COGNITIVE3DEDITOR_API int32 GetSceneSettingsScale();
}
