// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CelestialBody.generated.h"

class UGravitySourceComponent;

UCLASS()
class UNREALWILDS_API ACelestialBody : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACelestialBody();

	// --- Gravity Configuration (single source of truth) ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity")
	bool bUseInverseSquare = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity",
		meta = (ForceUnits="Kg", ClampMin = "1"))
	double Mass = 5.9722E24;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity",
		meta = (ForceUnits="cm", ClampMin = "1"))
	double PlanetRadius = 25000.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity",
		meta = (ForceUnits="cm", ClampMin = "0.1"))
	double HollowRadius = 500.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity")
	bool ApplyGravity = true;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly, Category="Gravity")
	UGravitySourceComponent* GravitySource;

};
