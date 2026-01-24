// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GravitySourceComponent.generated.h"

// 定义天体类型枚举
UENUM(BlueprintType)
enum class EGravitySourceType : uint8
{
	Sun UMETA(DisplayName = "Sun (Inverse Square)"),   // 太阳：距离平方反比
	Planet UMETA(DisplayName = "Planet (Inverse Linear)") // 行星：距离反比
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UNREALWILDS_API UGravitySourceComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UGravitySourceComponent();

	// 天体类型
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gravity")
	EGravitySourceType SourceType = EGravitySourceType::Planet;
	
	// 星球质量（kg）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gravity")
	float Mass = 1e24f;
 
	// 引力作用半径（米）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gravity")
	float GravityRadius = 30000.f;
 
	// 行星半径（米）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gravity")
	float PlanetRadius = 200.f;
 
	// 计算目标点引力矢量
	FVector GetGravityAtPoint(const FVector& Point) const;
};
