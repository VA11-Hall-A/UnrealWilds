// Fill out your copyright notice in the Description page of Project Settings.


#include "Astro/Planet.h"

#include "Astro/Star.h"
#include "Components/SphereComponent.h"
#include "Components/SceneComponent.h"
#include "Character/UWCharacter.h"
#include "Pawn/PlanetAttachmentComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Gravity/GravitySourceComponent.h"

APlanet::APlanet()
{
	PrimaryActorTick.bCanEverTick = true;
	
	UE_LOG(LogTemp,Warning,TEXT("%s"),*RootComponent.GetName())
	HollowInnerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("HollowInnerSphere"));
	HollowInnerSphere->SetupAttachment(GetRootComponent());
	HollowInnerSphere->SetSphereRadius(5000.0f);
	HollowInnerSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	HollowInnerSphere->SetCollisionObjectType(ECC_WorldDynamic);
	HollowInnerSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	HollowInnerSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	AtmosphereSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AtmosphereSphere"));
	AtmosphereSphere->SetupAttachment(GetRootComponent());
	AtmosphereSphere->SetSphereRadius(15000.0f);
	AtmosphereSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	AtmosphereSphere->SetCollisionObjectType(ECC_WorldDynamic);
	AtmosphereSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	AtmosphereSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void APlanet::BeginPlay()
{
	Super::BeginPlay();

	// Set sphere radii from actor-level config (inherited from CelestialBody)
	HollowInnerSphere->SetSphereRadius(static_cast<float>(HollowRadius));
	AtmosphereSphere->SetSphereRadius(static_cast<float>(AtmosphereSphereRadius));

	HollowInnerSphere->OnComponentBeginOverlap.AddDynamic(this, &APlanet::OnHollowBeginOverlap);
	HollowInnerSphere->OnComponentEndOverlap.AddDynamic(this, &APlanet::OnHollowEndOverlap);
	AtmosphereSphere->OnComponentBeginOverlap.AddDynamic(this, &APlanet::OnAtmosphereBeginOverlap);
	AtmosphereSphere->OnComponentEndOverlap.AddDynamic(this, &APlanet::OnAtmosphereEndOverlap);

	if (OrbitCenterClass)
	{
		OrbitCenterActor = Cast<ACelestialBody>(UGameplayStatics::GetActorOfClass(this, OrbitCenterClass));
	}

	if (OrbitCenterActor)
	{
		FVector Direction = GetActorLocation() - OrbitCenterActor->GetActorLocation();
		CurrentOrbitAngle = FMath::Atan2(Direction.Y, Direction.X) * (180.0f / PI);
		OrbitRadius = Direction.Size2D();
	
		// Calculate OrbitSpeed based on orbit center's Mass (directly from CelestialBody)
		if (OrbitRadius > 0.0f)
		{
			// v = sqrt(GM / r)  =>  w = sqrt(GM / r^3)
			// GravitySourceData uses Mass * 6.67430E-5 for GM
			double GM = OrbitCenterActor->Mass * 6.67430e-5;
			double OrbitRadiusSq_times_Radius = FMath::Pow(static_cast<double>(OrbitRadius), 3.0);
			double Omega = FMath::Sqrt(GM / OrbitRadiusSq_times_Radius);
			
			// Convert radians/second to degrees/second
			OrbitSpeed = static_cast<float>(FMath::RadiansToDegrees(Omega));
		}
	}

	if (SunClass)
	{
		SunActor=Cast<AStar>(UGameplayStatics::GetActorOfClass(this, SunClass));
	}
}

void APlanet::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (OrbitCenterActor)
	{
		CurrentOrbitAngle += OrbitSpeed * DeltaTime;
		CurrentOrbitAngle = FMath::Fmod(CurrentOrbitAngle, 360.0f);
	
		float RadianAngle = FMath::DegreesToRadians(CurrentOrbitAngle);
		FVector NewLocation = OrbitCenterActor->GetActorLocation();
		NewLocation.X += OrbitRadius * FMath::Cos(RadianAngle);
		NewLocation.Y += OrbitRadius * FMath::Sin(RadianAngle);
		NewLocation.Z = GetActorLocation().Z;
		
		SetActorLocation(NewLocation);
	}
}

FVector APlanet::GetOrbitalVelocity() const
{
	if (!OrbitCenterActor)
	{
		return FVector::ZeroVector;
	}

	float RadianAngle = FMath::DegreesToRadians(CurrentOrbitAngle);
	float W = FMath::DegreesToRadians(OrbitSpeed);

	// Velocity = derivative of Position
	// X = Center.X + R * cos(wt) -> vX = -R * w * sin(wt)
	// Y = Center.Y + R * sin(wt) -> vY =  R * w * cos(wt)
	return FVector(-OrbitRadius * W * FMath::Sin(RadianAngle), 
					OrbitRadius * W * FMath::Cos(RadianAngle), 
					0.0f);
}

FVector APlanet::GetSunLocation() const
{
	return SunActor->GetActorLocation();
}

void APlanet::OnAtmosphereBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (UPlanetAttachmentComponent* Attachment = OtherActor->FindComponentByClass<UPlanetAttachmentComponent>())
	{
		Attachment->AttachToPlanet(this);
	}
}

void APlanet::OnAtmosphereEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (UPlanetAttachmentComponent* Attachment = OtherActor->FindComponentByClass<UPlanetAttachmentComponent>())
	{
		Attachment->DetachFromPlanet();
	}
}

void APlanet::OnHollowBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (AUWCharacter* Character = Cast<AUWCharacter>(OtherActor))
	{
		if (Character->IsLocallyControlled() && !Character->IsOnGravityFloor())
		{
			Character->EnterZeroG();
		}
	}
}

void APlanet::OnHollowEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (AUWCharacter* Character = Cast<AUWCharacter>(OtherActor))
	{
		if (Character->IsLocallyControlled() && !Character->IsOnGravityFloor())
		{
			Character->EnterSurfaceGravity();
		}
	}
}
