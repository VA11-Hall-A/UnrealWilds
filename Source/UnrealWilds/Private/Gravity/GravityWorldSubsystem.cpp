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

	if (CMComponent)
	{
		TimeSinceLastOriginRebase += DeltaTime;
		// 定时将actor（世界中心）重新设定，避免远离中心导致的浮点数精度丢失问题
		// Outer Wilds等星际探索游戏常见做法：每隔一段时间（或距离），将世界原点平移至玩家当前位置
		if (TimeSinceLastOriginRebase >= 10.0f) 
		{
			TimeSinceLastOriginRebase = 0.0f;
			FVector ActorLoc = CMComponent->GetActorLocation();
			
			// 可以根据需要添加距离判定，只有超过一定距离才重置原点，这里简单判断一下避免过频
			if (ActorLoc.SizeSquared() > FMath::Square(5000.0))
			{
				if (UWorld* World = GetWorld())
				{
					// RequestNewWorldOrigin在帧末尾安全地将世界原点平移到玩家所在坐标
					World->SetNewWorldOrigin(FIntVector(ActorLoc.X, ActorLoc.Y, ActorLoc.Z) + World->OriginLocation);
					if (GEngine)
					{
						GEngine->AddOnScreenDebugMessage(
							-1,              // Key: -1 表示每次都创建新消息，不覆盖旧的
							5.0f,            // Duration: 显示时长（秒）
							FColor::Yellow,  // Color: 文本颜色
							TEXT("Hello World! This is a screen log.") // 消息内容
						);
					}
				}
			}
		}
	}
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
	if (CMComponent !=nullptr)
	{
		// 角色如果不受 CMC 控制（如在零重力模式下开启了物理模拟，MovementMode == MOVE_None)
		// 则直接跳过由 CMC 施加引力和强制方向的逻辑。引力将由 Chaos AsyncCallback 自然接管。
		if (CMComponent->MovementMode == MOVE_None)
		{
			return;
		}

		FVector AdditionalAcceleration = FVector::ZeroVector; 
		for (auto& GravityAttractor: Sources)
		{
			if (GravityAttractor.IsValid() && GravityAttractor->ApplyGravity)
			{
				FGravitySourceData GravityAttractorData = GravityAttractor->GetGravitySourceData();
				
				// 1. 优先获取平方距离 (避免过早使用昂贵的 Sqrt 开方操作)
				FVector Direction = GravityAttractorData.Location - CMComponent->GetActorLocation();
				double SquaredDistance = Direction.SquaredLength();
				
				double HollowRadiusSq = FMath::Square(GravityAttractorData.HollowRadius); 
				
				// 2. Early-Out 优化：如果处于空洞内部，不受引力影响，直接跳过计算
				if (SquaredDistance <= HollowRadiusSq)
				{
					continue;
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
						TRACE_CPUPROFILER_EVENT_SCOPE_STR("IsInsidePlanet")
						// 线性插值重映射：保证从地表的 SurfaceIntensity 平滑递减到空洞边界(HollowRadius)的 0
						double SurfaceIntensity = GravityAttractorData.MassDotG / GravityAttractorData.PlanetRadius;
						
						Intensity = SurfaceIntensity * ((Distance - GravityAttractorData.HollowRadius) / (GravityAttractorData.PlanetRadius - GravityAttractorData.HollowRadius));
					}
					else
					{
						Intensity = GravityAttractorData.MassDotG / Distance;
					}
				}
				
				// 使用已经计算好的 Distance 进行归一化
				Direction /= Distance;
	 
				// Add the new acceleration to the force field.  
				AdditionalAcceleration += Intensity * Direction;
			}
		}
 
		DrawDebugDirectionalArrow(GetWorld(), CMComponent->GetActorLocation(), CMComponent->GetActorLocation() + AdditionalAcceleration, 1.0, FColor::Red, 0, false, 1.0f  );
		DrawDebugString(GetWorld(), CMComponent->GetActorLocation(), * FString::Printf(TEXT("%.2f"), AdditionalAcceleration.Length()), nullptr, FColor::Red, 0, false, 1.0f  );
		
		CMComponent->AddForce(AdditionalAcceleration*CMComponent->Mass);
		
		// 只有当受引力影响时，才更改角色的重力方向和旋转，避免引力为 0 时 SetGravityDirection(0,0,0) 导致问题
		if (!AdditionalAcceleration.IsNearlyZero())
		{
			CMComponent->SetGravityDirection(AdditionalAcceleration.GetSafeNormal());
		}
	}
}
