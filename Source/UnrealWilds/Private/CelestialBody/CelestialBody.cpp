// Fill out your copyright notice in the Description page of Project Settings.


#include "CelestialBody/CelestialBody.h"

#include "Gravity/GravityManagerSubsystem.h"
#include "Gravity/GravitySourceComponent.h"
#include "Components/SphereComponent.h"
#include "OWCharacter.h"

// Sets default values
ACelestialBody::ACelestialBody()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
	RootComponent = BodyMesh;

	GravitySource = CreateDefaultSubobject<UGravitySourceComponent>(TEXT("GravitySource"));

	LocalSpaceSphere = CreateDefaultSubobject<USphereComponent>(TEXT("LocalSpaceSphere"));
	LocalSpaceSphere->SetupAttachment(RootComponent);
	LocalSpaceSphere->SetSphereRadius(5000.f);
	LocalSpaceSphere->SetCollisionProfileName(TEXT("Trigger"));
	LocalSpaceSphere->OnComponentBeginOverlap.AddDynamic(this, &ACelestialBody::OnLocalSpaceSphereBeginOverlap);
	LocalSpaceSphere->OnComponentEndOverlap.AddDynamic(this, &ACelestialBody::OnLocalSpaceSphereEndOverlap);
}

// Called when the game starts or when spawned
void ACelestialBody::BeginPlay()
{
	Super::BeginPlay();
	if (UWorld* World = GetWorld())
	{
		if (UGravityManagerSubsystem* GravitySystem = World->GetSubsystem<UGravityManagerSubsystem>())
		{
			GravitySystem->RegisterGravitySource(GravitySource);
		}
	}
}

void ACelestialBody::OnLocalSpaceSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (AOWCharacter* Character = Cast<AOWCharacter>(OtherActor))
	{
		Character->SetCurrentPlanet(this);
	}
}

void ACelestialBody::OnLocalSpaceSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (AOWCharacter* Character = Cast<AOWCharacter>(OtherActor))
	{
		if (Character->GetCurrentPlanet() == this)
		{
			Character->SetCurrentPlanet(nullptr);
		}
	}
}
