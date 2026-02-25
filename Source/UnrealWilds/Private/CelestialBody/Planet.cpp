// Fill out your copyright notice in the Description page of Project Settings.


#include "CelestialBody/Planet.h"
#include "Gravity/GravityManagerSubsystem.h"
#include "Gravity/GravityReceiverComponent.h"

APlanet::APlanet()
{
	GravityReceiver=CreateDefaultSubobject<UGravityReceiverComponent>(TEXT("GravityReceiver"));
}



void APlanet::BeginPlay()
{
	Super::BeginPlay();

	// 静止
	BodyMesh->SetPhysicsLinearVelocity(FVector::ZeroVector);
	
}
