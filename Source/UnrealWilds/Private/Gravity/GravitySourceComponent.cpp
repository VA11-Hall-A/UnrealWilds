// Fill out your copyright notice in the Description page of Project Settings.


#include "Gravity/GravitySourceComponent.h"
#include "Gravity/GravityWorldSubsystem.h"

// Sets default values for this component's properties
UGravitySourceComponent::UGravitySourceComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UGravitySourceComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UGravitySourceComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
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

