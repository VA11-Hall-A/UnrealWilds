// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ThrusterComponent.generated.h"

class UCharacterMovementComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAccelerationChanged, FVector, NewAcceleration);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UNREALWILDS_API UThrusterComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UThrusterComponent();

	void AddForceToMovemntComponent(FVector LocalInput);

	UPROPERTY(EditAnywhere)
	double ThrustForce = 100;
	UPROPERTY(VisibleAnywhere)
	bool bIsCharacterMode = true;
	UPROPERTY()
	double Mass;

	/** 当前本地空间加速度 (X=左右, Y=前后, Z=上下) */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Thruster")
	FVector CurrentLocalAcceleration = FVector::ZeroVector;

	UPROPERTY(BlueprintAssignable, Category = "Thruster")
	FOnAccelerationChanged OnAccelerationChanged;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UCharacterMovementComponent* MovementComponent = nullptr;
	UPrimitiveComponent* PrimitiveComponent = nullptr;
	
};
