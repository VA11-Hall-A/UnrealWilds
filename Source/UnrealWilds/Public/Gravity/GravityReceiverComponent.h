// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GravityReceiverComponent.generated.h"

UENUM(BlueprintType)
enum class EOrbitType : uint8
{
	CircularOrbit, // 圆形轨道
	EllipticalOrbit, // 椭圆轨道
	NoInitialVelocity // 不设置初速度
};

UENUM(BlueprintType)
enum class EReceiverType : uint8
{
	Planet,
	Character,
	Item,
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UNREALWILDS_API UGravityReceiverComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UGravityReceiverComponent();

	UFUNCTION(BlueprintCallable, Category = "Gravity")
	void RegisterGravity();

	UFUNCTION(BlueprintCallable, Category = "Gravity")
	void UnregisterGravity();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gravity")
	TObjectPtr<UPrimitiveComponent> TargetPrimitive;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EReceiverType ReceiverType;
	
	UPROPERTY()
	TObjectPtr<class UCharacterMovementComponent> CachedCMC;

	UFUNCTION()
	void ApplyGravity(const FVector& Force);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;


	UFUNCTION(BlueprintCallable, Category="Gravity")
	FVector CalculateInitialVelocity();

	// 追踪当前是否已经注册，防止重复注册
	bool bIsRegistered = false;

	//初始化设置
	virtual void OnRegister() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Gravity")
	bool bIsActivatedOnStartUp = true;

	UPROPERTY(EditAnywhere, Category="Gravity")
	bool bIsInitialClockWise = true;
	UPROPERTY(EditAnywhere, Category="Gravity")
	EOrbitType OrbitType;

	UPROPERTY(VisibleAnywhere, blueprintReadWrite, Category="Gravity")
	FVector InitialVelocity;
	UFUNCTION()
	void ApplyInitialVelocity();
};
