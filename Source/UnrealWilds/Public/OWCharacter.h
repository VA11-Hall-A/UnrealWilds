// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "OWCharacter.generated.h"
class ACelestialBody;

UCLASS()
class UNREALWILDS_API AOWCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AOWCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gravity")
	ACelestialBody* CurrentPlanet;

	virtual void SetCurrentPlanet(ACelestialBody* NewPlanet);
	ACelestialBody* GetCurrentPlanet() const { return CurrentPlanet; }

};
