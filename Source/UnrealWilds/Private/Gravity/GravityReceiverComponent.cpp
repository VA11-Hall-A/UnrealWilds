// Fill out your copyright notice in the Description page of Project Settings.


#include "Gravity/GravityReceiverComponent.h"

#include "OWSettings.h"
#include "Gravity/GravityManagerSubsystem.h"

// Sets default values for this component's properties
UGravityReceiverComponent::UGravityReceiverComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	OrbitType = EOrbitType::CircularOrbit;
}

void UGravityReceiverComponent::RegisterGravity()
{
	if (bIsRegistered)
	{
		return;
	}
	// 2. 获取子系统
	UWorld* World = GetWorld();
	if (!World) return;

	auto* GravitySubsystem = World->GetSubsystem<UGravityManagerSubsystem>();
	if (!GravitySubsystem) return;

	GravitySubsystem->RegisterGravityReceiver(this);
	bIsRegistered = true; // 标记状态
}

void UGravityReceiverComponent::UnregisterGravity()
{
	// 1. 如果根本没注册过，直接返回
	if (!bIsRegistered)
	{
		return;
	}

	UWorld* World = GetWorld();
	// EndPlay 时 World 可能已经开始销毁流程，需要注意判空
	if (!World) return;

	// 2. 获取子系统并执行注销
	if (auto* GravitySubsystem = World->GetSubsystem<UGravityManagerSubsystem>())
	{
		GravitySubsystem->UnregisterGravityReceiver(this);
	}

	// 3. 重置状态
	bIsRegistered = false;

}

void UGravityReceiverComponent::OnRegister()
{
	Super::OnRegister();
	if (AActor* Owner = GetOwner())
	{
		UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(Owner->GetRootComponent());
		checkf(RootPrim != nullptr, TEXT("root component must not be null here!"));
		TargetPrimitive = RootPrim;
		switch (OrbitType)
		{
		case EOrbitType::CircularOrbit:
			InitialVelocity = CalculateInitialVelocity();
			break;
		case EOrbitType::EllipticalOrbit:
			//to be done
			break;
		case EOrbitType::NoInitialVelocity:
			//to be done
			break;
		}
	}
}

void UGravityReceiverComponent::ApplyInitialVelocity()
{
	if (TargetPrimitive)
	{
		// 确保物理模拟已开启，否则施加速度无效
		if (!TargetPrimitive->IsSimulatingPhysics())
		{
			TargetPrimitive->SetSimulatePhysics(true);
		}

		TargetPrimitive->SetPhysicsLinearVelocity(InitialVelocity);

		// Debug Log (可选)
		UE_LOG(LogTemp, Log, TEXT("Actor %s Applied Initial Velocity: %s"), *GetOwner()->GetName(), *InitialVelocity.ToString());
	}
}


void UGravityReceiverComponent::ApplyGravity(const FVector& Force)
{
	TargetPrimitive->AddForce(Force);
}

// Called when the game starts
void UGravityReceiverComponent::BeginPlay()
{
	Super::BeginPlay();
	if (UWorld* World = GetWorld())
	{
		if (UGravityManagerSubsystem* GravitySubsystem = World->GetSubsystem<UGravityManagerSubsystem>())
		{
			if (bIsActivatedOnStartUp)
			{
				RegisterGravity();
				GravitySubsystem->OnGalaxyWakeUp.AddDynamic(this, &UGravityReceiverComponent::ApplyInitialVelocity);
			}
		}
	}
}

void UGravityReceiverComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

FVector UGravityReceiverComponent::CalculateInitialVelocity()
{
	if (!TargetPrimitive)
	{
		UE_LOG(LogTemp, Warning, TEXT("CalculateInitialVelocity: TargetPrimitive is None!"));
		return FVector::ZeroVector;
	}

	// 2. 获取已知参数
	// 太阳位置固定为 (0,0,0)
	const FVector SunPos = FVector::ZeroVector;
	const FVector PlanetPos = TargetPrimitive->GetComponentLocation();

	// 获取 G (根据你的需求从 Settings 获取)
	const float G = GetDefault<UOWSettings>()->GravityConstant;

	// 太阳质量 (默认为你提供的数值)
	const double SunMass = GetDefault<UOWSettings>()->SunMass;

	// 3. 计算半径向量 (r) 和 距离 (Distance)
	FVector RadiusVector = PlanetPos - SunPos;
	double Distance = RadiusVector.Size();
	Distance /= 100;

	// 防止距离过近导致除以零
	if (Distance < KINDA_SMALL_NUMBER)
	{
		return FVector::ZeroVector;
	}

	// 4. 计算标量速率 (Speed)
	// 公式: v = sqrt( (G * M) / r )
	double Speed = FMath::Sqrt((G * SunMass) / Distance);

	// 5. 计算速度方向
	// 假设轨道平面是 XY 平面，围绕 Z 轴旋转
	FVector OrbitAxis = FVector::UpVector; // (0, 0, 1)

	// 使用叉乘计算切线方向: Tangent = Axis x Radius
	// 这会产生一个垂直于半径和旋转轴的向量
	FVector TangentDirection = FVector::CrossProduct(OrbitAxis, RadiusVector).GetSafeNormal();

	// 如果需要反向旋转（顺时针），翻转方向
	if (bIsInitialClockWise)
	{
		TangentDirection *= -1.0;
	}

	// 6. 组合最终向量
	FVector ResultVelocity = TangentDirection * Speed * 100;

	return ResultVelocity;
}
