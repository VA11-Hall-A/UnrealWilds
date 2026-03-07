// Fill out your copyright notice in the Description page of Project Settings.


#include "Gravity/GravityWorldSubsystem.h"
#include "Gravity/GravityAsyncCallback.h"
#include "Physics/Experimental/PhysScene_Chaos.h"
#include "PBDRigidsSolver.h"
#include "GameFramework/CharacterMovementComponent.h"


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
	UpdateCMCGravities();
}

void UGravityWorldSubsystem::RegisterPlayerCharacter(UCharacterMovementComponent* InCMComponent)
{
	CMComponent=InCMComponent;
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

void UGravityWorldSubsystem::UpdateCMCGravities()
{
	//for (int i = 0; i < TrackedCharacterMovementComponents.Num(); i++)
	if (CMComponent !=nullptr)
	{
		// Compute the 
		FVector AdditionalAcceleration = FVector::ZeroVector; 
		for (auto& GravityAttractor: Sources)
		{
			if ( GravityAttractor->ApplyGravity)
			{
				FGravitySourceData GravityAttractorData = GravityAttractor->GetGravitySourceData();
				double Intensity=0.f;
				// Direction
				FVector Direction(GravityAttractorData.Location - CMComponent->GetActorLocation());
				
				if (GravityAttractorData.bUseInverseSquare)
				{
					double SquaredDistance = FVector::DotProduct(Direction, Direction); // We'll be using UE units here, no meters...
					Intensity = GravityAttractorData.MassDotG / SquaredDistance;
					
				}
				else
				{
					Intensity=GravityAttractorData.MassDotG/Direction.Length();
				}
				Direction.Normalize();
	 
				// Add the new acceleration to the force field.  
				AdditionalAcceleration += Intensity * Direction;
			}
		}
 
		DrawDebugDirectionalArrow(GetWorld(), CMComponent->GetActorLocation(), CMComponent->GetActorLocation() + AdditionalAcceleration, 1.0, FColor::Red, 0, false, 1.0f  );
		DrawDebugString(GetWorld(), CMComponent->GetActorLocation(), * FString::Printf(TEXT("%.2f"), AdditionalAcceleration.Length()), nullptr, FColor::Red, 0, false, 1.0f  );
		
		CMComponent->AddForce(AdditionalAcceleration);
		CMComponent->SetGravityDirection(AdditionalAcceleration.GetSafeNormal());
	}
}
