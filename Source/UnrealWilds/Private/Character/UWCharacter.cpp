// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/UWCharacter.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "Character/UWCharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Controller.h"
#include "InputActionValue.h"
#include "Engine/Engine.h"
#include "Gravity/GravityWorldSubsystem.h"
#include "Pawn/ThrusterComponent.h"
#include "Pawn/PlanetAttachmentComponent.h"
#include "Probe/ProbeLauncherComponent.h"
#include "Astro/Planet.h"
#include "Pawn/ShipPawn.h"
#include "EngineUtils.h"

// Sets default values

inline AUWCharacter::AUWCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UUWCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(0.f, 0.f, 50.f + BaseEyeHeight));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	Thruster = CreateDefaultSubobject<UThrusterComponent>(TEXT("Thruster"));
	Thruster->ThrustForce = 1000.0;
	Thruster->bIsCharacterMode = true;

	ProbeLauncher = CreateDefaultSubobject<UProbeLauncherComponent>(TEXT("ProbeLauncher"));

	PlanetAttachment = CreateDefaultSubobject<UPlanetAttachmentComponent>(TEXT("PlanetAttachment"));
	PlanetAttachment->Initialize(GetCapsuleComponent());
}

// Called when the game starts or when spawned
void AUWCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Add the common and foot mapping contexts (initial state: surface walking)
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (CommonMappingContext)
			{
				Subsystem->AddMappingContext(CommonMappingContext, 0);
			}
			if (FootMappingContext)
			{
				Subsystem->AddMappingContext(FootMappingContext, 1);
			}
		}
	}
	if (UWorld* World = GetWorld())
	{
		if (UGravityWorldSubsystem* GravitySubsystem = World->GetSubsystem<UGravityWorldSubsystem>())
		{
			GravitySubsystem->RegisterPlayerCharacter(GetCharacterMovement());
			UE_LOG(LogTemp, Warning, TEXT("Character Registered to Subsystem!"));
		}
	}

	// Bind planet attachment delegates
	PlanetAttachment->OnAttachedToPlanet.AddDynamic(this, &AUWCharacter::OnAttachedToPlanet);
	PlanetAttachment->OnDetachedFromPlanet.AddDynamic(this, &AUWCharacter::OnDetachedFromPlanet);

	CheckInitialMovementState();
}

// Called to bind functionality to input
void AUWCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
		{
			EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AUWCharacter::Move);
		}
		if (LookAction)
		{
			EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &AUWCharacter::Look);
		}
		if (JumpAction)
		{
			EIC->BindAction(JumpAction, ETriggerEvent::Triggered, this, &AUWCharacter::Jump);
		}
		if (FlyingMoveAction)
		{
			EIC->BindAction(FlyingMoveAction, ETriggerEvent::Triggered, this, &AUWCharacter::FlyingMove);
			EIC->BindAction(FlyingMoveAction, ETriggerEvent::Completed, this, &AUWCharacter::FlyingMove);
		}
		if (RollAction)
		{
			EIC->BindAction(RollAction, ETriggerEvent::Triggered, this, &AUWCharacter::Roll);
		}
		if (ToggleProbeAction)
		{
			EIC->BindAction(ToggleProbeAction, ETriggerEvent::Triggered, ProbeLauncher, &UProbeLauncherComponent::ToggleProbe);
		}
		if (RotateProbeCameraAction)
		{
			EIC->BindAction(RotateProbeCameraAction, ETriggerEvent::Triggered, ProbeLauncher, &UProbeLauncherComponent::RotateProbeCamera);
		}
		if (CaptureProbePhotoAction)
		{
			EIC->BindAction(CaptureProbePhotoAction, ETriggerEvent::Triggered, ProbeLauncher, &UProbeLauncherComponent::CaptureProbePhoto);
		}
		if (InteractAction)
		{
			EIC->BindAction(InteractAction, ETriggerEvent::Triggered, this, &AUWCharacter::OnInteract);
		}
	}
}

// ── Input callbacks ──────────────────────────────────────────────────────────

void AUWCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	FVector ForwardDirection = GetActorForwardVector();
	FVector RightDirection = GetActorRightVector();
	// 6. 根据输入向量沿计算出的两个贴地平面方向移动
	AddMovementInput(ForwardDirection, MovementVector.Y);
	AddMovementInput(RightDirection, MovementVector.X);
}

void AUWCharacter::Look(const FInputActionValue& Value)
{
	const FVector LookAxisVector = Value.Get<FVector>();

	if (CurrentMovementState == ECharacterMovementState::ZeroG)
	{
		UCapsuleComponent* Capsule = GetCapsuleComponent();
		if (Capsule)
		{
			 // Torque multiplier for AccelChange
			// Yaw rotates around UpVector, Pitch rotates around RightVector
			FVector Torque = GetActorUpVector() * LookAxisVector.X * TorqueMultiplier
				+ GetActorRightVector() * LookAxisVector.Y * TorqueMultiplier; // Inverted Y for natural pitch

			Capsule->AddTorqueInRadians(Torque, NAME_None, true); // true = bAccelChange (ignores mass)
		}
	}
	else
	{
		if (Controller)
		{
			AddControllerYawInput(LookAxisVector.X);
			AddControllerPitchInput(LookAxisVector.Y);
		}
	}
}

void AUWCharacter::FlyingMove(const FInputActionValue& Value)
{
	FVector MovemntVector = Value.Get<FVector>();
	if (Thruster)
	{
		Thruster->AddForceToMovemntComponent(MovemntVector);
	}
}

void AUWCharacter::Roll(const FInputActionValue& Value)
{
	float RollValue = Value.Get<float>();
	UCapsuleComponent* Capsule = GetCapsuleComponent();
	if (Capsule)
	{
		FVector Torque = GetActorForwardVector() * RollValue * TorqueMultiplier;
		Capsule->AddTorqueInRadians(Torque, NAME_None, true);
	}
}

void AUWCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);
	APlayerController* PC = Cast<APlayerController>(GetController());

	if (PC == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerController or InputMappingContext is null!"));
		return;
	}
	ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
	if (!LocalPlayer)
	{
		UE_LOG(LogTemp, Warning, TEXT("LocalPlayer is null!"));
		return;
	}
	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (!InputSubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("EnhancedInputLocalPlayerSubsystem is null!"));
		return;
	}

	if (PrevMovementMode == MOVE_Walking)
	{
		// Left ground: swap Foot → Thrust
		InputSubsystem->RemoveMappingContext(FootMappingContext);
		InputSubsystem->AddMappingContext(ThrustMappingContext, 1);
		GetCharacterMovement()->Velocity += PlanetAttachment->GetOrbitalVelocity();
		return;
	}
	if (GetCharacterMovement()->MovementMode == MOVE_Walking)
	{
		// Landed: swap Thrust → Foot, also remove Roll in case we came from ZeroG
		InputSubsystem->RemoveMappingContext(ThrustMappingContext);
		InputSubsystem->RemoveMappingContext(RollMappingContext);
		InputSubsystem->AddMappingContext(FootMappingContext, 1);
	}
}

void AUWCharacter::EnterSurfaceGravity(FVector OverrideUpVector)
{
	if (bIsTransitioningState) return;
	bIsTransitioningState = true;

	CurrentMovementState = ECharacterMovementState::SurfaceGravity;

	UCapsuleComponent* Capsule = GetCapsuleComponent();
	UCharacterMovementComponent* CMC = GetCharacterMovement();

	// Record camera world transform before state change
	FVector CameraLocation = FirstPersonCameraComponent->GetComponentLocation();
	FRotator CameraRotation = FirstPersonCameraComponent->GetComponentRotation();

	// Read velocity from physics before turning it off
	FVector PhysicsVelocity = Capsule->GetComponentVelocity();

	Capsule->SetSimulatePhysics(false);

	CMC->SetMovementMode(MOVE_Falling);
	CMC->Velocity = PhysicsVelocity;
	Thruster->bIsCharacterMode = true;

	// Calculate new upright body rotation based on gravity direction and camera yaw
	FVector NewUpVector;
	if (!OverrideUpVector.IsNearlyZero())
	{
		NewUpVector = OverrideUpVector;
	}
	else
	{
		NewUpVector = GetActorUpVector();
		if (APlanet* Planet = PlanetAttachment->GetCurrentPlanet())
		{
			NewUpVector = (GetActorLocation() - Planet->GetActorLocation()).GetSafeNormal();
		}
	}
	
	FVector CameraForward = CameraRotation.Vector();
	FVector NewRight = FVector::CrossProduct(NewUpVector, CameraForward).GetSafeNormal();
	if (NewRight.IsNearlyZero()) // Fallback if looking straight up/down
	{
		NewRight = FVector::CrossProduct(NewUpVector, GetActorForwardVector()).GetSafeNormal();
	}
	FVector NewForward = FVector::CrossProduct(NewRight, NewUpVector).GetSafeNormal();
	FRotator NewActorRotation = FRotationMatrix::MakeFromXZ(NewForward, NewUpVector).Rotator();

	// Offset actor location so the camera world position remains exactly the same
	FVector LocalCameraOffset = FirstPersonCameraComponent->GetRelativeLocation();
	FVector NewActorLocation = CameraLocation - NewActorRotation.RotateVector(LocalCameraOffset);
	SetActorLocationAndRotation(NewActorLocation, NewActorRotation, false, nullptr, ETeleportType::TeleportPhysics);

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->bAutoManageActiveCameraTarget = true;
		PC->SetControlRotation(CameraRotation);

		// Remove roll input when entering surface gravity
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->RemoveMappingContext(RollMappingContext);
		}
	}
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	bIsTransitioningState = false;
}

void AUWCharacter::EnterZeroG(FVector InheritedOrbitalVelocity)
{
	if (bIsTransitioningState) return;

	// Already in ZeroG (e.g. hollow sphere → leave atmosphere):
	// DetachFromPlanet already set the correct physics velocity, don't overwrite it.
	if (CurrentMovementState == ECharacterMovementState::ZeroG)
	{
		return;
	}

	bIsTransitioningState = true;

	CurrentMovementState = ECharacterMovementState::ZeroG;

	UCapsuleComponent* Capsule = GetCapsuleComponent();
	UCharacterMovementComponent* CMC = GetCharacterMovement();

	// Capture whether CMC velocity is planet-relative (on ground) or
	// world-absolute (falling after jump, where OnMovementModeChanged already baked in orbital).
	const bool bWasOnGround = CMC->IsMovingOnGround();
	FVector CMCVelocity = CMC->Velocity;

	// Record camera world transform before state change
	FVector CameraLocation = FirstPersonCameraComponent->GetComponentLocation();
	FRotator CameraRotation = FirstPersonCameraComponent->GetComponentRotation();

	CMC->SetMovementMode(MOVE_None);
	Thruster->bIsCharacterMode = false;

	// Align character body to camera's facing direction, and offset position
	// so the camera stays in exactly the same world location.
	FVector LocalCameraOffset = FirstPersonCameraComponent->GetRelativeLocation();
	FVector NewActorLocation = CameraLocation - CameraRotation.RotateVector(LocalCameraOffset);
	SetActorLocationAndRotation(NewActorLocation, CameraRotation, false, nullptr, ETeleportType::TeleportPhysics);

	Capsule->SetSimulatePhysics(true);
	Capsule->SetEnableGravity(false); // We handle custom forces if needed
	Capsule->BodyInstance.bLockXRotation = false;
	Capsule->BodyInstance.bLockYRotation = false;
	Capsule->BodyInstance.bLockZRotation = false;

	// Only add InheritedOrbitalVelocity when CMC was planet-relative (on ground).
	// If the character was falling (after jump), CMC already contains orbital velocity.
	FVector TotalVelocity = CMCVelocity + (bWasOnGround ? InheritedOrbitalVelocity : FVector::ZeroVector);
	Capsule->SetPhysicsLinearVelocity(TotalVelocity);

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->bAutoManageActiveCameraTarget = true;
		PC->SetControlRotation(CameraRotation);

		// Enable roll input in zero-g
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(RollMappingContext, 2);
		}
	}

	FirstPersonCameraComponent->bUsePawnControlRotation = false;
	FirstPersonCameraComponent->SetRelativeRotation(FRotator::ZeroRotator);

	bIsTransitioningState = false;
}

ECharacterMovementState AUWCharacter::GetCurrentMovementState() const
{
	return CurrentMovementState;
}

bool AUWCharacter::IsOnGravityFloor() const
{
	if (UWorld* World = GetWorld())
	{
		if (UGravityWorldSubsystem* Sub = World->GetSubsystem<UGravityWorldSubsystem>())
		{
			return Sub->IsGravityFloorActive();
		}
	}
	return false;
}

FVector AUWCharacter::GetVelocity() const
{
	FVector OrbitalVelocity = PlanetAttachment->GetOrbitalVelocity();

	if (CurrentMovementState == ECharacterMovementState::SurfaceGravity)
	{
		const FVector CMCVelocity = GetCharacterMovement()->Velocity;
		// On ground: CMC velocity is planet-relative, need to add orbital for world-space result.
		// In air (after jump): CMC velocity is already world-absolute (orbital baked in by OnMovementModeChanged).
		if (GetCharacterMovement()->IsMovingOnGround())
		{
			return CMCVelocity + OrbitalVelocity;
		}
		return CMCVelocity;
	}

	// ZeroG: use physics velocity from capsule
	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		return Capsule->GetPhysicsLinearVelocity() + OrbitalVelocity;
	}

	return Super::GetVelocity();
}

void AUWCharacter::CheckInitialMovementState()
{
	// Let the component find and attach to the nearest planet (may trigger OnAttachedToPlanet → EnterSurfaceGravity)
	PlanetAttachment->CheckInitialPlanetState();

	APlanet* Planet = PlanetAttachment->GetCurrentPlanet();
	if (Planet)
	{
		// Check if inside hollow sphere → override to ZeroG
		if (Planet->HollowInnerSphere)
		{
			float Dist = FVector::Dist(GetActorLocation(), Planet->GetActorLocation());
			float HollowRadius = Planet->HollowInnerSphere->GetScaledSphereRadius();
			if (Dist < HollowRadius)
			{
				EnterZeroG();
				return;
			}
			else
			{
				EnterSurfaceGravity();
			}
		}
		// Otherwise the delegate already set SurfaceGravity, nothing more to do
	}
	else
	{
		// Not near any planet → ZeroG in deep space
		EnterZeroG();
	}
}

void AUWCharacter::OnInteract()
{
	TArray<AActor*> OverlappingActors;
	GetOverlappingActors(OverlappingActors, AShipPawn::StaticClass());

	for (AActor* Actor : OverlappingActors)
	{
		if (AShipPawn* Ship = Cast<AShipPawn>(Actor))
		{
			if (!Ship->IsOccupied())
			{
				Ship->BoardShip(this);
				return;
			}
		}
	}
}

void AUWCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (APlayerController* PC = Cast<APlayerController>(NewController))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (CommonMappingContext)
			{
				Subsystem->AddMappingContext(CommonMappingContext, 0);
			}
			if (FootMappingContext)
			{
				Subsystem->AddMappingContext(FootMappingContext, 1);
			}
		}
	}
}

// ── Planet Attachment Delegates ─────────────────────────────────────────────

void AUWCharacter::OnAttachedToPlanet(APlanet* Planet)
{
	if (!IsLocallyControlled())
	{
		return;
	}
	if (IsOnGravityFloor())
	{
		return;
	}
	EnterSurfaceGravity();
}

void AUWCharacter::OnDetachedFromPlanet(FVector OrbitalVelocity)
{
	if (!IsLocallyControlled())
	{
		return;
	}
	if (IsOnGravityFloor())
	{
		return;
	}
	EnterZeroG(OrbitalVelocity);
}
