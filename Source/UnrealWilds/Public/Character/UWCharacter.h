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
	ZeroG UMETA(DisplayName = "Zero Gravity")
};

UCLASS()
class UNREALWILDS_API AUWCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	
	AUWCharacter(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void EnterSurfaceGravity();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void EnterZeroG(FVector InheritedOrbitalVelocity = FVector::ZeroVector);

	ECharacterMovementState GetCurrentMovementState() const;

	virtual FVector GetVelocity() const override;

	void CheckInitialMovementState();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	ECharacterMovementState CurrentMovementState = ECharacterMovementState::SurfaceGravity;

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

	void FlyingMove(const FInputActionValue& Value);
	void Roll(const FInputActionValue& Value);
	void OnInteract();

	UPROPERTY(EditDefaultsOnly)
	UThrusterComponent* Thruster;

	UPROPERTY(EditDefaultsOnly)
	UProbeLauncherComponent* ProbeLauncher;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputAction> LaunchProbeAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputAction> RecallProbeAction;

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

	// ── Camera ──────────────────────────────────────────────────────────────

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	TObjectPtr<UCameraComponent> FirstPersonCameraComponent;

	UPROPERTY(VisibleInstanceOnly)
	bool bOffGround =false;
	

};


