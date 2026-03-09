// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "UWCharacter.generated.h"

class UThrusterComponent;
class UProbeLauncherComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
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
	void EnterSurfaceGravity(class APlanet* Planet = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void EnterZeroG(class APlanet* Planet = nullptr);

	ECharacterMovementState GetCurrentMovementState() const;

	virtual FVector GetVelocity() const override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void CheckInitialMovementState();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	ECharacterMovementState CurrentMovementState = ECharacterMovementState::SurfaceGravity;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Astro|Planet")
	class APlanet* CurrentPlanet;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	// ── Enhanced Input ──────────────────────────────────────────────────────

	/** Default mapping context, added on BeginPlay */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputMappingContext> FlyingMappingContext;

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

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	
	void FlyingMove(const FInputActionValue& Value);

	UPROPERTY(EditDefaultsOnly)
	UThrusterComponent* Thruster;

	UPROPERTY(EditDefaultsOnly)
	UProbeLauncherComponent* ProbeLauncher;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputAction> LaunchProbeAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputAction> RecallProbeAction;

	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode = 0) override;

	// ── Camera ──────────────────────────────────────────────────────────────

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	TObjectPtr<UCameraComponent> FirstPersonCameraComponent;

	UPROPERTY(VisibleInstanceOnly)
	bool bOffGround =false;
	

};


