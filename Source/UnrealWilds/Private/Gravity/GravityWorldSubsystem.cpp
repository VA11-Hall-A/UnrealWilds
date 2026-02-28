// Fill out your copyright notice in the Description page of Project Settings.


#include "Gravity/GravityWorldSubsystem.h"

void UGravityWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UGravityWorldSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UGravityWorldSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
}

TStatId UGravityWorldSubsystem::GetStatId() const
{
	return Super::GetStatId();
}

void UGravityWorldSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void UGravityWorldSubsystem::AddSource(UGravitySourceComponent* GravityAttractorComponent)
{
	Sources.Add(GravityAttractorComponent);
}

void UGravityWorldSubsystem::RemoveSource(UGravitySourceComponent* GravityAttractorComponent)
{
	Sources.Remove(GravityAttractorComponent);
}
