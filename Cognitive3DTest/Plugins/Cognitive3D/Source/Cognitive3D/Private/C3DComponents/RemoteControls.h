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

class FRemoteControlsRecorder;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class COGNITIVE3D_API URemoteControls : public UActorComponent
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
	UFUNCTION()
	void QueryRemoteControlVariable(FString ParticipantId);

	void OnHttpResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	void ParseJsonResponse(const FString& JsonResponse);

	template <typename T>
	T GetRemoteControlVariable(FString key, T defaultValue);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRemoteControlVariableReceived);

	int32 GetRemoteControlVariableInt(const FString& Key, int32 DefaultValue);

	float GetRemoteControlVariableFloat(const FString& Key, float DefaultValue);

	FString GetRemoteControlVariableString(const FString& Key, const FString& DefaultValue);

	bool GetRemoteControlVariableBool(const FString& Key, bool DefaultValue);

	UPROPERTY(BlueprintAssignable, Category = "Remote Control")
	FOnRemoteControlVariableReceived OnRemoteControlVariableReceived;

	void CacheRemoteControlVariables(const FString& JsonResponse);
	void ReadFromCache();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Remote Control")
	bool bFetchVariablesAutomatically = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Remote Control")
	bool bUseParticipantID = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Remote Control")
	float WaitForParticipantIdTimeout = 5.0f;

	// Timer handle for managing the delay
	FTimerHandle TimerHandle;

	void CallTimerEndFunction();
};

template<typename T>
inline T URemoteControls::GetRemoteControlVariable(FString key, T defaultValue)
{
	if (FRemoteControlsRecorder::GetInstance()->RemoteControlVariablesInt.Contains(key))
	{
		return FRemoteControlsRecorder::GetInstance()->RemoteControlVariablesInt[key];
	}
	else if (FRemoteControlsRecorder::GetInstance()->RemoteControlVariablesFloat.Contains(key))
	{
		return FRemoteControlsRecorder::GetInstance()->RemoteControlVariablesFloat[key];
	}
	else if (FRemoteControlsRecorder::GetInstance()->RemoteControlVariablesString.Contains(key))
	{
		return FRemoteControlsRecorder::GetInstance()->RemoteControlVariablesString[key];
	}
	else if (FRemoteControlsRecorder::GetInstance()->RemoteControlVariablesBool.Contains(key))
	{
		return FRemoteControlsRecorder::GetInstance()->RemoteControlVariablesBool[key];
	}
	return defaultValue;
}
