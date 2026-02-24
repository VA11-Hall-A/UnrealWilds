// Fill out your copyright notice in the Description page of Project Settings.


#include "OWCharacter.h"
#include "CelestialBody/CelestialBody.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
AOWCharacter::AOWCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AOWCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AOWCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CurrentPlanet)
	{
		FVector GravityDirection = (CurrentPlanet->GetActorLocation() - GetActorLocation()).GetSafeNormal();
		
		// Align character's feet to face the planet direction (UpVector = -GravityDirection)
		FVector TargetUp = -GravityDirection;
		FQuat TargetRotation = FRotationMatrix::MakeFromZX(GetActorForwardVector(), TargetUp).ToQuat();
		
		// Interpolate rotation for smoothness or set directly
		SetActorRotation(TargetRotation);
	}
}

// Called to bind functionality to input
void AOWCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AOWCharacter::SetCurrentPlanet(ACelestialBody* NewPlanet)
{
	CurrentPlanet = NewPlanet;
	if (CurrentPlanet)
	{
		if (GetCharacterMovement())
		{
			GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		}
	}
	else
	{
		if (GetCharacterMovement())
		{
			GetCharacterMovement()->SetMovementMode(MOVE_Flying);
		}
	}
}
