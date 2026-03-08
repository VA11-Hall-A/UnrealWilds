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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Astro|Planet")
	USphereComponent* HollowInnerSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Astro|Planet")
	USphereComponent* AtmosphereSphere;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	float AtmosphereSphereRadius = 35000.0f;

	UFUNCTION(BlueprintCallable, Category = "Astro|Orbit")
	FVector GetOrbitalVelocity() const;

	UFUNCTION()
	void OnAtmosphereBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnAtmosphereEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void OnHollowBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnHollowEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

protected:
	virtual void BeginPlay() override;

	// The class of the actor this planet revolves around
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Astro|Orbit")
	TSubclassOf<class ACelestialBody> OrbitCenterClass;

	// The actor this planet revolves around (found at runtime)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category="Astro|Orbit")
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
