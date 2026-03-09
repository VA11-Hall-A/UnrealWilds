// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ProbeLauncherComponent.generated.h"

class AProbe;

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

	UFUNCTION(BlueprintPure, Category="Probe")
	AProbe* GetActiveProbe() const;

protected:
	UPROPERTY(EditDefaultsOnly, Category="Probe")
	TSubclassOf<AProbe> ProbeClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Probe")
	float LaunchSpeed = 5000.0f;

	static TWeakObjectPtr<AProbe> ActiveProbe;
};
