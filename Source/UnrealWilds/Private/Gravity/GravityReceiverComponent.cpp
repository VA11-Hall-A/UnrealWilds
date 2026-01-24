// Fill out your copyright notice in the Description page of Project Settings.


#include "Gravity/GravityReceiverComponent.h"
#include "Gravity/GravityManagerSubsystem.h"

// Sets default values for this component's properties
UGravityReceiverComponent::UGravityReceiverComponent()
{

	PrimaryComponentTick.bCanEverTick = false;
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

	// 3. 确定要注册哪个物理组件
	// 如果 CachedPrimitive 为空，说明是第一次注册或者之前被清空了，需要重新寻找
	if (!CachedPrimitive)
	{
		CachedPrimitive = TargetPrimitive;
        
		// 如果没有手动指定 TargetPrimitive，尝试获取 Owner 的 RootComponent
		if (!CachedPrimitive)
		{
			CachedPrimitive = Cast<UPrimitiveComponent>(GetOwner()->GetRootComponent());
		}
	}

	// 4. 执行注册
	// 确保组件存在且开启了物理模拟 (IsSimulatingPhysics 检查通常很有必要)
	if (CachedPrimitive && CachedPrimitive->IsSimulatingPhysics())
	{
		GravitySubsystem->RegisterGravityReceiver(CachedPrimitive);
		bIsRegistered = true; // 标记状态
	}
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
		// 只有当 CachedPrimitive 有效时才需要注销
		if (CachedPrimitive)
		{
			GravitySubsystem->UnregisterGravityReceiver(CachedPrimitive);
		}
	}

	// 3. 重置状态
	bIsRegistered = false;
	// 注意：这里不要把 CachedPrimitive 置为 nullptr，因为你可能之后还要再次 RegisterGravity()，
	// 到时候还需要用这个指针。除非你确定 EndPlay 后组件就销毁了。
}


// Called when the game starts
void UGravityReceiverComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

void UGravityReceiverComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}




