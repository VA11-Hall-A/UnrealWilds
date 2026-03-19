// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Probe.generated.h"

class USceneCaptureComponent2D;
class UTextureRenderTarget2D;

UCLASS()
class UNREALWILDS_API AProbe : public AActor
{
	GENERATED_BODY()

public:
	AProbe();

	void Launch(const FVector& Direction, float Speed, const FVector& InheritedVelocity);

	/** Rotate the probe camera left by 45 degrees around the probe's local Z axis. */
	UFUNCTION(BlueprintCallable, Category="Probe|Camera")
	void RotateCamera();

	/** Capture a photo from the probe camera and return the render target. */
	UFUNCTION(BlueprintCallable, Category="Probe|Camera")
	UTextureRenderTarget2D* CapturePhoto();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnProbeHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> ProbeMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Probe|Camera")
	TObjectPtr<USceneCaptureComponent2D> ProbeCamera;

	UPROPERTY(Transient, BlueprintReadOnly, Category="Probe|Camera")
	TObjectPtr<UTextureRenderTarget2D> PhotoRenderTarget;

	/** Render target resolution (width and height). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Probe|Camera")
	int32 RenderTargetResolution = 1024;

	bool bIsAttached = false;

	int32 CameraYawStep = 0;
};
