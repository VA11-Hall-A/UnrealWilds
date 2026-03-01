// Fill out your copyright notice in the Description page of Project Settings.


#include "Gravity/GravitySourceComponent.h"
#include "Gravity/GravityWorldSubsystem.h"
#include "Gravity/GravityAsyncCallback.h"

// Sets default values for this component's properties
UGravitySourceComponent::UGravitySourceComponent()
{

	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true; 
	PrimaryComponentTick.TickGroup = ETickingGroup::TG_PrePhysics; 
}


FGravitySourceData UGravitySourceComponent::GetGravitySourceData() const
{
	FGravitySourceData GravitySourceData;
	GravitySourceData.Location=GetComponentLocation();
	GravitySourceData.bUseInverseSquare=bUseInverseSquare;
	if (bUseGravityAtRadius)
	{
		GravitySourceData.MassDotG=Gravity*Radius*Radius;
	}
	else
	{
		GravitySourceData.MassDotG = Mass * 6.67430E-5;
	}
	return GravitySourceData;
}

// Called when the game starts
void UGravitySourceComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

void UGravitySourceComponent::BuildAsyncInput()
{
	if (ApplyGravity)
	{
		if (UWorld* World = GetWorld())
		{
			if (UGravityWorldSubsystem* GravitySubsystem=World->GetSubsystem<UGravityWorldSubsystem>())
			{
				GravitySubsystem->AddGravitySourceData(GetGravitySourceData());
			}
		}
	}
}


// Called every frame
void UGravitySourceComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	BuildAsyncInput();
}

void UGravitySourceComponent::OnRegister()
{
	Super::OnRegister();
	if (UWorld* World = GetWorld())
	{
		if (UGravityWorldSubsystem* GravitySubsystem=World->GetSubsystem<UGravityWorldSubsystem>())
		{
			GravitySubsystem->AddSource(this);
		}
	}
}

void UGravitySourceComponent::OnUnregister()
{
	Super::OnUnregister();
	if (UWorld* World = GetWorld())
	{
		if (UGravityWorldSubsystem* GravitySubsystem=World->GetSubsystem<UGravityWorldSubsystem>())
		{
			GravitySubsystem->RemoveSource(this);
		}
	}
}

