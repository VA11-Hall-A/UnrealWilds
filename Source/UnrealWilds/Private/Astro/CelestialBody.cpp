// Fill out your copyright notice in the Description page of Project Settings.


#include "Astro/CelestialBody.h"

#include "Gravity/GravitySourceComponent.h"

// Sets default values
ACelestialBody::ACelestialBody()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	GravitySource=CreateDefaultSubobject<UGravitySourceComponent>(TEXT("GravitySourceComponent"));
	
}

// Called when the game starts or when spawned
void ACelestialBody::BeginPlay()
{
	Super::BeginPlay();

	// Sync actor-level config to the gravity source component
	if (GravitySource)
	{
		GravitySource->bUseInverseSquare = bUseInverseSquare;
		GravitySource->Mass = Mass;
		GravitySource->PlanetRadius = PlanetRadius;
		GravitySource->HollowRadius = HollowRadius;
		GravitySource->ApplyGravity = ApplyGravity;
	}
}


