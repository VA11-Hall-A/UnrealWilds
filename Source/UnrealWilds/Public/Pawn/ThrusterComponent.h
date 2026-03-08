// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ThrusterComponent.generated.h"

class UCharacterMovementComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UNREALWILDS_API UThrusterComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UThrusterComponent();

	void AddForceToMovemntComponent(FVector InputDirection);

	UPROPERTY(EditAnywhere)
	double ThrustForce = 0;
	UPROPERTY(VisibleAnywhere)
	bool bIsCharacterMode = true;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UCharacterMovementComponent* MovementComponent = nullptr;
	UPrimitiveComponent* PrimitiveComponent = nullptr;
	
};
