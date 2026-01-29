// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CelestialBody/CelestialBody.h"
#include "Planet.generated.h"

class UGravityReceiverComponent;
/**
 * 
 */
UCLASS()
class UNREALWILDS_API APlanet : public ACelestialBody
{
	GENERATED_BODY()

public:
	// 构造函数
	APlanet();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UGravityReceiverComponent* GravityReceiver;
	
protected:
	// 游戏开始时调用
	virtual void BeginPlay() override;
};
