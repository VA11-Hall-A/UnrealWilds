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
						double Distance =Direction.Length();
						Distance = FMath::Max(Distance, UE_DOUBLE_SMALL_NUMBER);
						double SquaredDistance = FVector::DotProduct(Direction, Direction); // We'll be using UE units here, no meters... 

						// Intensity
						double Intensity = 0.f;
						bool bIsInsidePlanet = Distance < GravityAttractorData.PlanetRadius;
						
						if (GravityAttractorData.bUseInverseSquare)
						{
							if (bIsInsidePlanet)
							{
								// 星球内部：引力与距离中心点成正比 (均匀球体模型)
								double R3 = FMath::Pow(GravityAttractorData.PlanetRadius, 3.0);
								Intensity = (GravityAttractorData.MassDotG / R3) * Distance;
							}
							else
							{
								// 星球外部：标准的平方反比定律
								Intensity = GravityAttractorData.MassDotG / SquaredDistance;
							}
						}
						else
						{
							// 非平方反比的 Gameplay 逻辑分支
							if (bIsInsidePlanet)
							{
								// 为了保证穿过地表时引力的平滑过渡，内部同样做线性插值递减到地心为0
								double SurfaceIntensity = GravityAttractorData.MassDotG / GravityAttractorData.PlanetRadius;
								Intensity = SurfaceIntensity * (Distance / GravityAttractorData.PlanetRadius);
							}
							else
							{
								Intensity = GravityAttractorData.MassDotG / Distance;
							}
						}

						// 使用已经计算好的 Distance 进行归一化，替代原有的 Direction.Normalize() 节省性能
						Direction /= Distance;

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
