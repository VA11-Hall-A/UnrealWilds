// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ProbeLauncherComponent.generated.h"

class AProbe;
class UTextureRenderTarget2D;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPhotoCaptured, UTextureRenderTarget2D*, RenderTarget);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UNREALWILDS_API UProbeLauncherComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UProbeLauncherComponent();

	UFUNCTION(BlueprintCallable, Category="Probe")
	void LaunchProbe();

	UFUNCTION(BlueprintCallable, Category="Probe")
	void RecallProbe();

	UFUNCTION(BlueprintCallable, Category="Probe")
	void ToggleProbe();

	UFUNCTION(BlueprintPure, Category="Probe")
	AProbe* GetActiveProbe() const;

	UFUNCTION(BlueprintCallable, Category="Probe|Camera")
	void RotateProbeCamera();

	UFUNCTION(BlueprintCallable, Category="Probe|Camera")
	void CaptureProbePhoto();

	UPROPERTY(BlueprintAssignable, Category="Probe|Camera")
	FOnPhotoCaptured OnPhotoCaptured;

protected:
	UPROPERTY(EditDefaultsOnly, Category="Probe")
	TSubclassOf<AProbe> ProbeClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Probe")
	float LaunchSpeed = 5000.0f;

	static TWeakObjectPtr<AProbe> ActiveProbe;
};
