// Fill out your copyright notice in the Description page of Project Settings.

#include "Probe/ProbeLauncherComponent.h"
#include "Probe/Probe.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"

TWeakObjectPtr<AProbe> UProbeLauncherComponent::ActiveProbe;

UProbeLauncherComponent::UProbeLauncherComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UProbeLauncherComponent::LaunchProbe()
{
	if (ActiveProbe.IsValid())
	{
		ActiveProbe->Destroy();
		ActiveProbe = nullptr;
	}

	AActor* Owner = GetOwner();
	if (!Owner || !ProbeClass)
	{
		return;
	}

	UCameraComponent* Camera = Owner->FindComponentByClass<UCameraComponent>();
	if (!Camera)
	{
		return;
	}

	const FVector Direction = Camera->GetForwardVector();
	const FVector SpawnLocation = Camera->GetComponentLocation() + Direction * 150.0f;
	const FRotator SpawnRotation = Camera->GetComponentRotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Owner;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AProbe* NewProbe = GetWorld()->SpawnActor<AProbe>(ProbeClass, SpawnLocation, SpawnRotation, SpawnParams);
	if (NewProbe)
	{
		ActiveProbe = NewProbe;
		const FVector InheritedVelocity = Owner->GetVelocity();
		NewProbe->Launch(Direction, LaunchSpeed, InheritedVelocity);
	}
	UE_LOG(LogTemp, Warning, TEXT("Probe launched!"));
}

void UProbeLauncherComponent::RecallProbe()
{
	if (ActiveProbe.IsValid())
	{
		ActiveProbe->Destroy();
		ActiveProbe = nullptr;
	}
}

AProbe* UProbeLauncherComponent::GetActiveProbe() const
{
	return ActiveProbe.Get();
}
