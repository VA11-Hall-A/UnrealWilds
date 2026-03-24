// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "UWCharacter.generated.h"

class UThrusterComponent;
class UProbeLauncherComponent;
class UPlanetAttachmentComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class APlanet;
class AShipPawn;
struct FInputActionValue;

UENUM(BlueprintType)
enum class ECharacterMovementState : uint8
{
	SurfaceGravity UMETA(DisplayName = "Surface Gravity"),
	TransitionToSurface UMETA(DisplayName = "Transition To Surface"),
	ZeroG UMETA(DisplayName = "Zero Gravity")
};

UCLASS()
class UNREALWILDS_API AUWCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	
	AUWCharacter(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void EnterSurfaceGravity(FVector OverrideUpVector = FVector::ZeroVector, double OverrideGravityAcceleration = -1.0);

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void EnterZeroG(FVector InheritedOrbitalVelocity = FVector::ZeroVector);

	void UpdateTransitionSurfaceGravity(FVector OverrideUpVector, double OverrideGravityAcceleration = -1.0);

	ECharacterMovementState GetCurrentMovementState() const;

	UFUNCTION(BlueprintCallable, Category = "Movement")
	bool IsOnGravityFloor() const;

	virtual FVector GetVelocity() const override;

	void CheckInitialMovementState();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	ECharacterMovementState CurrentMovementState = ECharacterMovementState::SurfaceGravity;

	/** Re-entrancy guard: prevents overlap callbacks from recursing during state transitions. */
	bool bIsTransitioningState = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Astro|Planet")
	TObjectPtr<UPlanetAttachmentComponent> PlanetAttachment;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	// ── Enhanced Input ──────────────────────────────────────────────────────

	/** Always-active context: Look, Interact, Probes */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputMappingContext> CommonMappingContext;

	/** Surface walking: Move + Jump */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputMappingContext> FootMappingContext;

	/** Airborne / Zero-G: 6-axis thrust */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputMappingContext> ThrustMappingContext;

	/** Zero-G only: roll rotation */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputMappingContext> RollMappingContext;

	/** Move action (2-D axis: X = forward/backward, Y = right/left) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputAction> MoveAction;

	/** Look action (2-D axis: X = yaw, Y = pitch) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputAction> FlyingMoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputAction> RollAction;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	virtual void Jump() override;

	void FlyingMove(const FInputActionValue& Value);
	void Roll(const FInputActionValue& Value);
	void OnInteract();

	UPROPERTY(BlueprintReadOnly)
	UThrusterComponent* Thruster;

	UPROPERTY(EditDefaultsOnly)
	UProbeLauncherComponent* ProbeLauncher;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputAction> ToggleProbeAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputAction> RotateProbeCameraAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputAction> CaptureProbePhotoAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputAction> InteractAction;

	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode = 0) override;

	UFUNCTION()
	void OnAttachedToPlanet(APlanet* Planet);

	UFUNCTION()
	void OnDetachedFromPlanet(FVector OrbitalVelocity);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	TObjectPtr<UCameraComponent> FirstPersonCameraComponent;

	void StartTransitionToSurface(FVector OverrideUpVector, double OverrideGravityAcceleration);
	void TickTransitionToSurface(float DeltaTime);
	void FinishTransitionToSurface();
	void AbortTransitionToSurface(FVector InheritedOrbitalVelocity = FVector::ZeroVector);
	void SnapToSurfaceGravity(FVector OverrideUpVector, double OverrideGravityAcceleration);
	bool ResolveSurfaceGravity(FVector OverrideUpVector, double OverrideGravityAcceleration, FVector& OutUpVector, FVector& OutGravityDirection, double& OutGravityAcceleration) const;

	FQuat TransitionStartActorQuat = FQuat::Identity;
	FQuat TransitionTargetActorQuat = FQuat::Identity;
	FQuat TransitionStartCameraQuat = FQuat::Identity;
	FQuat TransitionTargetCameraQuat = FQuat::Identity;
	FVector TransitionVelocityWorld = FVector::ZeroVector;
	FVector TransitionGravityDirection = FVector::DownVector;
	FVector TransitionCameraLocalOffset = FVector::ZeroVector;
	double TransitionGravityAcceleration = 980.0;
	float TransitionElapsed = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float TransitionDuration = 0.5f;

	bool bTransitionPendingFloorHit = false;

	UPROPERTY(VisibleInstanceOnly)
	bool bOffGround =false;

	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite)
	float TorqueMultiplier=150.0f;

};

