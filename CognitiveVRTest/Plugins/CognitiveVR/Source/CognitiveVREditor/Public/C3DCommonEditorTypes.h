// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "C3DCommonEditorTypes.generated.h"

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