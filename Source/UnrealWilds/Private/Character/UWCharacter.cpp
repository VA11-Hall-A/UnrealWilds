// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/UWCharacter.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "Character/UWCharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Controller.h"
#include "InputActionValue.h"
#include "Engine/Engine.h"
#include "Gravity/GravityWorldSubsystem.h"
#include "Pawn/ThrusterComponent.h"

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

	Thruster=CreateDefaultSubobject<UThrusterComponent>(TEXT("Thruster"));
	Thruster->ThrustForce=1000.0;
	Thruster->bIsCharacterMode=true;
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

	if (Controller)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
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
