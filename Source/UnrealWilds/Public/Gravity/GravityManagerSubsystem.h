// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GravityManagerSubsystem.generated.h"
class UGravityReceiverComponent;
class UGravitySourceComponent;

/**
 * 
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGalaxyWakeUp);

UCLASS()
class UNREALWILDS_API UGravityManagerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	virtual void OnPhysScenePreTick(FPhysScene_Chaos* PhysScene, float DeltaTime) ;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }

	UFUNCTION(BlueprintCallable,Category="Gravity")
	void RegisterGravitySource(UGravitySourceComponent* Source);
	UFUNCTION(BlueprintCallable,Category="Gravity")
	void UnregisterGravitySource(UGravitySourceComponent* Source);

	void RegisterGravityReceiver(UGravityReceiverComponent* GravityReceiver);
	void UnregisterGravityReceiver(UGravityReceiverComponent* GravityReceiver);

	UPROPERTY(BlueprintAssignable, Category = "Game Events")
	FOnGalaxyWakeUp OnGalaxyWakeUp;

	// 这是一个辅助函数，供外部调用来触发广播
	UFUNCTION(BlueprintCallable, Category = "Game Events")
	void TriggerGalaxyWakeUp();
private:
	TArray<TWeakObjectPtr<UGravitySourceComponent>> GravitySources;
	TArray<TWeakObjectPtr<UGravityReceiverComponent>> GravityReceivers;
	FDelegateHandle PhysPreTickHandle;

	
};
