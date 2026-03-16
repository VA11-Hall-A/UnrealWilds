#include "Pawn/PlanetAttachmentComponent.h"

#include "Astro/Planet.h"
#include "Components/SphereComponent.h"
#include "EngineUtils.h"

UPlanetAttachmentComponent::UPlanetAttachmentComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPlanetAttachmentComponent::Initialize(UPrimitiveComponent* InPhysicsPrimitive)
{
	PhysicsPrimitive = InPhysicsPrimitive;
}

void UPlanetAttachmentComponent::AttachToPlanet(APlanet* Planet)
{
	if (!Planet || CurrentPlanet == Planet)
	{
		return;
	}

	CurrentPlanet = Planet;

	OnAttachedToPlanet.Broadcast(Planet);
}

void UPlanetAttachmentComponent::DetachFromPlanet()
{
	if (!CurrentPlanet)
	{
		return;
	}

	FVector OrbitalVelocity = CurrentPlanet->GetOrbitalVelocity();

	// If PhysicsPrimitive is simulating physics, handle velocity inheritance automatically
	FVector PhysicsVelocity = FVector::ZeroVector;
	bool bWasSimulatingPhysics = PhysicsPrimitive && PhysicsPrimitive->IsSimulatingPhysics();
	if (bWasSimulatingPhysics)
	{
		PhysicsVelocity = PhysicsPrimitive->GetPhysicsLinearVelocity();
	}

	CurrentPlanet = nullptr;

	if (bWasSimulatingPhysics)
	{
		PhysicsPrimitive->SetPhysicsLinearVelocity(PhysicsVelocity + OrbitalVelocity);
	}

	OnDetachedFromPlanet.Broadcast(OrbitalVelocity);
}

FVector UPlanetAttachmentComponent::GetOrbitalVelocity() const
{
	if (CurrentPlanet)
	{
		return CurrentPlanet->GetOrbitalVelocity();
	}
	return FVector::ZeroVector;
}

void UPlanetAttachmentComponent::CheckInitialPlanetState()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	APlanet* NearestPlanet = nullptr;
	float MinDistSq = MAX_FLT;

	for (TActorIterator<APlanet> It(GetWorld()); It; ++It)
	{
		float DistSq = FVector::DistSquared(Owner->GetActorLocation(), It->GetActorLocation());
		if (DistSq < MinDistSq)
		{
			MinDistSq = DistSq;
			NearestPlanet = *It;
		}
	}

	if (NearestPlanet && NearestPlanet->AtmosphereSphere)
	{
		float Dist = FMath::Sqrt(MinDistSq);
		float AtmosphereRadius = NearestPlanet->AtmosphereSphere->GetScaledSphereRadius();

		if (Dist < AtmosphereRadius)
		{
			AttachToPlanet(NearestPlanet);
		}
	}
}
