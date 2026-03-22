// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawn/ThrusterComponent.h"


#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Math/UnitConversion.h"


// Sets default values for this component's properties
UThrusterComponent::UThrusterComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	// ...
}

void UThrusterComponent::AddForceToMovemntComponent(FVector LocalInput)
{
	CurrentLocalAcceleration = LocalInput * ThrustForce;
	OnAccelerationChanged.Broadcast(LocalInput);

	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return;
	}

	FVector WorldDirection = OwnerActor->GetActorForwardVector() * LocalInput.Y
		+ OwnerActor->GetActorRightVector() * LocalInput.X
		+ OwnerActor->GetActorUpVector() * LocalInput.Z;

	if (bIsCharacterMode)
	{
		MovementComponent->AddForce(WorldDirection * ThrustForce * Mass);
	}
	else
	{
		PrimitiveComponent->AddForce(WorldDirection * ThrustForce * Mass);
	}
}


// Called when the game starts
void UThrusterComponent::BeginPlay()
{
	Super::BeginPlay();

	// Cache owner once for efficiency and readability
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		// Owner is null, cannot proceed safely
		UE_LOG(LogTemp, Warning, TEXT("%s: Owner actor is null in BeginPlay"), *GetName());
		return;
	}

	// Get root component and cast to UPrimitiveComponent for physics or rendering
	PrimitiveComponent = Cast<UPrimitiveComponent>(OwnerActor->GetRootComponent());
	if (!PrimitiveComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: Owner has no valid UPrimitiveComponent root"), *GetName());
	}

	// Attempt to cast owner to ACharacter to get movement component and mass
	if (ACharacter* Character = Cast<ACharacter>(OwnerActor))
	{
		MovementComponent = Character->GetCharacterMovement();
		Mass = MovementComponent->Mass;
	}
	else if (PrimitiveComponent)
	{
		Mass = PrimitiveComponent->GetMass();
	}
}
