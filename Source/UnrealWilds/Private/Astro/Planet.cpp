// Fill out your copyright notice in the Description page of Project Settings.


#include "Astro/Planet.h"
#include "Components/SphereComponent.h"
#include "Components/SceneComponent.h"

APlanet::APlanet()
{
	PrimaryActorTick.bCanEverTick = true;

	HollowInnerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("HollowInnerSphere"));
	HollowInnerSphere->SetupAttachment(RootComponent);
	HollowInnerSphere->SetSphereRadius(5000.0f);

	AtmosphereSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AtmosphereSphere"));
	AtmosphereSphere->SetupAttachment(RootComponent);
	AtmosphereSphere->SetSphereRadius(15000.0f);
}

void APlanet::BeginPlay()
{
	Super::BeginPlay();

	// Set sphere radii from actor-level config (inherited from CelestialBody)
	HollowInnerSphere->SetSphereRadius(static_cast<float>(HollowRadius));
	AtmosphereSphere->SetSphereRadius(static_cast<float>(AtmosphereSphereRadius));

	// if (OrbitCenterActor)
	// {
	// 	FVector Direction = GetActorLocation() - OrbitCenterActor->GetActorLocation();
	// 	CurrentOrbitAngle = FMath::Atan2(Direction.Y, Direction.X) * (180.0f / PI);
	// 	OrbitRadius = Direction.Size2D();
	//
	// 	// Calculate OrbitSpeed based on orbit center's Mass (directly from CelestialBody)
	// 	if (OrbitRadius > 0.0f)
	// 	{
	// 		// v = sqrt(GM / r)  =>  w = sqrt(GM / r^3)
	// 		// GravitySourceData uses Mass * 6.67430E-5 for GM
	// 		double GM = OrbitCenterActor->Mass * 6.67430e-5;
	// 		double OrbitRadiusSq_times_Radius = FMath::Pow(static_cast<double>(OrbitRadius), 3.0);
	// 		double Omega = FMath::Sqrt(GM / OrbitRadiusSq_times_Radius);
	// 		
	// 		// Convert radians/second to degrees/second
	// 		OrbitSpeed = static_cast<float>(FMath::RadiansToDegrees(Omega));
	// 	}
	// }
}

void APlanet::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// if (OrbitCenterActor)
	// {
	// 	CurrentOrbitAngle += OrbitSpeed * DeltaTime;
	// 	CurrentOrbitAngle = FMath::Fmod(CurrentOrbitAngle, 360.0f);
	//
	// 	float RadianAngle = FMath::DegreesToRadians(CurrentOrbitAngle);
	// 	FVector NewLocation = OrbitCenterActor->GetActorLocation();
	// 	NewLocation.X += OrbitRadius * FMath::Cos(RadianAngle);
	// 	NewLocation.Y += OrbitRadius * FMath::Sin(RadianAngle);
	// 	NewLocation.Z = GetActorLocation().Z;
	// 	
	// 	SetActorLocation(NewLocation);
	// }
}
