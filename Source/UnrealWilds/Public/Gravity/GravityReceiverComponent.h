// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GravityReceiverComponent.generated.h"

UENUM(BlueprintType)
enum class EOrbitType : uint8
{
	CircularOrbit,      // 圆形轨道
	EllipticalOrbit,    // 椭圆轨道
	NoInitialVelocity   // 不设置初速度
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

protected:

	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gravity")
	TObjectPtr<UPrimitiveComponent> TargetPrimitive;

	UFUNCTION(BlueprintCallable, Category="Gravity")
	FVector CalculateInitialVelocity();
	// 缓存实际使用的 PrimitiveComponent，防止由 RootComponent 变动导致的空指针
	UPROPERTY()
	UPrimitiveComponent* CachedPrimitive = nullptr;

	// 追踪当前是否已经注册，防止重复注册
	bool bIsRegistered = false;

	//初始化设置
	virtual void OnRegister() override;
	
	UPROPERTY(EditAnywhere, Category="Gravity")
	bool bIsInitialClockWise = true;
	UPROPERTY(EditAnywhere,Category="Gravity")
	EOrbitType OrbitType;
	
	UPROPERTY(VisibleAnywhere,blueprintReadWrite, Category="Gravity")
	FVector InitialVelocity;
};
