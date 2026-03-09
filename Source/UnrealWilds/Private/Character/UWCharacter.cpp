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
#include "Probe/ProbeLauncherComponent.h"
#include "Astro/Planet.h"
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
}

// Called when the game starts or when spawned
void AUWCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Add the default mapping context to the local player's Enhanced Input subsystem
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
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
		}
		if (LaunchProbeAction)
		{
			EIC->BindAction(LaunchProbeAction, ETriggerEvent::Started, ProbeLauncher, &UProbeLauncherComponent::LaunchProbe);
		}
		if (RecallProbeAction)
		{
			EIC->BindAction(RecallProbeAction, ETriggerEvent::Started, ProbeLauncher, &UProbeLauncherComponent::RecallProbe);
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
			float TorqueMultiplier = 15.0f; // Torque multiplier for AccelChange
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
	FVector FinalVector = GetActorForwardVector() * MovemntVector.Y + GetActorRightVector() * MovemntVector.X + GetActorUpVector() * MovemntVector.Z;
	if (Thruster)
	{
		Thruster->AddForceToMovemntComponent(FinalVector);
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
		InputSubsystem->AddMappingContext(FlyingMappingContext, 1);
		return;
	}
	if (GetCharacterMovement()->MovementMode == MOVE_Walking)
	{
		InputSubsystem->RemoveMappingContext(FlyingMappingContext);
	}
}

void AUWCharacter::EnterSurfaceGravity(APlanet* Planet)
{
	CurrentMovementState = ECharacterMovementState::SurfaceGravity;

	if (Planet)
	{
		CurrentPlanet = Planet;
		AttachToActor(CurrentPlanet, FAttachmentTransformRules::KeepWorldTransform);
	}

	UCapsuleComponent* Capsule = GetCapsuleComponent();
	UCharacterMovementComponent* CMC = GetCharacterMovement();

	// Read velocity from physics before turning it off
	FVector PhysicsVelocity = Capsule->GetComponentVelocity();

	Capsule->SetSimulatePhysics(false);

	CMC->SetMovementMode(MOVE_Falling);
	CMC->Velocity = PhysicsVelocity;
	Thruster->bIsCharacterMode = true;

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->bAutoManageActiveCameraTarget = true;
	}
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Instant rotation correction to align Z-up with planet normal
	// APlanet* NearestPlanet = nullptr;
	// float MinDistSq = MAX_FLT;
	// for (TActorIterator<APlanet> It(GetWorld()); It; ++It)
	// {
	// 	float DistSq = GetSquaredDistanceTo(*It);
	// 	if (DistSq < MinDistSq)
	// 	{
	// 		MinDistSq = DistSq;
	// 		NearestPlanet = *It;
	// 	}
	// }

	// if (NearestPlanet)
	// {
	// 	FVector UpDirection = (GetActorLocation() - NearestPlanet->GetActorLocation()).GetSafeNormal();
	// 	FVector ForwardDirection = GetActorForwardVector();
	// 	FVector RightDirection = FVector::CrossProduct(UpDirection, ForwardDirection).GetSafeNormal();
	// 	ForwardDirection = FVector::CrossProduct(RightDirection, UpDirection).GetSafeNormal();

	// 	FRotator TargetRotation = FRotationMatrix::MakeFromXZ(ForwardDirection, UpDirection).Rotator();
	// 	SetActorRotation(TargetRotation);

	// 	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	// 	{
	// 		PC->SetControlRotation(TargetRotation);
	// 	}
	// }
}

void AUWCharacter::EnterZeroG(APlanet* Planet)
{
	CurrentMovementState = ECharacterMovementState::ZeroG;

	// Store the current planet before we potentially clear it
	APlanet* PreviousPlanet = CurrentPlanet;

	if (Planet)
	{
		CurrentPlanet = Planet;
		AttachToActor(CurrentPlanet, FAttachmentTransformRules::KeepWorldTransform);
	}
	else
	{
		CurrentPlanet = nullptr;
		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	}

	UCapsuleComponent* Capsule = GetCapsuleComponent();
	UCharacterMovementComponent* CMC = GetCharacterMovement();

	FVector CMCVelocity = CMC->Velocity;

	CMC->SetMovementMode(MOVE_None);
	Thruster->bIsCharacterMode = false;

	Capsule->SetSimulatePhysics(true);
	Capsule->SetEnableGravity(false); // We handle custom forces if needed
	Capsule->BodyInstance.bLockXRotation = false;
	Capsule->BodyInstance.bLockYRotation = false;
	Capsule->BodyInstance.bLockZRotation = false;

	FVector TotalVelocity = CMCVelocity;
	if (!Planet && PreviousPlanet) 
	{
		// We just detached from a planet into deep space, keep orbital momentum!
		TotalVelocity += PreviousPlanet->GetOrbitalVelocity();
	}
	Capsule->SetPhysicsLinearVelocity(TotalVelocity);

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->bAutoManageActiveCameraTarget = true;
	}

	FirstPersonCameraComponent->bUsePawnControlRotation = false;
	FirstPersonCameraComponent->SetRelativeRotation(FRotator::ZeroRotator);
}

ECharacterMovementState AUWCharacter::GetCurrentMovementState() const
{
	return CurrentMovementState;
}

void AUWCharacter::CheckInitialMovementState()
{
	APlanet* NearestPlanet = nullptr;
	float MinDistSq = MAX_FLT;
	for (TActorIterator<APlanet> It(GetWorld()); It; ++It)
	{
		float DistSq = GetSquaredDistanceTo(*It);
		if (DistSq < MinDistSq)
		{
			MinDistSq = DistSq;
			NearestPlanet = *It;
		}
	}

	if (NearestPlanet)
	{
		float Dist = FMath::Sqrt(MinDistSq);
		float HollowRadius = 0.0f;
		float AtmosphereRadius = 0.0f;

		if (NearestPlanet->HollowInnerSphere)
		{
			HollowRadius = NearestPlanet->HollowInnerSphere->GetScaledSphereRadius();
		}
		if (NearestPlanet->AtmosphereSphere)
		{
			AtmosphereRadius = NearestPlanet->AtmosphereSphere->GetScaledSphereRadius();
		}

		if (Dist < HollowRadius || Dist > AtmosphereRadius)
		{
			EnterZeroG(Dist < HollowRadius ? NearestPlanet : nullptr);
		}
		else
		{
			EnterSurfaceGravity(NearestPlanet);
		}
	}
	else
	{
		EnterZeroG(nullptr);
	}
}
