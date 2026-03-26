#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GravityFloor.generated.h"

class UBoxComponent;
class UPrimitiveComponent;
class UStaticMeshComponent;
class AUWCharacter;

UCLASS()
class UNREALWILDS_API AGravityFloor : public AActor
{
	GENERATED_BODY()

public:
	AGravityFloor();

	FVector GetGravityDirection() const;
	double GetGravityAcceleration() const;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GravityFloor")
	TObjectPtr<UBoxComponent> DetectionVolume;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GravityFloor")
	TObjectPtr<UBoxComponent> EntryVolume;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GravityFloor")
	TObjectPtr<UStaticMeshComponent> FloorMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GravityFloor", meta = (ForceUnits = "cm/s2"))
	float GravityAcceleration = 980.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GravityFloor", meta = (ForceUnits = "cm"))
	FVector BoxExtent = FVector(200.0f, 200.0f, 150.0f);

private:
	UFUNCTION()
	void OnEntryVolumeBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnVolumeEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void DetermineExitState(AUWCharacter* Character);
};
