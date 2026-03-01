#include "Gravity/GravityAsyncCallback.h"
#include "PBDRigidsSolver.h"
#include "Chaos/DebugDrawQueue.h"

using namespace Chaos;

double FGravityAsyncCallback::GravitationalConstant = 6.67430E-11;

FGravityAsyncCallback::FGravityAsyncCallback()
{
}

FGravityAsyncCallback::~FGravityAsyncCallback()
{
}

void FGravityAsyncCallback::OnPreSimulate_Internal()
{
}

void FGravityAsyncCallback::OnPreIntegrate_Internal()
{
	// We are running on the PT here... 
	if (FPBDRigidsSolver* MySolver = static_cast<FPBDRigidsSolver*>(GetSolver()))
	{
		// Get a reference to the PT input structure
		const FGravityAsyncInput* Input = GetConsumerInput_Internal();
		if (Input)
		{
			// Iterate over all the currenly simulated rigid bodies - They are named Particles in Chaos. 
			TParticleView<FPBDRigidParticles> ActiveParticles = MySolver->GetParticles().GetNonDisabledDynamicView();

			for (auto& ActiveParticle : ActiveParticles)
			{
				if (ActiveParticle.Handle())
				{
					// Draw current acceleration
					//FDebugDrawQueue::GetInstance().DrawDebugDirectionalArrow(ActiveParticle.GetX(), ActiveParticle.GetX() + ActiveParticle.Acceleration() , 20.f, FColor::Yellow, false, 0, 0, 1.f);

					FVector AdditionalAcceleration = FVector::ZeroVector;
					// Compute the combined forces of all attractors
					for (auto& GravityAttractorData : Input->GravitySourceData)
					{
						// Direction
						FVector Direction(GravityAttractorData.Location - ActiveParticle.GetX());
						double SquaredDistance = FVector::DotProduct(Direction, Direction); // We'll be using UE units here, no meters... 

						// Intensity
						double Intensity = 0.f;
						if (GravityAttractorData.bUseInverseSquare)
						{
							Intensity = GravityAttractorData.MassDotG / SquaredDistance;
						}
						else
						{
							Intensity=GravityAttractorData.MassDotG / Direction.Length();
						}
						Direction.Normalize();

						// Add the new acceleration to the force field.  
						AdditionalAcceleration += Intensity * Direction;

						// Debug draw
						{
							//FDebugDrawQueue::GetInstance().DrawDebugLine(ActiveParticle.GetX(), GravityAttractorData.Location, FColor::Magenta, false, 0, 0, 1.f);
							//FDebugDrawQueue::GetInstance().DrawDebugDirectionalArrow(ActiveParticle.GetX(), ActiveParticle.GetX() + Intensity * Direction, 10.f, FColor::White, false, 0, 0, 1.f);	
							//FDebugDrawQueue::GetInstance().DrawDebugString(ActiveParticle.GetX() + Intensity * Direction, * FString::Printf(TEXT("%.2f"), Intensity),nullptr, FColor::White, 0, false, 1.0f  );
						}
					}

					{
						FDebugDrawQueue::GetInstance().DrawDebugDirectionalArrow(ActiveParticle.GetX(),
						                                                         ActiveParticle.GetX() + ActiveParticle.Acceleration() + AdditionalAcceleration,
						                                                         20.f, FColor::Emerald, false, 0, 0, 2.f);
						//FDebugDrawQueue::GetInstance().DrawDebugDirectionalArrow(ActiveParticle.GetX(), ActiveParticle.GetX() - ActiveParticle.GetR().GetUpVector()* (ActiveParticle.Acceleration() + AdditionalAcceleration).Length(), 20.f, FColor::Cyan, false, 0, 0, 1.f);
						//FDebugDrawQueue::GetInstance().DrawDebugString(ActiveParticle.GetX(), * FString::Printf(TEXT("I:%.2f / A:%.2f"), ActiveParticle.Acceleration().Length(), AdditionalAcceleration.Length()),nullptr, FColor::Red, 0, false, 1.0f  );
					}

					// Add the force field value to the rigid body. 
					ActiveParticle.SetAcceleration(ActiveParticle.Acceleration() + AdditionalAcceleration);
				}
			}
		}
	}
}
