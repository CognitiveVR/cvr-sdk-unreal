// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "C3DCommonTypes.h"
#include "Cognitive3D/Private/C3DUtil/Util.h"
#include "Cognitive3D/Public/Cognitive3D.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "RemoteControls.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class URemoteControls : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	URemoteControls();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	TSharedPtr<FAnalyticsProviderCognitive3D> cog;

	UFUNCTION()
	void OnSessionBegin();
	UFUNCTION()
	void OnSessionEnd();

	UFUNCTION(BlueprintCallable, Category = "Remote Control")
	void QueryRemoteControlVariable(FString UserID);

	void OnHttpResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	void ParseJsonResponse(const FString& JsonResponse);

	TMap<FString, FString> RemoteControlVariablesString;
	TMap<FString, int32> RemoteControlVariablesInt;
	TMap<FString, float> RemoteControlVariablesFloat;
	TMap<FString, bool> RemoteControlVariablesBool;

	template <typename T>
	T GetRemoteControlVariable(FString key, T defaultValue);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRemoteControlVariableReceived);

	UFUNCTION(BlueprintCallable, Category = "Remote Control")
	int32 GetRemoteControlVariableInt(const FString& Key, int32 DefaultValue);

	UFUNCTION(BlueprintCallable, Category = "Remote Control")
	float GetRemoteControlVariableFloat(const FString& Key, float DefaultValue);

	UFUNCTION(BlueprintCallable, Category = "Remote Control")
	FString GetRemoteControlVariableString(const FString& Key, const FString& DefaultValue);

	UFUNCTION(BlueprintCallable, Category = "Remote Control")
	bool GetRemoteControlVariableBool(const FString& Key, bool DefaultValue);

	UPROPERTY(BlueprintAssignable, Category = "Remote Control")
	FOnRemoteControlVariableReceived OnRemoteControlVariableReceived;

	void CacheTuningVariables(FHttpResponsePtr Response, const FString& JsonResponse);
	void ReadFromCache();
};

template<typename T>
inline T URemoteControls::GetRemoteControlVariable(FString key, T defaultValue)
{
	if (RemoteControlVariablesInt.Contains(key))
	{
		return RemoteControlVariablesInt[key];
	}
	else if (RemoteControlVariablesFloat.Contains(key))
	{
		return RemoteControlVariablesFloat[key];
	}
	else if (RemoteControlVariablesString.Contains(key))
	{
		return RemoteControlVariablesString[key];
	}
	else if (RemoteControlVariablesBool.Contains(key))
	{
		return RemoteControlVariablesBool[key];
	}
	return defaultValue;
}
