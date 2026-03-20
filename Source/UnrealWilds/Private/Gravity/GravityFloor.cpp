#include "Gravity/GravityFloor.h"
#include "Gravity/GravityWorldSubsystem.h"
#include "Character/UWCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Pawn/PlanetAttachmentComponent.h"
#include "Astro/Planet.h"
#include "GameFramework/CharacterMovementComponent.h"

AGravityFloor::AGravityFloor()
{
	PrimaryActorTick.bCanEverTick = false;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	DetectionVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("DetectionVolume"));
	DetectionVolume->SetupAttachment(Root);
	DetectionVolume->SetBoxExtent(BoxExtent);
	DetectionVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	DetectionVolume->SetCollisionObjectType(ECC_WorldDynamic);
	DetectionVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	DetectionVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void AGravityFloor::BeginPlay()
{
	Super::BeginPlay();

	DetectionVolume->SetBoxExtent(BoxExtent);
	DetectionVolume->OnComponentBeginOverlap.AddDynamic(this, &AGravityFloor::OnVolumeBeginOverlap);
	DetectionVolume->OnComponentEndOverlap.AddDynamic(this, &AGravityFloor::OnVolumeEndOverlap);
}

FVector AGravityFloor::GetGravityDirection() const
{
	return -GetActorUpVector();
}

double AGravityFloor::GetGravityAcceleration() const
{
	return GravityAcceleration;
}

void AGravityFloor::OnVolumeBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AUWCharacter* Character = Cast<AUWCharacter>(OtherActor);
	if (!Character || !Character->IsLocallyControlled())
	{
		return;
	}

	// 若角色处于 ZeroG，先切换到 SurfaceGravity
	if (Character->GetCurrentMovementState() == ECharacterMovementState::ZeroG)
	{
		Character->EnterSurfaceGravity(-GetActorUpVector());
	}

	UGravityWorldSubsystem* Sub = GetWorld()->GetSubsystem<UGravityWorldSubsystem>();
	if (Sub)
	{
		Sub->RegisterGravityFloor(this);
	}

	// 立即设置重力方向，不等待 subsystem 下一帧 tick
	Character->GetCharacterMovement()->SetGravityDirection(GetGravityDirection());
}

void AGravityFloor::OnVolumeEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AUWCharacter* Character = Cast<AUWCharacter>(OtherActor);
	if (!Character || !Character->IsLocallyControlled())
	{
		return;
	}

	UGravityWorldSubsystem* Sub = GetWorld()->GetSubsystem<UGravityWorldSubsystem>();
	if (Sub)
	{
		Sub->UnregisterGravityFloor(this);

		if (!Sub->IsGravityFloorActive())
		{
			DetermineExitState(Character);
		}
	}
}

void AGravityFloor::DetermineExitState(AUWCharacter* Character)
{
	UPlanetAttachmentComponent* Attachment = Character->FindComponentByClass<UPlanetAttachmentComponent>();
	if (!Attachment)
	{
		Character->EnterZeroG(Character->GetCharacterMovement()->Velocity);
		return;
	}

	if (Attachment->IsAttachedToPlanet())
	{
		APlanet* Planet = Attachment->GetCurrentPlanet();
		if (Planet && Planet->HollowInnerSphere)
		{
			float Dist = FVector::Dist(Character->GetActorLocation(), Planet->GetActorLocation());
			float HollowRadius = Planet->HollowInnerSphere->GetScaledSphereRadius();
			if (Dist < HollowRadius)
			{
				// 在空洞内部，进入零重力
				Character->EnterZeroG(Character->GetCharacterMovement()->Velocity);
				return;
			}
		}
		// 在行星表面，subsystem 自然恢复行星引力，无需操作
	}
	else
	{
		// 未附着行星（太空中），进入零重力
		Character->EnterZeroG(Character->GetCharacterMovement()->Velocity);
	}
}
