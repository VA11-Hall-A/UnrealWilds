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
		
		if (UCharacterMovementComponent* CMC = GetCharacterMovement())
		{
			CMC->SetGravityDirection(GravityDirection);
		}
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
			GetCharacterMovement()->SetMovementMode(MOVE_Falling);
		}
	}
	else
	{
		if (GetCharacterMovement())
		{
			GetCharacterMovement()->SetMovementMode(MOVE_None);
		}
	}
}
