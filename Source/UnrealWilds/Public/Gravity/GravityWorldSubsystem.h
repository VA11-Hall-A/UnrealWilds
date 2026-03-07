// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GravitySourceComponent.h"
#include "GravityWorldSubsystem.generated.h"

class AUWCharacter;
class FGravityAsyncCallback;
class UCharacterMovementComponent;
struct FGravitySourceData;

/**
 * 
 */
UCLASS()
class UNREALWILDS_API UGravityWorldSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()
public: // UTickableWorldSubsystem overrides
	virtual void Deinitialize() override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual TStatId GetStatId() const override;
	
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void RegisterPlayerCharacter(UCharacterMovementComponent* InCMComponent);
	
	void UpdateCMCGravities();
	
	// Keep track of any attractors (optional)
	void AddSource(UGravitySourceComponent* GravityAttractorComponent);
	void RemoveSource(UGravitySourceComponent* GravityAttractorComponent);

	virtual void RegisterAsyncCallback(); 
	virtual bool IsAsyncCallbackRegistered() const;
	void AddGravitySourceData(const FGravitySourceData& InputData) const;
	FGravityAsyncCallback* AsyncCallback = nullptr;
 
protected:
 
	// List of existing gravity attractors in the world
	TArray<TWeakObjectPtr<UGravitySourceComponent>> Sources;

	UPROPERTY(VisibleInstanceOnly)
	TObjectPtr<UCharacterMovementComponent> CMComponent = nullptr;
};
