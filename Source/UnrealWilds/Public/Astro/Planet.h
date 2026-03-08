// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Astro/CelestialBody.h"
#include "Planet.generated.h"

class USphereComponent;

/**
 * 
 */
UCLASS()
class UNREALWILDS_API APlanet : public ACelestialBody
{
	GENERATED_BODY()
	
public:
	APlanet();

	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Astro|Planet")
	USphereComponent* HollowInnerSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Astro|Planet")
	USphereComponent* AtmosphereSphere;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	float AtmosphereSphereRadius = 35000.0f;
	
	// The actor this planet revolves around (e.g., a Sun)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Astro|Orbit")
	ACelestialBody* OrbitCenterActor;

	// The speed of revolution (degrees per second)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Astro|Orbit")
	float OrbitSpeed = 10.0f;

	// The radius of the orbit
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Astro|Orbit")
	float OrbitRadius = 10000.0f;

private:
	float CurrentOrbitAngle = 0.0f;
};
