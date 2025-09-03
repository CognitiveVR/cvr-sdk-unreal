/*
** Copyright (c) 2024 Cognitive3D, Inc. All rights reserved.
*/

#pragma once

#include "http.h"
#include "C3DCommonEditorTypes.generated.h"

DECLARE_DELEGATE_ThreeParams(FOnUploadSceneGeometry, FHttpRequestPtr, FHttpResponsePtr, bool)

DECLARE_DELEGATE_OneParam(FOnUploadAllDynamics, bool)
DECLARE_DELEGATE_OneParam(FOnUploadAllSceneGeometry, bool)
DECLARE_DELEGATE_OneParam(FOnExportAllSceneGeometry, bool)

USTRUCT()
struct FApplicationKeyResponse
{
	GENERATED_BODY()

public:
	UPROPERTY()
		FString apikey;

	UPROPERTY()
		bool valid;
};

USTRUCT()
struct FSubscriptionDataResponse
{
	GENERATED_BODY()

public:
	UPROPERTY()
		int64 beginning;
	UPROPERTY()
		int64 expiration;
	UPROPERTY()
		FString planType;
	UPROPERTY()
		bool isFreeTrial;
};

USTRUCT()
struct FOrganizationDataResponse
{
	GENERATED_BODY()

public:
	UPROPERTY()
		FString organizationName;
	UPROPERTY()
		TArray< FSubscriptionDataResponse> subscriptions;
};

USTRUCT()
struct FDashboardObject
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FString sdkId;
	UPROPERTY()
	FString name;
	UPROPERTY()
	FString meshName;
	UPROPERTY()
	int64 updatedAt;
};