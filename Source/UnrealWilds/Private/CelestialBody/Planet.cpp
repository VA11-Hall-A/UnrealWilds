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



void APlanet::BeginPlay()
{
	Super::BeginPlay();

	// 静止
	BodyMesh->SetPhysicsLinearVelocity(FVector::ZeroVector);
	
}
