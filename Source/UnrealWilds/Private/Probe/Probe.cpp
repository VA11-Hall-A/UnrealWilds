// Fill out your copyright notice in the Description page of Project Settings.

#include "Probe/Probe.h"
#include "Components/StaticMeshComponent.h"

AProbe::AProbe()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	ProbeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProbeMesh"));
	SetRootComponent(ProbeMesh);

	ProbeMesh->SetSimulatePhysics(true);
	ProbeMesh->SetEnableGravity(false);
	ProbeMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	ProbeMesh->SetNotifyRigidBodyCollision(true);
}

void AProbe::BeginPlay()
{
	Super::BeginPlay();

	ProbeMesh->OnComponentHit.AddDynamic(this, &AProbe::OnProbeHit);
}

void AProbe::OnProbeHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	if (bIsAttached)
	{
		return;
	}

	bIsAttached = true;

	ProbeMesh->SetSimulatePhysics(false);

	// Align probe's -Z axis with impact normal (bottom faces surface)
	const FVector UpDirection = Hit.ImpactNormal;
	FVector ForwardDirection = GetActorForwardVector();
	// Project forward onto the surface plane
	FVector ProjectedForward = ForwardDirection - (ForwardDirection | UpDirection) * UpDirection;
	if (ProjectedForward.IsNearlyZero())
	{
		// If forward is nearly parallel to normal, use right vector as fallback
		ProjectedForward = GetActorRightVector();
	}
	ProjectedForward.Normalize();

	const FRotator TargetRotation = FRotationMatrix::MakeFromZX(UpDirection, ProjectedForward).Rotator();
	SetActorRotation(TargetRotation);

	// Attach to the hit component so the probe follows it
	AttachToComponent(OtherComp, FAttachmentTransformRules::KeepWorldTransform);
}

void AProbe::Launch(const FVector& Direction, float Speed, const FVector& InheritedVelocity)
{
	bIsAttached = false;

	// Detach in case we are reusing a probe
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	ProbeMesh->SetSimulatePhysics(true);
	ProbeMesh->SetEnableGravity(false);

	ProbeMesh->SetPhysicsLinearVelocity(Direction * Speed + InheritedVelocity);
}
