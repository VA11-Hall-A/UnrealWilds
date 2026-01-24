// Fill out your copyright notice in the Description page of Project Settings.


#include "Gravity/GravitySourceComponent.h"
#include "OWSettings.h"

// Sets default values for this component's properties
UGravitySourceComponent::UGravitySourceComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


FVector UGravitySourceComponent::GetGravityAtPoint(const FVector& Point) const
{
	FVector Center = GetOwner()->GetActorLocation();
	FVector Dir = Center - Point;
	double Dist = Dir.Size()/100;

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			2, // 使用 -1 确保每帧都会产生新消息，或者使用特定 Key 覆盖旧消息
			1, // 显示持续时间（秒）
			FColor::Red, // 字体颜色
			FString::Printf(TEXT("r:%f "), Dist)
		);
	}
	// 超出最大引力范围，无引力
	if (Dist > GravityRadius)
		return FVector::ZeroVector;
	
	const float G = GetDefault<UOWSettings>()->GravityConstant;
 
	double GravityStrength = 0.0f;

	if (SourceType == EGravitySourceType::Sun)
	{
		// --- 太阳模式：距离平方反比 (1/r^2) ---
		if (Dist >= PlanetRadius)
		{
			// 公式: G * M / r^2
			GravityStrength = (G * Mass) / (Dist * Dist);
		}
		else
		{
			// 内部线性引力，匹配表面的 GM/R^2
			// 公式: (G * M * r) / R^3
			GravityStrength = (G * Mass * Dist) / (PlanetRadius * PlanetRadius * PlanetRadius);
		}
	}
	else
	{
		// --- 行星模式：距离反比 (1/r) ---
		if (Dist >= PlanetRadius)
		{
			// 公式: G * M / r
			GravityStrength = (G * Mass) / Dist;
		}
		else
		{
			// 内部线性引力，需要匹配表面的 GM/R
			// 为了保证在 Dist == PlanetRadius 时引力连续：
			// 表面引力 SurfaceGravity = (G * Mass) / PlanetRadius
			// 内部引力 = SurfaceGravity * (Dist / PlanetRadius)
			// 推导: (G * M / R) * (r / R) = (G * M * r) / R^2
			GravityStrength = (G * Mass * Dist) / (PlanetRadius * PlanetRadius);
		}
	}
 
	return Dir.GetSafeNormal() * GravityStrength;
}