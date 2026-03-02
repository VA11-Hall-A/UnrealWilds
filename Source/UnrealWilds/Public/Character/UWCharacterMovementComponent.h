// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UWCharacterMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class UNREALWILDS_API UUWCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
	virtual void UpdateBasedMovement(float DeltaSeconds) override;
};
