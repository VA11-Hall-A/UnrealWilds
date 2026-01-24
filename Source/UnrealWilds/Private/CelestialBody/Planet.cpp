// Fill out your copyright notice in the Description page of Project Settings.


#include "CelestialBody/Planet.h"
#include "Gravity/GravityManagerSubsystem.h"
#include "Gravity/GravityReceiverComponent.h"

APlanet::APlanet()
{
	BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
	GravityReceiver=CreateDefaultSubobject<UGravityReceiverComponent>(TEXT("GravityReceiver"));
	RootComponent = BodyMesh;
}

void APlanet::ApplyInitialVelocity()
{
	if (BodyMesh)
	{
		BodyMesh->SetPhysicsLinearVelocity(InitialVelocity);
	}
}

void APlanet::BeginPlay()
{
	Super::BeginPlay();

	// 静止
	BodyMesh->SetPhysicsLinearVelocity(FVector::ZeroVector);

	// --- 获取 GameState 并订阅 ---
	UWorld* World = GetWorld();
	if (World)
	{
		auto GravityManagerSubsystem = World->GetSubsystem<UGravityManagerSubsystem>();

		if (GravityManagerSubsystem)
		{
			// 绑定：当 GravityManagerSubsystem 喊话时，执行我的 ApplyInitialVelocity
			GravityManagerSubsystem->OnGalaxyWakeUp.AddDynamic(this, &APlanet::ApplyInitialVelocity);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Planet: Failed to find GravityManagerSubsystem!"));
		}
	}
	GravityReceiver->RegisterGravity();
	ApplyInitialVelocity();
}
