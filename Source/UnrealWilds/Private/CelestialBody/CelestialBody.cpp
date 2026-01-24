// Fill out your copyright notice in the Description page of Project Settings.


#include "CelestialBody/CelestialBody.h"

#include "Gravity/GravityManagerSubsystem.h"
#include "Gravity/GravitySourceComponent.h"

// Sets default values
ACelestialBody::ACelestialBody()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	GravitySource = CreateDefaultSubobject<UGravitySourceComponent>(TEXT("GravitySource"));
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


