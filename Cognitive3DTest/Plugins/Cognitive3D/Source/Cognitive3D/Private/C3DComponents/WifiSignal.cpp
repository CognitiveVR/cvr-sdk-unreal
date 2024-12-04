// Fill out your copyright notice in the Description page of Project Settings.


#include "C3DComponents/WifiSignal.h"
#include "Cognitive3D/Private/C3DUtil/Util.h"
#include "Cognitive3D/Public/Cognitive3D.h"
#if PLATFORM_ANDROID
#include "Android/AndroidJNI.h"
#include "Android/AndroidApplication.h"
#endif

// Sets default values for this component's properties
UWifiSignal::UWifiSignal()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UWifiSignal::BeginPlay()
{
	Super::BeginPlay();

	auto cognitive = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
	if (cognitive.IsValid())
	{
		cognitive->OnSessionBegin.AddDynamic(this, &UWifiSignal::OnSessionBegin);
		cognitive->OnPreSessionEnd.AddDynamic(this, &UWifiSignal::OnSessionEnd);
		if (cognitive->HasStartedSession())
		{
			OnSessionBegin();
		}
	}

}


// Called every frame
void UWifiSignal::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UWifiSignal::OnSessionBegin()
{
	auto world = ACognitive3DActor::GetCognitiveSessionWorld();
	if (world == nullptr) { return; }
	world->GetTimerManager().SetTimer(IntervalHandle, FTimerDelegate::CreateUObject(this, &UWifiSignal::EndInterval), IntervalDuration, true);
}

void UWifiSignal::OnSessionEnd()
{

}

void UWifiSignal::EndInterval()
{
#if PLATFORM_ANDROID
	JNIEnv* Env = FAndroidApplication::GetJavaEnv();
	if (Env == nullptr)
	{
		return;
	}
	jobject GameActivity = FAndroidApplication::GetGameActivityThis();
	jclass PluginClass = FAndroidApplication::FindJavaClass("com/c3d/androidjavaplugin/Plugin");
	jmethodID MethodID = Env->GetMethodID(PluginClass, "getWifiSignalStrength", "()I");
	jmethodID GetInstanceMethod = Env->GetStaticMethodID(PluginClass, "getInstance", "(Landroid/app/Activity;)Lcom/c3d/androidjavaplugin/Plugin;");
	jobject PluginInstance = Env->CallStaticObjectMethod(PluginClass, GetInstanceMethod, GameActivity);
	currentSignalStrength = Env->CallIntMethod(PluginInstance, MethodID);
	UE_LOG(LogTemp, Warning, TEXT("Signal Strength: %d"), currentSignalStrength);
	if (previousSignalStrength != currentSignalStrength)
	{
		previousSignalStrength = currentSignalStrength;
		auto cognitive = FAnalyticsCognitive3D::Get().GetCognitive3DProvider().Pin();
		if (cognitive.IsValid())
		{
			float SignalStrFloat = float(currentSignalStrength);
			cognitive->sensors->RecordSensor("WifiRSSI", SignalStrFloat);
		}
	}

#endif
}

