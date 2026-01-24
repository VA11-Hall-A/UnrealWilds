// Fill out your copyright notice in the Description page of Project Settings.


#include "Gravity/GravityManagerSubsystem.h"
#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"
#include "Gravity/GravitySourceComponent.h"
#include "EngineUtils.h"
#include "Physics/Experimental/PhysScene_Chaos.h"

void UGravityManagerSubsystem::RegisterGravitySource(UGravitySourceComponent* Source)
{
	GravitySources.AddUnique(Source);
	UWorld* World = GetWorld();
	if (World && !PhysPreTickHandle.IsValid())
	{
		FPhysScene_Chaos* PhysScene = World->GetPhysicsScene();
		if (PhysScene)
		{
			PhysPreTickHandle = PhysScene->OnPhysScenePreTick.AddUObject(this, &UGravityManagerSubsystem::OnPhysScenePreTick);
		}
	}
}

void UGravityManagerSubsystem::UnregisterGravitySource(UGravitySourceComponent* Source)
{
	GravitySources.Remove(Source);
	if (GravitySources.Num() == 0)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			FPhysScene_Chaos* PhysScene = World->GetPhysicsScene();
			if (PhysScene && PhysPreTickHandle.IsValid())
			{
				PhysScene->OnPhysScenePreTick.Remove(PhysPreTickHandle);
				PhysPreTickHandle.Reset();
			}
		}
	}
}

void UGravityManagerSubsystem::RegisterGravityReceiver(UPrimitiveComponent* PrimitiveComp)
{
	if (PrimitiveComp)
	{
		GravityReceivers.AddUnique(PrimitiveComp);
	}
}

void UGravityManagerSubsystem::UnregisterGravityReceiver(UPrimitiveComponent* PrimitiveComp)
{
	if (PrimitiveComp)
	{
		// 移除所有匹配的 WeakPtr
		GravityReceivers.RemoveAll([PrimitiveComp](const TWeakObjectPtr<UPrimitiveComponent>& Ptr)
		{
			return Ptr.Get() == PrimitiveComp;
		});
	}
}

void UGravityManagerSubsystem::TriggerGalaxyWakeUp()
{
	if (OnGalaxyWakeUp.IsBound())
	{
		OnGalaxyWakeUp.Broadcast();
        
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Purple, TEXT("GameState: Waking up all planets!"));
	}
}

void UGravityManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UGravityManagerSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UGravityManagerSubsystem::OnPhysScenePreTick(FPhysScene_Chaos* PhysScene, float DeltaTime)
{
	// 如果没有引力源，直接跳过
	if (GravitySources.Num() == 0) return;

	// 倒序遍历，方便在遍历中安全移除无效元素
	for (int32 i = GravityReceivers.Num() - 1; i >= 0; --i)
	{
		UPrimitiveComponent* Comp = GravityReceivers[i].Get();

		// 1. 检查组件是否还存在
		if (!Comp)
		{
			GravityReceivers.RemoveAt(i);
			continue;
		}

		// 2. 检查是否开启了物理模拟（可能在运行时被关闭）
		if (!Comp->IsSimulatingPhysics())
		{
			continue;
		}

		AActor* OwnerActor = Comp->GetOwner();
		if (!OwnerActor) continue;

		FVector TotalGravity = FVector::ZeroVector;
		FVector Location = Comp->GetComponentLocation();
		bool bOwnerIsPlanet = OwnerActor->ActorHasTag(FName("Planet"));

		// 叠加所有引力源的引力
		for (TWeakObjectPtr Source : GravitySources)
		{
			if (Source == nullptr) continue;
			AActor* SourceOwner = Source->GetOwner();
			if (SourceOwner == nullptr) continue;

			// 逻辑保持不变：行星之间互不施加引力
			bool bSourceIsPlanet = (Source->SourceType == EGravitySourceType::Planet);
			if (bSourceIsPlanet && bOwnerIsPlanet) continue;

			// 避免自己吸引自己 (例如星球也是物理模拟物体时)
			if (SourceOwner == OwnerActor) continue;
			TotalGravity += Source->GetGravityAtPoint(Location);
		}
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				1, // 使用 -1 确保每帧都会产生新消息，或者使用特定 Key 覆盖旧消息
				1, // 显示持续时间（秒）
				FColor::Yellow, // 字体颜色
				FString::Printf(TEXT("Applied acceleration: x = %.17f,y = %.17f, z = %.17f"), TotalGravity.X, TotalGravity.Y, TotalGravity.Z)
			);
		}

		// 施加引力
		if (!TotalGravity.IsZero())
		{
			Comp->AddForce(TotalGravity * Comp->GetMass() * 100);
		}
	}
}
