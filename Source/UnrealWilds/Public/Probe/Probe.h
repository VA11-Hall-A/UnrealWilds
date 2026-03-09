// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Probe.generated.h"

UCLASS()
class UNREALWILDS_API AProbe : public AActor
{
	GENERATED_BODY()

public:
	AProbe();

	void Launch(const FVector& Direction, float Speed, const FVector& InheritedVelocity);

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnProbeHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> ProbeMesh;

	bool bIsAttached = false;
};
