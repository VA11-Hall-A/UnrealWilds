// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/UWCharacter.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "Character/UWCharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Gravity/GravityFloor.h"
#include "Gravity/GravityWorldSubsystem.h"
#include "Pawn/ThrusterComponent.h"
#include "Pawn/PlanetAttachmentComponent.h"
#include "Probe/ProbeLauncherComponent.h"
#include "Astro/Planet.h"
#include "Pawn/ShipPawn.h"
#include "EngineUtils.h"

namespace
{
FRotator GetGravityRelativeRotation(FRotator Rotation, const FVector& GravityDirection)
{
	if (!GravityDirection.Equals(FVector::DownVector))
	{
		const FQuat GravityRotation = FQuat::FindBetweenNormals(GravityDirection, FVector::DownVector);
		return (GravityRotation * Rotation.Quaternion()).Rotator();
	}

	return Rotation;
}

FRotator GetGravityWorldRotation(FRotator Rotation, const FVector& GravityDirection)
{
	if (!GravityDirection.Equals(FVector::DownVector))
	{
		const FQuat GravityRotation = FQuat::FindBetweenNormals(FVector::DownVector, GravityDirection);
		return (GravityRotation * Rotation.Quaternion()).Rotator();
	}

	return Rotation;
}

bool IsWalkableHit(const FHitResult& Hit, const FVector& GravityDirection, const UCharacterMovementComponent* CharacterMovement)
{
	if (!Hit.bBlockingHit || CharacterMovement == nullptr)
	{
		return false;
	}

	const FVector UpDirection = -GravityDirection.GetSafeNormal();
	return FVector::DotProduct(Hit.ImpactNormal, UpDirection) >= CharacterMovement->GetWalkableFloorZ();
}

FVector BuildSurfaceForward(const FVector& UpVector, const FVector& PreferredForward, const FVector& FallbackForward)
{
	FVector NewForward = FVector::VectorPlaneProject(PreferredForward, UpVector).GetSafeNormal();
	if (NewForward.IsNearlyZero())
	{
		NewForward = FVector::VectorPlaneProject(FallbackForward, UpVector).GetSafeNormal();
	}
	if (NewForward.IsNearlyZero())
	{
		NewForward = FVector::CrossProduct(UpVector, FVector::RightVector).GetSafeNormal();
		if (NewForward.IsNearlyZero())
		{
			NewForward = FVector::CrossProduct(UpVector, FVector::ForwardVector).GetSafeNormal();
		}
	}
	return NewForward;
}

double ComputePlanetGravityAcceleration(const APlanet* Planet, const FVector& ActorLocation)
{
	if (Planet == nullptr || !Planet->ApplyGravity)
	{
		return 0.0;
	}

	const FVector Direction = Planet->GetActorLocation() - ActorLocation;
	const double SquaredDistance = Direction.SquaredLength();
	const double HollowRadiusSq = FMath::Square(Planet->HollowRadius);
	if (SquaredDistance <= HollowRadiusSq)
	{
		return 0.0;
	}

	const double Distance = FMath::Max(FMath::Sqrt(SquaredDistance), UE_DOUBLE_SMALL_NUMBER);
	const double MassDotG = Planet->Mass * 6.67430E-5;
	const bool bIsInsidePlanet = Distance < Planet->PlanetRadius;

	double Intensity = 0.0;
	if (Planet->bUseInverseSquare)
	{
		if (bIsInsidePlanet)
		{
			const double HollowR3 = HollowRadiusSq * Planet->HollowRadius;
			const double PlanetR3 = FMath::Pow(Planet->PlanetRadius, 3.0);
			const double CurrentR3 = Distance * SquaredDistance;
			Intensity = (MassDotG / SquaredDistance) * ((CurrentR3 - HollowR3) / (PlanetR3 - HollowR3));
		}
		else
		{
			Intensity = MassDotG / SquaredDistance;
		}
	}
	else
	{
		if (bIsInsidePlanet)
		{
			const double SurfaceIntensity = MassDotG / Planet->PlanetRadius;
			Intensity = SurfaceIntensity * ((Distance - Planet->HollowRadius) / (Planet->PlanetRadius - Planet->HollowRadius));
		}
		else
		{
			Intensity = MassDotG / Distance;
		}
	}

	return FMath::Max(0.0, Intensity);
}

bool FindWalkableFloorUnderCharacter(const AUWCharacter* Character, const FVector& GravityDirection)
{
	if (Character == nullptr)
	{
		return false;
	}

	const UCapsuleComponent* Capsule = Character->GetCapsuleComponent();
	const UCharacterMovementComponent* CMC = Character->GetCharacterMovement();
	UWorld* World = Character->GetWorld();
	if (Capsule == nullptr || CMC == nullptr || World == nullptr)
	{
		return false;
	}

	const FVector Direction = GravityDirection.GetSafeNormal();
	if (Direction.IsNearlyZero())
	{
		return false;
	}

	const FVector Start = Character->GetActorLocation();
	const FVector End = Start + Direction * (CMC->MaxStepHeight + 5.0f);
	FCollisionShape SweepShape = FCollisionShape::MakeCapsule(Capsule->GetScaledCapsuleRadius(), Capsule->GetScaledCapsuleHalfHeight());
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(GravityFloorEntryFloorQuery), false, Character);

	FHitResult Hit;
	if (!World->SweepSingleByChannel(Hit, Start, End, Character->GetActorQuat(), ECC_Pawn, SweepShape, QueryParams))
	{
		return false;
	}

	return IsWalkableHit(Hit, Direction, CMC);
}
}

// Sets default values

inline AUWCharacter::AUWCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UUWCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(0.f, 0.f, 50.f + BaseEyeHeight));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	EnvironmentProbe = CreateDefaultSubobject<USphereComponent>(TEXT("EnvironmentProbe"));
	EnvironmentProbe->SetupAttachment(GetCapsuleComponent());
	EnvironmentProbe->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	EnvironmentProbe->SetCollisionObjectType(ECC_Pawn);
	EnvironmentProbe->SetCollisionResponseToAllChannels(ECR_Ignore);
	EnvironmentProbe->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	EnvironmentProbe->SetGenerateOverlapEvents(true);
	EnvironmentProbe->CanCharacterStepUpOn = ECB_No;
	RefreshEnvironmentProbe();

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
	RefreshEnvironmentProbe();

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

void AUWCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CurrentMovementState == ECharacterMovementState::TransitionToSurface)
	{
		TickTransitionToSurface(DeltaTime);
		return;
	}

	if (CurrentMovementState == ECharacterMovementState::TransitionToGravityFloor)
	{
		TickGravityFloorEntryTransition(DeltaTime);
	}
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
			EIC->BindAction(ToggleProbeAction, ETriggerEvent::Triggered, this, &AUWCharacter::OnToggleProbe);
		}
		if (RotateProbeCameraAction)
		{
			EIC->BindAction(RotateProbeCameraAction, ETriggerEvent::Triggered, this, &AUWCharacter::OnRotateProbeCamera);
		}
		if (CaptureProbePhotoAction)
		{
			EIC->BindAction(CaptureProbePhotoAction, ETriggerEvent::Triggered, this, &AUWCharacter::OnCaptureProbePhoto);
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
	if (IsInputLocked())
	{
		return;
	}

	FVector2D MovementVector = Value.Get<FVector2D>();

	FVector ForwardDirection = GetActorForwardVector();
	FVector RightDirection = GetActorRightVector();
	AddMovementInput(ForwardDirection, MovementVector.Y);
	AddMovementInput(RightDirection, MovementVector.X);
}

void AUWCharacter::Look(const FInputActionValue& Value)
{
	if (IsInputLocked())
	{
		return;
	}

	const FVector LookAxisVector = Value.Get<FVector>();

	if (CurrentMovementState == ECharacterMovementState::ZeroG)
	{
		UCapsuleComponent* Capsule = GetCapsuleComponent();
		if (Capsule)
		{
			FVector Torque = GetActorUpVector() * LookAxisVector.X * TorqueMultiplier
				+ GetActorRightVector() * LookAxisVector.Y * TorqueMultiplier;

			Capsule->AddTorqueInRadians(Torque, NAME_None, true);
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

void AUWCharacter::Jump()
{
	if (IsInputLocked())
	{
		return;
	}

	Super::Jump();
}

void AUWCharacter::FlyingMove(const FInputActionValue& Value)
{
	if (IsInputLocked())
	{
		return;
	}

	FVector MovemntVector = Value.Get<FVector>();
	if (Thruster)
	{
		Thruster->AddForceToMovemntComponent(MovemntVector);
	}
}

void AUWCharacter::Roll(const FInputActionValue& Value)
{
	if (IsInputLocked())
	{
		return;
	}

	float RollValue = Value.Get<float>();
	UCapsuleComponent* Capsule = GetCapsuleComponent();
	if (Capsule)
	{
		FVector Torque = GetActorForwardVector() * RollValue * TorqueMultiplier;
		Capsule->AddTorqueInRadians(Torque, NAME_None, true);
	}
}

void AUWCharacter::OnToggleProbe()
{
	if (IsInputLocked() || ProbeLauncher == nullptr)
	{
		return;
	}

	ProbeLauncher->ToggleProbe();
}

void AUWCharacter::OnRotateProbeCamera()
{
	if (IsInputLocked() || ProbeLauncher == nullptr)
	{
		return;
	}

	ProbeLauncher->RotateProbeCamera();
}

void AUWCharacter::OnCaptureProbePhoto()
{
	if (IsInputLocked() || ProbeLauncher == nullptr)
	{
		return;
	}

	ProbeLauncher->CaptureProbePhoto();
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
		InputSubsystem->RemoveMappingContext(FootMappingContext);
		InputSubsystem->AddMappingContext(ThrustMappingContext, 1);
		GetCharacterMovement()->Velocity += PlanetAttachment->GetOrbitalVelocity();
		return;
	}
	if (GetCharacterMovement()->MovementMode == MOVE_Walking)
	{
		InputSubsystem->RemoveMappingContext(ThrustMappingContext);
		InputSubsystem->RemoveMappingContext(RollMappingContext);
		InputSubsystem->AddMappingContext(FootMappingContext, 1);
	}
}

bool AUWCharacter::ResolveSurfaceGravity(FVector OverrideUpVector, double OverrideGravityAcceleration, FVector& OutUpVector, FVector& OutGravityDirection, double& OutGravityAcceleration) const
{
	if (!OverrideUpVector.IsNearlyZero())
	{
		OutUpVector = OverrideUpVector.GetSafeNormal();
		OutGravityDirection = -OutUpVector;
		OutGravityAcceleration = OverrideGravityAcceleration > 0.0 ? OverrideGravityAcceleration : TransitionGravityAcceleration;
		if (OutGravityAcceleration <= 0.0)
		{
			OutGravityAcceleration = 980.0;
		}
		return true;
	}

	if (APlanet* Planet = PlanetAttachment->GetCurrentPlanet())
	{
		OutUpVector = (GetActorLocation() - Planet->GetActorLocation()).GetSafeNormal();
		OutGravityDirection = -OutUpVector;
		OutGravityAcceleration = OverrideGravityAcceleration > 0.0
			? OverrideGravityAcceleration
			: ComputePlanetGravityAcceleration(Planet, GetActorLocation());
		return true;
	}

	return false;
}

void AUWCharacter::UpdateTransitionSurfaceGravity(FVector OverrideUpVector, double OverrideGravityAcceleration)
{
	if (CurrentMovementState != ECharacterMovementState::TransitionToSurface)
	{
		return;
	}

	FVector NewUpVector;
	FVector NewGravityDirection;
	double NewGravityAcceleration = 0.0;
	if (!ResolveSurfaceGravity(OverrideUpVector, OverrideGravityAcceleration, NewUpVector, NewGravityDirection, NewGravityAcceleration))
	{
		return;
	}

	TransitionGravityDirection = NewGravityDirection;
	TransitionGravityAcceleration = NewGravityAcceleration;

	const FRotator CameraRotation = FirstPersonCameraComponent->GetComponentRotation();
	const FVector NewForward = BuildSurfaceForward(NewUpVector, CameraRotation.Vector(), GetActorForwardVector());
	TransitionTargetActorQuat = FRotationMatrix::MakeFromXZ(NewForward, NewUpVector).ToQuat();

	FRotator GravityRelativeCamera = GetGravityRelativeRotation(CameraRotation, TransitionGravityDirection);
	GravityRelativeCamera.Roll = 0.0f;
	TransitionTargetCameraQuat = GetGravityWorldRotation(GravityRelativeCamera, TransitionGravityDirection).Quaternion();
}

void AUWCharacter::SnapToSurfaceGravity(FVector OverrideUpVector, double OverrideGravityAcceleration)
{
	FVector NewUpVector;
	FVector NewGravityDirection;
	double NewGravityAcceleration = 0.0;
	if (!ResolveSurfaceGravity(OverrideUpVector, OverrideGravityAcceleration, NewUpVector, NewGravityDirection, NewGravityAcceleration))
	{
		return;
	}

	CurrentMovementState = ECharacterMovementState::SurfaceGravity;

	UCapsuleComponent* Capsule = GetCapsuleComponent();
	UCharacterMovementComponent* CMC = GetCharacterMovement();

	const FVector CameraLocation = FirstPersonCameraComponent->GetComponentLocation();
	const FRotator CameraRotation = FirstPersonCameraComponent->GetComponentRotation();
	const FVector PhysicsVelocity = Capsule->GetComponentVelocity();

	Capsule->SetSimulatePhysics(false);
	CMC->SetGravityDirection(NewGravityDirection);
	CMC->SetMovementMode(MOVE_Falling);
	CMC->Velocity = PhysicsVelocity;
	Thruster->bIsCharacterMode = true;

	const FVector NewForward = BuildSurfaceForward(NewUpVector, CameraRotation.Vector(), GetActorForwardVector());
	const FRotator NewActorRotation = FRotationMatrix::MakeFromXZ(NewForward, NewUpVector).Rotator();

	const FVector LocalCameraOffset = FirstPersonCameraComponent->GetRelativeLocation();
	const FVector NewActorLocation = CameraLocation - NewActorRotation.RotateVector(LocalCameraOffset);
	SetActorLocationAndRotation(NewActorLocation, NewActorRotation, false, nullptr, ETeleportType::TeleportPhysics);

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->bAutoManageActiveCameraTarget = true;
		PC->SetControlRotation(CameraRotation);

		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->RemoveMappingContext(RollMappingContext);
		}
	}
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
}

void AUWCharacter::StartTransitionToSurface(FVector OverrideUpVector, double OverrideGravityAcceleration)
{
	if (bIsTransitioningState)
	{
		return;
	}

	bIsTransitioningState = true;

	UCapsuleComponent* Capsule = GetCapsuleComponent();
	const FVector InitialVelocity = Capsule->GetPhysicsLinearVelocity();

	Capsule->SetSimulatePhysics(false);
	Capsule->SetEnableGravity(false);
	Capsule->SetPhysicsAngularVelocityInRadians(FVector::ZeroVector);

	InitTransitionToSurface(InitialVelocity, OverrideUpVector, OverrideGravityAcceleration);
	bIsTransitioningState = false;
}

void AUWCharacter::StartTransitionBetweenSurfaces(FVector OverrideUpVector, double OverrideGravityAcceleration)
{
	if (bIsTransitioningState)
	{
		return;
	}

	bIsTransitioningState = true;

	UCharacterMovementComponent* CMC = GetCharacterMovement();
	const bool bWasOnGround = CMC->IsMovingOnGround();
	FVector InitialVelocity = CMC->Velocity;
	if (bWasOnGround && PlanetAttachment)
	{
		InitialVelocity += PlanetAttachment->GetOrbitalVelocity();
	}

	InitTransitionToSurface(InitialVelocity, OverrideUpVector, OverrideGravityAcceleration);
	bIsTransitioningState = false;
}

void AUWCharacter::InitTransitionToSurface(FVector InitialVelocity, FVector OverrideUpVector, double OverrideGravityAcceleration)
{
	CurrentMovementState = ECharacterMovementState::TransitionToSurface;
	TransitionElapsed = 0.0f;
	bTransitionPendingFloorHit = false;
	TransitionCameraLocalOffset = FirstPersonCameraComponent->GetRelativeLocation();
	TransitionVelocityWorld = InitialVelocity;
	TransitionStartActorQuat = GetActorQuat();
	TransitionStartCameraQuat = FirstPersonCameraComponent->GetComponentQuat();

	UpdateTransitionSurfaceGravity(OverrideUpVector, OverrideGravityAcceleration);

	GetCharacterMovement()->SetMovementMode(MOVE_None);
	GetCharacterMovement()->SetGravityDirection(TransitionGravityDirection);
	Thruster->bIsCharacterMode = false;

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->bAutoManageActiveCameraTarget = true;
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->RemoveMappingContext(RollMappingContext);
		}
	}

	FirstPersonCameraComponent->bUsePawnControlRotation = false;
	SetActorTickEnabled(true);
}

void AUWCharacter::TickTransitionToSurface(float DeltaTime)
{
	UCapsuleComponent* Capsule = GetCapsuleComponent();
	UCharacterMovementComponent* CMC = GetCharacterMovement();
	if (Capsule == nullptr || CMC == nullptr)
	{
		return;
	}

	CMC->SetGravityDirection(TransitionGravityDirection);

	const FVector StartActorLocation = GetActorLocation();
	const FVector StartCameraLocation = FirstPersonCameraComponent->GetComponentLocation();

	TransitionVelocityWorld += TransitionGravityDirection * static_cast<float>(TransitionGravityAcceleration) * DeltaTime;
	const FVector MoveDelta = TransitionVelocityWorld * DeltaTime;

	FHitResult MoveHit;
	SetActorLocation(StartActorLocation + MoveDelta, true, &MoveHit, ETeleportType::None);
	const FVector ActualMoveDelta = GetActorLocation() - StartActorLocation;

	if (MoveHit.bBlockingHit)
	{
		TransitionVelocityWorld = FVector::VectorPlaneProject(TransitionVelocityWorld, MoveHit.ImpactNormal);
		bTransitionPendingFloorHit = bTransitionPendingFloorHit || IsWalkableHit(MoveHit, TransitionGravityDirection, CMC);
	}

	TransitionElapsed += DeltaTime;
	const float Alpha = FMath::Clamp(TransitionElapsed / FMath::Max(TransitionDuration, KINDA_SMALL_NUMBER), 0.0f, 1.0f);
	const float EasedAlpha = FMath::InterpEaseInOut(0.0f, 1.0f, Alpha, 2.0f);

	const FQuat NewActorQuat = FQuat::Slerp(TransitionStartActorQuat, TransitionTargetActorQuat, EasedAlpha).GetNormalized();
	const FQuat NewCameraQuat = FQuat::Slerp(TransitionStartCameraQuat, TransitionTargetCameraQuat, EasedAlpha).GetNormalized();

	const FVector DesiredCameraLocation = StartCameraLocation + ActualMoveDelta;
	const FVector NewActorLocation = DesiredCameraLocation - NewActorQuat.RotateVector(TransitionCameraLocalOffset);

	SetActorLocationAndRotation(NewActorLocation, NewActorQuat.Rotator(), false, nullptr, ETeleportType::None);
	FirstPersonCameraComponent->SetWorldRotation(NewCameraQuat.Rotator());

	if (Alpha >= 1.0f)
	{
		FinishTransitionToSurface();
	}
}

void AUWCharacter::FinishTransitionToSurface()
{
	if (CurrentMovementState != ECharacterMovementState::TransitionToSurface)
	{
		return;
	}

	UCapsuleComponent* Capsule = GetCapsuleComponent();
	UCharacterMovementComponent* CMC = GetCharacterMovement();
	if (Capsule == nullptr || CMC == nullptr)
	{
		return;
	}

	const FVector CameraLocation = FirstPersonCameraComponent->GetComponentLocation();
	const FVector FinalActorLocation = CameraLocation - TransitionTargetActorQuat.RotateVector(TransitionCameraLocalOffset);
	SetActorLocationAndRotation(FinalActorLocation, TransitionTargetActorQuat.Rotator(), false, nullptr, ETeleportType::TeleportPhysics);
	FirstPersonCameraComponent->SetWorldRotation(TransitionTargetCameraQuat.Rotator());

	Capsule->SetSimulatePhysics(false);
	CMC->SetGravityDirection(TransitionGravityDirection);
	Thruster->bIsCharacterMode = true;

	const FVector OrbitalVelocity = PlanetAttachment ? PlanetAttachment->GetOrbitalVelocity() : FVector::ZeroVector;
	if (bTransitionPendingFloorHit)
	{
		CMC->Velocity = FVector::VectorPlaneProject(TransitionVelocityWorld - OrbitalVelocity, TransitionGravityDirection);
		CMC->SetMovementMode(MOVE_Walking);
	}
	else
	{
		CMC->Velocity = TransitionVelocityWorld;
		CMC->SetMovementMode(MOVE_Falling);
	}

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->bAutoManageActiveCameraTarget = true;
		PC->SetControlRotation(TransitionTargetCameraQuat.Rotator());
	}

	FirstPersonCameraComponent->bUsePawnControlRotation = true;
	CurrentMovementState = ECharacterMovementState::SurfaceGravity;
	TransitionElapsed = 0.0f;
	bTransitionPendingFloorHit = false;
	SetActorTickEnabled(false);
}

void AUWCharacter::AbortTransitionToSurface(FVector InheritedOrbitalVelocity)
{
	if (CurrentMovementState != ECharacterMovementState::TransitionToSurface)
	{
		return;
	}

	UCapsuleComponent* Capsule = GetCapsuleComponent();
	UCharacterMovementComponent* CMC = GetCharacterMovement();
	if (Capsule == nullptr || CMC == nullptr)
	{
		return;
	}

	SetActorTickEnabled(false);
	CurrentMovementState = ECharacterMovementState::ZeroG;
	TransitionElapsed = 0.0f;
	bTransitionPendingFloorHit = false;

	const FVector CameraLocation = FirstPersonCameraComponent->GetComponentLocation();
	const FRotator CameraRotation = FirstPersonCameraComponent->GetComponentRotation();
	const FVector LocalCameraOffset = FirstPersonCameraComponent->GetRelativeLocation();
	const FVector NewActorLocation = CameraLocation - CameraRotation.RotateVector(LocalCameraOffset);
	SetActorLocationAndRotation(NewActorLocation, CameraRotation, false, nullptr, ETeleportType::TeleportPhysics);

	CMC->SetMovementMode(MOVE_None);
	Thruster->bIsCharacterMode = false;

	Capsule->SetSimulatePhysics(true);
	Capsule->SetEnableGravity(false);
	Capsule->BodyInstance.bLockXRotation = false;
	Capsule->BodyInstance.bLockYRotation = false;
	Capsule->BodyInstance.bLockZRotation = false;
	Capsule->SetPhysicsLinearVelocity(InheritedOrbitalVelocity.IsNearlyZero() ? TransitionVelocityWorld : InheritedOrbitalVelocity);

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->bAutoManageActiveCameraTarget = true;
		PC->SetControlRotation(CameraRotation);

		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(RollMappingContext, 2);
		}
	}

	FirstPersonCameraComponent->bUsePawnControlRotation = false;
	FirstPersonCameraComponent->SetRelativeRotation(FRotator::ZeroRotator);
}

void AUWCharacter::StartGravityFloorEntryTransition(AGravityFloor* Floor)
{
	if (Floor == nullptr || bIsTransitioningState)
	{
		return;
	}

	if (CurrentMovementState == ECharacterMovementState::TransitionToGravityFloor)
	{
		if (GravityFloorTransitionFloor.Get() == Floor)
		{
			return;
		}

		AbortGravityFloorEntryTransition();
	}

	if (CurrentMovementState == ECharacterMovementState::TransitionToSurface)
	{
		AbortTransitionToSurface();
	}

	UCapsuleComponent* Capsule = GetCapsuleComponent();
	UCharacterMovementComponent* CMC = GetCharacterMovement();
	if (Capsule == nullptr || CMC == nullptr || FirstPersonCameraComponent == nullptr || EnvironmentProbe == nullptr)
	{
		return;
	}

	bIsTransitioningState = true;

	const FVector FloorUpVector = -Floor->GetGravityDirection();
	const FVector FloorGravityDirection = Floor->GetGravityDirection();
	const FRotator CameraRotation = FirstPersonCameraComponent->GetComponentRotation();
	const FVector NewForward = BuildSurfaceForward(FloorUpVector, CameraRotation.Vector(), GetActorForwardVector());

	GravityFloorTransitionFloor = Floor;
	GravityFloorTransitionElapsed = 0.0f;
	GravityFloorTransitionProbeLocalToFloor = Floor->GetActorTransform().InverseTransformPosition(GetEnvironmentProbeLocation());
	GravityFloorTransitionStartActorQuat = GetActorQuat();
	GravityFloorTransitionTargetActorQuat = FRotationMatrix::MakeFromXZ(NewForward, FloorUpVector).ToQuat();
	GravityFloorTransitionStartCameraQuat = FirstPersonCameraComponent->GetComponentQuat();

	FRotator GravityRelativeCamera = GetGravityRelativeRotation(CameraRotation, FloorGravityDirection);
	GravityRelativeCamera.Roll = 0.0f;
	GravityFloorTransitionTargetCameraQuat = GetGravityWorldRotation(GravityRelativeCamera, FloorGravityDirection).Quaternion();

	CMC->StopMovementImmediately();
	CMC->SetMovementMode(MOVE_None);
	CMC->SetGravityDirection(FloorGravityDirection);

	if (CurrentMovementState == ECharacterMovementState::ZeroG)
	{
		Capsule->SetSimulatePhysics(false);
		Capsule->SetEnableGravity(false);
		Capsule->SetPhysicsLinearVelocity(FVector::ZeroVector);
	}

	Capsule->SetPhysicsAngularVelocityInRadians(FVector::ZeroVector);
	Thruster->bIsCharacterMode = false;
	SetInputLocked(true);

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->bAutoManageActiveCameraTarget = true;
	}

	FirstPersonCameraComponent->bUsePawnControlRotation = false;
	CurrentMovementState = ECharacterMovementState::TransitionToGravityFloor;
	SetActorTickEnabled(true);
	bIsTransitioningState = false;
}

void AUWCharacter::TickGravityFloorEntryTransition(float DeltaTime)
{
	if (CurrentMovementState != ECharacterMovementState::TransitionToGravityFloor)
	{
		return;
	}

	AGravityFloor* Floor = GravityFloorTransitionFloor.Get();
	if (Floor == nullptr || FirstPersonCameraComponent == nullptr || EnvironmentProbe == nullptr)
	{
		AbortGravityFloorEntryTransition();
		return;
	}

	GravityFloorTransitionElapsed += DeltaTime;
	const float Alpha = FMath::Clamp(GravityFloorTransitionElapsed / FMath::Max(TransitionDuration, KINDA_SMALL_NUMBER), 0.0f, 1.0f);
	const float EasedAlpha = FMath::InterpEaseInOut(0.0f, 1.0f, Alpha, 2.0f);

	const FQuat NewActorQuat = FQuat::Slerp(GravityFloorTransitionStartActorQuat, GravityFloorTransitionTargetActorQuat, EasedAlpha).GetNormalized();
	const FQuat NewCameraQuat = FQuat::Slerp(GravityFloorTransitionStartCameraQuat, GravityFloorTransitionTargetCameraQuat, EasedAlpha).GetNormalized();
	const FVector ProbeWorldLocation = Floor->GetActorTransform().TransformPosition(GravityFloorTransitionProbeLocalToFloor);
	const FVector NewActorLocation = ProbeWorldLocation - NewActorQuat.RotateVector(EnvironmentProbe->GetRelativeLocation());

	SetActorLocationAndRotation(NewActorLocation, NewActorQuat.Rotator(), false, nullptr, ETeleportType::TeleportPhysics);
	FirstPersonCameraComponent->SetWorldRotation(NewCameraQuat.Rotator());

	if (Alpha >= 1.0f)
	{
		FinishGravityFloorEntryTransition();
	}
}

void AUWCharacter::FinishGravityFloorEntryTransition()
{
	if (CurrentMovementState != ECharacterMovementState::TransitionToGravityFloor)
	{
		return;
	}

	AGravityFloor* Floor = GravityFloorTransitionFloor.Get();
	UCapsuleComponent* Capsule = GetCapsuleComponent();
	UCharacterMovementComponent* CMC = GetCharacterMovement();
	if (Floor == nullptr || Capsule == nullptr || CMC == nullptr || FirstPersonCameraComponent == nullptr || EnvironmentProbe == nullptr)
	{
		AbortGravityFloorEntryTransition();
		return;
	}

	const FVector ProbeWorldLocation = Floor->GetActorTransform().TransformPosition(GravityFloorTransitionProbeLocalToFloor);
	const FVector FinalActorLocation = ProbeWorldLocation - GravityFloorTransitionTargetActorQuat.RotateVector(EnvironmentProbe->GetRelativeLocation());
	SetActorLocationAndRotation(FinalActorLocation, GravityFloorTransitionTargetActorQuat.Rotator(), false, nullptr, ETeleportType::TeleportPhysics);
	FirstPersonCameraComponent->SetWorldRotation(GravityFloorTransitionTargetCameraQuat.Rotator());

	Capsule->SetSimulatePhysics(false);
	Capsule->SetEnableGravity(false);
	CMC->SetGravityDirection(Floor->GetGravityDirection());
	CMC->Velocity = FVector::ZeroVector;
	CMC->SetMovementMode(FindWalkableFloorUnderCharacter(this, Floor->GetGravityDirection()) ? MOVE_Walking : MOVE_Falling);
	Thruster->bIsCharacterMode = true;

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->bAutoManageActiveCameraTarget = true;
		PC->SetControlRotation(GravityFloorTransitionTargetCameraQuat.Rotator());
	}

	if (FirstPersonCameraComponent != nullptr)
	{
		FirstPersonCameraComponent->bUsePawnControlRotation = true;
	}
	GravityFloorTransitionFloor.Reset();
	GravityFloorTransitionElapsed = 0.0f;
	CurrentMovementState = ECharacterMovementState::SurfaceGravity;
	SetInputLocked(false);
	SetActorTickEnabled(false);
}

void AUWCharacter::AbortGravityFloorEntryTransition()
{
	if (CurrentMovementState != ECharacterMovementState::TransitionToGravityFloor)
	{
		return;
	}

	UCapsuleComponent* Capsule = GetCapsuleComponent();
	UCharacterMovementComponent* CMC = GetCharacterMovement();
	if (Capsule != nullptr)
	{
		Capsule->SetSimulatePhysics(false);
		Capsule->SetEnableGravity(false);
	}

	if (CMC != nullptr)
	{
		CMC->StopMovementImmediately();
		if (AGravityFloor* Floor = GravityFloorTransitionFloor.Get())
		{
			CMC->SetGravityDirection(Floor->GetGravityDirection());
		}
		CMC->SetMovementMode(MOVE_Falling);
	}

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->bAutoManageActiveCameraTarget = true;
		if (FirstPersonCameraComponent != nullptr)
		{
			PC->SetControlRotation(FirstPersonCameraComponent->GetComponentRotation());
		}
	}

	if (FirstPersonCameraComponent != nullptr)
	{
		FirstPersonCameraComponent->bUsePawnControlRotation = true;
	}
	Thruster->bIsCharacterMode = true;
	GravityFloorTransitionFloor.Reset();
	GravityFloorTransitionElapsed = 0.0f;
	CurrentMovementState = ECharacterMovementState::SurfaceGravity;
	SetInputLocked(false);
	SetActorTickEnabled(false);
}

void AUWCharacter::EnterSurfaceGravity(FVector OverrideUpVector, double OverrideGravityAcceleration)
{
	if (bIsTransitioningState)
	{
		return;
	}

	if (CurrentMovementState == ECharacterMovementState::TransitionToGravityFloor)
	{
		return;
	}

	if (CurrentMovementState == ECharacterMovementState::TransitionToSurface)
	{
		UpdateTransitionSurfaceGravity(OverrideUpVector, OverrideGravityAcceleration);
		return;
	}

	if (CurrentMovementState == ECharacterMovementState::ZeroG)
	{
		StartTransitionToSurface(OverrideUpVector, OverrideGravityAcceleration);
		return;
	}

	// SurfaceGravity: check if gravity direction changes significantly
	FVector NewUpVector;
	FVector NewGravityDirection;
	double NewGravityAcceleration = 0.0;
	if (ResolveSurfaceGravity(OverrideUpVector, OverrideGravityAcceleration, NewUpVector, NewGravityDirection, NewGravityAcceleration))
	{
		const FVector CurrentGravityDir = GetCharacterMovement()->GetGravityDirection();
		const double Dot = FVector::DotProduct(CurrentGravityDir, NewGravityDirection);
		if (Dot < 0.999)
		{
			StartTransitionBetweenSurfaces(OverrideUpVector, OverrideGravityAcceleration);
			return;
		}
	}

	SnapToSurfaceGravity(OverrideUpVector, OverrideGravityAcceleration);
}

void AUWCharacter::EnterZeroG(FVector InheritedOrbitalVelocity)
{
	if (CurrentMovementState == ECharacterMovementState::TransitionToGravityFloor)
	{
		AbortGravityFloorEntryTransition();
	}

	if (CurrentMovementState == ECharacterMovementState::TransitionToSurface)
	{
		AbortTransitionToSurface(InheritedOrbitalVelocity);
		return;
	}

	if (bIsTransitioningState)
	{
		return;
	}

	if (CurrentMovementState == ECharacterMovementState::ZeroG)
	{
		return;
	}

	bIsTransitioningState = true;

	CurrentMovementState = ECharacterMovementState::ZeroG;

	UCapsuleComponent* Capsule = GetCapsuleComponent();
	UCharacterMovementComponent* CMC = GetCharacterMovement();

	const bool bWasOnGround = CMC->IsMovingOnGround();
	const FVector CMCVelocity = CMC->Velocity;

	const FVector CameraLocation = FirstPersonCameraComponent->GetComponentLocation();
	const FRotator CameraRotation = FirstPersonCameraComponent->GetComponentRotation();

	CMC->SetMovementMode(MOVE_None);
	Thruster->bIsCharacterMode = false;

	const FVector LocalCameraOffset = FirstPersonCameraComponent->GetRelativeLocation();
	const FVector NewActorLocation = CameraLocation - CameraRotation.RotateVector(LocalCameraOffset);
	SetActorLocationAndRotation(NewActorLocation, CameraRotation, false, nullptr, ETeleportType::TeleportPhysics);

	Capsule->SetSimulatePhysics(true);
	Capsule->SetEnableGravity(false);
	Capsule->BodyInstance.bLockXRotation = false;
	Capsule->BodyInstance.bLockYRotation = false;
	Capsule->BodyInstance.bLockZRotation = false;

	const FVector TotalVelocity = CMCVelocity + (bWasOnGround ? InheritedOrbitalVelocity : FVector::ZeroVector);
	Capsule->SetPhysicsLinearVelocity(TotalVelocity);

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->bAutoManageActiveCameraTarget = true;
		PC->SetControlRotation(CameraRotation);

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

UPrimitiveComponent* AUWCharacter::GetEnvironmentProbeComponent() const
{
	return EnvironmentProbe;
}

FVector AUWCharacter::GetEnvironmentProbeLocation() const
{
	return EnvironmentProbe ? EnvironmentProbe->GetComponentLocation() : GetActorLocation();
}

bool AUWCharacter::IsEnvironmentProbe(const UPrimitiveComponent* Component) const
{
	return Component != nullptr && Component == EnvironmentProbe;
}

bool AUWCharacter::IsTransitioningToGravityFloor(const AGravityFloor* Floor) const
{
	return CurrentMovementState == ECharacterMovementState::TransitionToGravityFloor
		&& GravityFloorTransitionFloor.Get() == Floor;
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

	if (CurrentMovementState == ECharacterMovementState::TransitionToSurface)
	{
		return TransitionVelocityWorld;
	}

	if (CurrentMovementState == ECharacterMovementState::TransitionToGravityFloor)
	{
		return FVector::ZeroVector;
	}

	if (CurrentMovementState == ECharacterMovementState::SurfaceGravity)
	{
		const FVector CMCVelocity = GetCharacterMovement()->Velocity;
		if (GetCharacterMovement()->IsMovingOnGround())
		{
			return CMCVelocity + OrbitalVelocity;
		}
		return CMCVelocity;
	}

	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		return Capsule->GetPhysicsLinearVelocity() + OrbitalVelocity;
	}

	return Super::GetVelocity();
}

void AUWCharacter::CheckInitialMovementState()
{
	PlanetAttachment->CheckInitialPlanetState();

	APlanet* Planet = PlanetAttachment->GetCurrentPlanet();
	if (Planet)
	{
		if (Planet->HollowInnerSphere)
		{
			const float Dist = FVector::Dist(GetEnvironmentProbeLocation(), Planet->GetActorLocation());
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
	}
	else
	{
		EnterZeroG();
	}
}

void AUWCharacter::OnInteract()
{
	if (IsInputLocked())
	{
		return;
	}

	TArray<AActor*> OverlappingActors;
	if (EnvironmentProbe)
	{
		EnvironmentProbe->GetOverlappingActors(OverlappingActors, AShipPawn::StaticClass());
	}

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

void AUWCharacter::SetInputLocked(bool bLocked)
{
	bInputLocked = bLocked;

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->SetIgnoreMoveInput(bLocked);
		PC->SetIgnoreLookInput(bLocked);
	}
}

bool AUWCharacter::IsInputLocked() const
{
	return bInputLocked
		|| CurrentMovementState == ECharacterMovementState::TransitionToSurface
		|| CurrentMovementState == ECharacterMovementState::TransitionToGravityFloor;
}

void AUWCharacter::RefreshEnvironmentProbe()
{
	if (!EnvironmentProbe)
	{
		return;
	}

	EnvironmentProbe->SetSphereRadius(EnvironmentProbeRadius);
	EnvironmentProbe->SetRelativeLocation(EnvironmentProbeOffset);
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
	if (CurrentMovementState == ECharacterMovementState::TransitionToSurface)
	{
		AbortTransitionToSurface();
		return;
	}
	EnterZeroG(OrbitalVelocity);
}
