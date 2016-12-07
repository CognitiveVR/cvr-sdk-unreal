// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "CognitiveVRTest.h"
#include "CognitiveVRTestProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"

ACognitiveVRTestProjectile::ACognitiveVRTestProjectile() 
{
	// Use a sphere as a simple collision representation
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->BodyInstance.SetCollisionProfileName("Projectile");
	CollisionComp->OnComponentHit.AddDynamic(this, &ACognitiveVRTestProjectile::OnHit);		// set up a notification for when this component hits something blocking

	// Players can't walk on it
	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;

	// Set as root component
	RootComponent = CollisionComp;

	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 30000.f;
	ProjectileMovement->MaxSpeed = 30000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;

	// Die after 3 seconds by default
	InitialLifeSpan = 3.0f;
}

int ACognitiveVRTestProjectile::boxesHit = 0;

void ACognitiveVRTestProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Only add impulse and destroy projectile if we hit a physics
	if ((OtherActor != NULL) && (OtherActor != this) && (OtherComp != NULL) && OtherComp->IsSimulatingPhysics())
	{
		OtherComp->AddImpulseAtLocation(GetVelocity() * 100.0f, GetActorLocation());

		/*DrawDebugSphere(
			GetWorld(),
			this->GetActorLocation(),
			24,  					//size
			32,
			FColor(255, 0, 255),
			true
		);*/

		//generic analytics code

		TSharedPtr<FAnalyticsProviderCognitiveVR> cognitive = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider();
		TSharedPtr<FJsonObject> properties = MakeShareable(new FJsonObject);
		//properties->SetStringField("MyStrin", "stringvalue");
		//properties->SetNumberField("MyNumber", 5);
		//properties->SetBoolField("MyBoolean", true);
		cognitive.Get()->transaction->BeginEndPosition("boxhit", this->GetActorLocation(), properties);
		
		/*bool myBool = false;
		FAnalyticsProviderCognitiveVR *cognitive = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider().Get();
		if (cognitive != NULL)
		{
			CognitiveVRResponse resp = cognitive->tuning->GetValue("MyTuningBool", "false");

			if (resp.IsSuccessful() && resp.GetContent().TryGetBoolField("MyTuningBool",myBool))
			{
				UE_LOG(LogTemp, Log, TEXT("MyTuningBool is: %s"), myBool ? TEXT("true") : TEXT("false"));
			}
		}*/


		/*
		auto resp2 = cognitive.Get()->tuning->GetValue("MyTuningInt", "0");
		if (resp2.IsSuccessful())
		{
			//FString val = resp2.GetContent().GetStringField("MyTuningInt");
			bool outBool;
			if (resp2.GetContent().TryGetBoolField("MyTuningInt", outBool))
			{
				//whatever out bool
			}

			//if (resp2.GetContent().)

			FString outString;
			int32 outInt;
			if (resp2.GetContent().TryGetStringField("MyTuningInt", outString))
			{
				//it's a string

				outInt = FCString::Atoi(*outString);
				UE_LOG(LogTemp, Warning, TEXT("MyTuningInt %d"), outInt);
			}

			//double metres = resp2.GetContent().GetNumberField("MyTuningInt");
		}*/

		/*
		Analytics.Get()->StartSession();
		Analytics.Get()->RecordEvent("simple event");

		TArray<FAnalyticsEventAttribute> attributes;
		attributes.Add(FAnalyticsEventAttribute("MyBoolean", false));
		attributes.Add(FAnalyticsEventAttribute("NumbersNotSupported", FString("5")));
		attributes.Add(FAnalyticsEventAttribute("MyString", FString("Words")));

		Analytics.Get()->RecordEvent("event with attributes", attributes);
		*/
		/*
		TSharedPtr<IAnalyticsProvider> analytics = FAnalytics::Get().GetDefaultConfiguredProvider();
		TArray<FAnalyticsEventAttribute> attributes;
		attributes.Add(FAnalyticsEventAttribute("MyBoolean", false));
		attributes.Add(FAnalyticsEventAttribute("NumbersNotSupported", FString("5")));
		attributes.Add(FAnalyticsEventAttribute("MyString", FString("Words")));
		analytics.Get()->RecordEvent("EventName", attributes);

		analytics.Get()->SetUserID("UniqueUserID");





		//cogntive vr analytics code
		
		TSharedPtr<FAnalyticsProviderCognitiveVR> cognitive = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider();
		TSharedPtr<FJsonObject> properties = MakeShareable(new FJsonObject);
		properties->SetStringField("MyStrin", "stringvalue");
		properties->SetNumberField("MyNumber", 5);
		properties->SetBoolField("MyBoolean", true);
		cognitive.Get()->transaction->BeginEnd("EventName", properties);


		cognitive.Get()->StartSession(attributes);
		cognitive.Get()->SetUserID("UniqueUserID");
		cognitive.Get()->SetGender("Male");
		cognitive.Get()->SetAge(20);
		
		int newValue = 5;
		int delta = 1;
		bool isCurrency = false;
		cognitive.Get()->core_utils->UpdateCollection("collectionname", newValue, delta, isCurrency);

		//cognitive.Get()->core_utils->UpdateCollection()



		properties->SetStringField("hair color", "black");
		properties->SetNumberField("height", 180);
		cognitive.Get()->core_utils->UpdateUserState("UniqueUserID", properties);
		cognitive.Get()->transaction->Begin("EventName",NULL,"UniqueTransactionID");
		cognitive.Get()->transaction->End("EventName", NULL, "UniqueTransactionID");*/
/*

		auto trans = cog.Get()->transaction;
		trans->Begin("begin");
		trans->Update("update");
		trans->End("end");

		TSharedPtr<FJsonObject> properties = MakeShareable(new FJsonObject);
		properties->SetStringField("mystring", "stringvalue");
		properties->SetNumberField("mynumber", 5);

		trans->BeginEnd("beginend",properties);


		auto tune = cog.Get()->tuning;
		tune->GetAllValues("entityid", cognitivevrapi::EntityType::kEntityTypeDevice);
		tune->GetValue("name", "defaultvalue", "entityid", cognitivevrapi::EntityType::kEntityTypeUser);
		tune->RecordValueAsync(NULL, "name", "defaultvalue");

		auto core = cog.Get()->core_utils;
		core->UpdateCollection("collectionname", 5, 1, false);
		core->NewDevice("deviceid");
		core->NewUser("userid");
		*/

		boxesHit++;
		//FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider()->core_utils->UpdateCollection("boxesHit",boxesHit,1,false);

		Destroy();
	}
}