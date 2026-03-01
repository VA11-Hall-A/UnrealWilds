// Fill out your copyright notice in the Description page of Project Settings.


#include "Gravity/GravityWorldSubsystem.h"
#include "Gravity/GravityAsyncCallback.h"
#include "Physics/Experimental/PhysScene_Chaos.h"
#include "PBDRigidsSolver.h"

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
	GEngine->AddOnScreenDebugMessage( -1, 0, FColor::Green, *FString::Printf(TEXT("%d Attractors in the World"), Sources.Num()));
 
	if (!IsAsyncCallbackRegistered())
	{
		RegisterAsyncCallback();
	}
}

TStatId UGravityWorldSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UGravityWorldSubSystem, STATGROUP_Tickables);
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

void UGravityWorldSubsystem::RegisterAsyncCallback()
{
	if (UWorld* World = GetWorld())
	{
		if (FPhysScene* PhysScene = World->GetPhysicsScene())
		{
			AsyncCallback = PhysScene->GetSolver()->CreateAndRegisterSimCallbackObject_External<FGravityAsyncCallback>();
		}
	}
}

bool UGravityWorldSubsystem::IsAsyncCallbackRegistered() const
{
	return AsyncCallback != nullptr;
}

void UGravityWorldSubsystem::AddGravitySourceData(const FGravitySourceData& InputData) const
{
	if (IsAsyncCallbackRegistered())
	{
		FGravityAsyncInput* Input=AsyncCallback->GetProducerInputData_External();
		Input->GravitySourceData.Add(InputData);
	}
}
