#include "Gravity/GravityFloor.h"
#include "Gravity/GravityWorldSubsystem.h"
#include "Character/UWCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Pawn/PlanetAttachmentComponent.h"
#include "Astro/Planet.h"
#include "GameFramework/CharacterMovementComponent.h"

AGravityFloor::AGravityFloor()
{
	PrimaryActorTick.bCanEverTick = false;

	FloorMesh=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FloorMesh"));
	SetRootComponent(FloorMesh);

	EntryVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("EntryVolume"));
	EntryVolume->SetupAttachment(FloorMesh);
	EntryVolume->SetBoxExtent(BoxExtent);
	EntryVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	EntryVolume->SetCollisionObjectType(ECC_WorldDynamic);
	EntryVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	EntryVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	DetectionVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("DetectionVolume"));
	DetectionVolume->SetupAttachment(FloorMesh);
	DetectionVolume->SetBoxExtent(BoxExtent);
	DetectionVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	DetectionVolume->SetCollisionObjectType(ECC_WorldDynamic);
	DetectionVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	DetectionVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void AGravityFloor::BeginPlay()
{
	Super::BeginPlay();
	
	EntryVolume->OnComponentBeginOverlap.AddDynamic(this, &AGravityFloor::OnEntryVolumeBeginOverlap);
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

void AGravityFloor::OnEntryVolumeBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AUWCharacter* Character = Cast<AUWCharacter>(OtherActor);
	if (!Character || !Character->IsEnvironmentProbe(OtherComp) || !Character->IsLocallyControlled())
	{
		return;
	}

	UGravityWorldSubsystem* Sub = GetWorld()->GetSubsystem<UGravityWorldSubsystem>();
	if (Sub)
	{
		Sub->RegisterGravityFloor(this);
	}

	Character->StartGravityFloorEntryTransition(this);
}

void AGravityFloor::OnVolumeEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AUWCharacter* Character = Cast<AUWCharacter>(OtherActor);
	if (!Character || !Character->IsEnvironmentProbe(OtherComp) || !Character->IsLocallyControlled())
	{
		return;
	}

	UGravityWorldSubsystem* Sub = GetWorld()->GetSubsystem<UGravityWorldSubsystem>();
	if (Sub)
	{
		if (Character->IsTransitioningToGravityFloor(this))
		{
			Character->AbortGravityFloorEntryTransition();
		}

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
			const float Dist = FVector::Dist(Character->GetEnvironmentProbeLocation(), Planet->GetActorLocation());
			const float HollowRadius = Planet->HollowInnerSphere->GetScaledSphereRadius();
			if (Dist < HollowRadius)
			{
				Character->EnterZeroG(Character->GetCharacterMovement()->Velocity);
				return;
			}
		}
		// Attached to planet, outside hollow: transition back to planet gravity
		Character->EnterSurfaceGravity();
	}
	else
	{
		Character->EnterZeroG(Character->GetCharacterMovement()->Velocity);
	}
}
