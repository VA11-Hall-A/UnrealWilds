#include "Pawn/ShipPawn.h"

#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Pawn/ThrusterComponent.h"
#include "Probe/ProbeLauncherComponent.h"
#include "Character/UWCharacter.h"
#include "Astro/Planet.h"
#include "EngineUtils.h"

AShipPawn::AShipPawn()
{
	PrimaryActorTick.bCanEverTick = false;

	// Ship mesh as root, physics-driven
	ShipMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShipMesh"));
	SetRootComponent(ShipMesh);
	ShipMesh->SetSimulatePhysics(true);
	ShipMesh->SetEnableGravity(false);
	ShipMesh->SetCollisionObjectType(ECC_Pawn);
	ShipMesh->SetGenerateOverlapEvents(true);

	// Camera attached to ship mesh, follows physics rotation
	ShipCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ShipCamera"));
	ShipCamera->SetupAttachment(ShipMesh);
	ShipCamera->bUsePawnControlRotation = false;

	// Thruster in physics mode
	Thruster = CreateDefaultSubobject<UThrusterComponent>(TEXT("Thruster"));
	Thruster->ThrustForce = 1000.0;
	Thruster->bIsCharacterMode = false;

	// Probe launcher (reuses existing component)
	ProbeLauncher = CreateDefaultSubobject<UProbeLauncherComponent>(TEXT("ProbeLauncher"));

	// Interaction zone for boarding
	InteractionZone = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionZone"));
	InteractionZone->SetupAttachment(ShipMesh);
	InteractionZone->SetSphereRadius(500.0f);
	InteractionZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionZone->SetCollisionObjectType(ECC_WorldDynamic);
	InteractionZone->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionZone->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	InteractionZone->SetGenerateOverlapEvents(true);
}

void AShipPawn::BeginPlay()
{
	Super::BeginPlay();
	CheckInitialPlanetState();
}

void AShipPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (LookAction)
		{
			EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &AShipPawn::ShipLook);
		}
		if (FlyingMoveAction)
		{
			EIC->BindAction(FlyingMoveAction, ETriggerEvent::Triggered, this, &AShipPawn::ShipFlyingMove);
		}
		if (LaunchProbeAction)
		{
			EIC->BindAction(LaunchProbeAction, ETriggerEvent::Triggered, ProbeLauncher.Get(), &UProbeLauncherComponent::LaunchProbe);
		}
		if (RecallProbeAction)
		{
			EIC->BindAction(RecallProbeAction, ETriggerEvent::Triggered, ProbeLauncher.Get(), &UProbeLauncherComponent::RecallProbe);
		}
		if (InteractAction)
		{
			EIC->BindAction(InteractAction, ETriggerEvent::Triggered, this, &AShipPawn::OnInteract);
		}
	}
}

void AShipPawn::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (APlayerController* PC = Cast<APlayerController>(NewController))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (ShipMappingContext)
			{
				Subsystem->AddMappingContext(ShipMappingContext, 0);
			}
		}
	}
}

void AShipPawn::UnPossessed()
{
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (ShipMappingContext)
			{
				Subsystem->RemoveMappingContext(ShipMappingContext);
			}
		}
	}

	Super::UnPossessed();
}

// ── Input Handlers ──────────────────────────────────────────────────────────

void AShipPawn::ShipLook(const FInputActionValue& Value)
{
	const FVector LookAxis = Value.Get<FVector>();

	FVector Torque = GetActorUpVector() * LookAxis.X * TorqueMultiplier
		+ GetActorRightVector() * LookAxis.Y * TorqueMultiplier;

	ShipMesh->AddTorqueInRadians(Torque, NAME_None, true); // bAccelChange = true
}

void AShipPawn::ShipFlyingMove(const FInputActionValue& Value)
{
	FVector MovementVector = Value.Get<FVector>();
	FVector FinalDirection = GetActorForwardVector() * MovementVector.Y
		+ GetActorRightVector() * MovementVector.X
		+ GetActorUpVector() * MovementVector.Z;

	if (Thruster)
	{
		Thruster->AddForceToMovemntComponent(FinalDirection);
	}
}

void AShipPawn::OnInteract()
{
	OnExitShip();
}

// ── Boarding ────────────────────────────────────────────────────────────────

void AShipPawn::BoardShip(AUWCharacter* Character)
{
	if (!Character || StoredCharacter)
	{
		return;
	}

	AController* CharController = Character->GetController();
	if (!CharController)
	{
		return;
	}

	StoredCharacter = Character;

	// Hide and disable character
	Character->SetActorHiddenInGame(true);
	Character->SetActorEnableCollision(false);
	Character->SetActorLocation(GetActorLocation());

	// Possess the ship
	CharController->Possess(this);
}

void AShipPawn::OnExitShip()
{
	if (!StoredCharacter)
	{
		return;
	}

	AController* ShipController = GetController();
	if (!ShipController)
	{
		return;
	}

	AUWCharacter* Character = StoredCharacter;
	StoredCharacter = nullptr;

	// Position character near the ship
	FVector ExitLocation = GetActorLocation() + GetActorRightVector() * 300.0f;
	Character->SetActorLocation(ExitLocation);

	// Restore character visibility and collision
	Character->SetActorHiddenInGame(false);
	Character->SetActorEnableCollision(true);

	// Possess the character back
	ShipController->Possess(Character);

	// Character will restore its own input context in PossessedBy,
	// and determine correct gravity state
	Character->CheckInitialMovementState();
}

// ── Planet Gravity ──────────────────────────────────────────────────────────

void AShipPawn::OnEnterPlanetGravity(APlanet* Planet)
{
	if (!Planet)
	{
		return;
	}

	CurrentPlanet = Planet;
	AttachToActor(Planet, FAttachmentTransformRules::KeepWorldTransform);
}

void AShipPawn::OnExitPlanetGravity()
{
	if (!CurrentPlanet)
	{
		return;
	}

	// Read current physics velocity and add orbital velocity for momentum inheritance
	FVector PhysicsVelocity = ShipMesh->GetPhysicsLinearVelocity();
	FVector OrbitalVelocity = CurrentPlanet->GetOrbitalVelocity();

	CurrentPlanet = nullptr;
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	ShipMesh->SetPhysicsLinearVelocity(PhysicsVelocity + OrbitalVelocity);
}

FVector AShipPawn::GetVelocity() const
{
	FVector PhysicsVelocity = ShipMesh ? ShipMesh->GetPhysicsLinearVelocity() : FVector::ZeroVector;

	if (CurrentPlanet)
	{
		PhysicsVelocity += CurrentPlanet->GetOrbitalVelocity();
	}

	return PhysicsVelocity;
}

// ── Initial State ───────────────────────────────────────────────────────────

void AShipPawn::CheckInitialPlanetState()
{
	APlanet* NearestPlanet = nullptr;
	float MinDistSq = MAX_FLT;

	for (TActorIterator<APlanet> It(GetWorld()); It; ++It)
	{
		float DistSq = FVector::DistSquared(GetActorLocation(), It->GetActorLocation());
		if (DistSq < MinDistSq)
		{
			MinDistSq = DistSq;
			NearestPlanet = *It;
		}
	}

	if (NearestPlanet && NearestPlanet->AtmosphereSphere)
	{
		float Dist = FMath::Sqrt(MinDistSq);
		float AtmosphereRadius = NearestPlanet->AtmosphereSphere->GetScaledSphereRadius();

		if (Dist < AtmosphereRadius)
		{
			OnEnterPlanetGravity(NearestPlanet);
		}
	}
}

void AShipPawn::OnInteractionZoneBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Reserved for future use (e.g., UI prompts when character enters interaction zone)
}
