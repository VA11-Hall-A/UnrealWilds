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
						// 1. 优先获取平方距离 (避免过早使用昂贵的 Sqrt 开方操作)
						FVector Direction = GravityAttractorData.Location - ActiveParticle.GetX();
						double SquaredDistance = Direction.SquaredLength(); // 替代原有的 DotProduct，语义更清晰，性能等价
						
						// 强烈建议在引力源初始化/更新时预计算 HollowRadiusSq，避免每个粒子每帧计算
						double HollowRadiusSq = FMath::Square(GravityAttractorData.HollowRadius); 
						
						// 2. Early-Out 优化：如果处于空洞内部，不受引力影响，直接跳过所有昂贵计算
						if (SquaredDistance <= HollowRadiusSq)
						{
							// 如果是在一个遍历粒子的 for 循环中，这里写 continue;
							// 如果这是一个单独处理粒子的函数，这里写 return;
							// 这里假设用 if 跳过，你可以根据上下文调整：
							return; 
						}
						
						// 3. 只有在需要计算引力时，才执行 Sqrt
						double Distance = FMath::Sqrt(SquaredDistance);
						Distance = FMath::Max(Distance, UE_DOUBLE_SMALL_NUMBER);
						
						double Intensity = 0.f;
						bool bIsInsidePlanet = Distance < GravityAttractorData.PlanetRadius;
						
						if (GravityAttractorData.bUseInverseSquare)
						{
							if (bIsInsidePlanet)
							{
							   // 星球地壳内部：基于均匀球壳定理 (Shell Theorem)
							   // 公式确保了在 HollowRadius 处引力为 0，且向地表平滑过渡
							   double HollowR3 = HollowRadiusSq * GravityAttractorData.HollowRadius; // R_in^3
							   double PlanetR3 = FMath::Pow(GravityAttractorData.PlanetRadius, 3.0); // R_out^3
							   double CurrentR3 = Distance * SquaredDistance; // 避免使用 FMath::Pow 计算当前距离的3次方
							   
							   // 建议预计算常量: double InvShellVolume = 1.0 / (PlanetR3 - HollowR3);
							   Intensity = (GravityAttractorData.MassDotG / SquaredDistance) * ((CurrentR3 - HollowR3) / (PlanetR3 - HollowR3));
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
							   // 线性插值重映射：保证从地表的 SurfaceIntensity 平滑递减到空洞边界(HollowRadius)的 0
							   double SurfaceIntensity = GravityAttractorData.MassDotG / GravityAttractorData.PlanetRadius;
							   
							   // 建议预计算常量: double InvCrustThickness = 1.0 / (GravityAttractorData.PlanetRadius - GravityAttractorData.HollowRadius);
							   Intensity = SurfaceIntensity * ((Distance - GravityAttractorData.HollowRadius) / (GravityAttractorData.PlanetRadius - GravityAttractorData.HollowRadius));
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
