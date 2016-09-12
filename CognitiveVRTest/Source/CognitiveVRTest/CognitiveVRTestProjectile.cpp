// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "CognitiveVRTest.h"
#include "CognitiveVRTestProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"

//#include "Private/CognitiveVRProvider.h"
//#include "Public/transaction.h"
//#include "Public/CognitiveVR.h"

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

		/*TSharedPtr<IAnalyticsProvider> Analytics = FAnalytics::Get().GetDefaultConfiguredProvider();
		
		Analytics.Get()->StartSession();

		Analytics.Get()->RecordEvent("simple event");

		TArray<FAnalyticsEventAttribute> attributes;
		attributes.Add(FAnalyticsEventAttribute("MyBoolean", false));
		attributes.Add(FAnalyticsEventAttribute("NumbersNotSupported", FString("5")));
		attributes.Add(FAnalyticsEventAttribute("MyString", FString("Words")));

		Analytics.Get()->RecordEvent("event with attributes", attributes);

		*/

		//Analytics.Get()->RecordEvent()

		

		//TODO can't link transaction.beginend
		TSharedPtr<FAnalyticsProviderCognitiveVR> cog = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider();

		//std::string temp = "STRINGRIGN";

		//cog.Get()->SetAge(1);
		auto trans = cog.Get()->transaction;// ->FBeginEnd("name");
		FString temp = "name";
		std::string stdtemp = "userid";

		//cog.Get()->core_utils;// ->NewUser(stdtemp);
		//trans->FBeginEnd(temp);
		trans->BeginEnd(stdtemp);

		//cog.Get()->transaction->FBeginEnd(FString("SOMEVALUE"));

		//auto ca = FAnalyticsCognitiveVR::Get().GetCognitiveVRProvider();
		//ca.Get()->transaction->BeginEnd("c impact", NULL);

		//TSharedPtr<FJsonObject> properties = MakeShareable(new FJsonObject);
		//properties->SetStringField("projectile", "hit");

		//FAnalytics::Get().GetDefaultConfiguredProvider()->RecordEvent("hit");

		//FCognitiveVRAnalytics::Get().CognitiveVR()->transaction->BeginEnd("impact", properties);

		boxesHit++;
		//FCognitiveVRAnalytics::Get().CognitiveVR()->core_utils->UpdateCollection("boxesHit",boxesHit,1,false);

		Destroy();
	}
}