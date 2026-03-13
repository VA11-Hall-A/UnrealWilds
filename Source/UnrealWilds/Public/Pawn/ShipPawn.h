#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "ShipPawn.generated.h"

class UStaticMeshComponent;
class UCameraComponent;
class UThrusterComponent;
class UProbeLauncherComponent;
class UPlanetAttachmentComponent;
class USphereComponent;
class UInputMappingContext;
class UInputAction;
class AUWCharacter;
struct FInputActionValue;

UCLASS()
class UNREALWILDS_API AShipPawn : public APawn
{
	GENERATED_BODY()

public:
	AShipPawn();

	virtual FVector GetVelocity() const override;

	void BoardShip(AUWCharacter* Character);
	void OnExitShip();

	bool IsOccupied() const { return StoredCharacter != nullptr; }

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;

	// ── Components ──────────────────────────────────────────────────────────

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ship")
	TObjectPtr<UStaticMeshComponent> ShipMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ship|Camera")
	TObjectPtr<UCameraComponent> ShipCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ship")
	TObjectPtr<UThrusterComponent> Thruster;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ship")
	TObjectPtr<UProbeLauncherComponent> ProbeLauncher;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ship|Interaction")
	TObjectPtr<USphereComponent> InteractionZone;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ship")
	TObjectPtr<UPlanetAttachmentComponent> PlanetAttachment;

	// ── State ───────────────────────────────────────────────────────────────

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Ship")
	TObjectPtr<AUWCharacter> StoredCharacter;

	// ── Input ───────────────────────────────────────────────────────────────

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputMappingContext> ShipMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputAction> FlyingMoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputAction> LaunchProbeAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputAction> RecallProbeAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputAction> InteractAction;

	// ── Config ──────────────────────────────────────────────────────────────

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ship|Movement")
	float TorqueMultiplier = 15.0f;

private:
	void ShipLook(const FInputActionValue& Value);
	void ShipFlyingMove(const FInputActionValue& Value);
	void OnInteract();

	UFUNCTION()
	void OnInteractionZoneBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
