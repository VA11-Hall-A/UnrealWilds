// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "GravitySourceComponent.generated.h"

struct FGravitySourceData;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UNREALWILDS_API UGravitySourceComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UGravitySourceComponent();
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void OnRegister() override;
	virtual void OnUnregister() override;
	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Source")
	bool bUseInverseSquare = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Source",
		meta = (ForceUnits="Kg", ClampMin = "1"))
	double Mass = 5.9722E24;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Source")
	double PlanetRadius = 500.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Source")
	bool ApplyGravity = true;

	FGravitySourceData GetGravitySourceData() const;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	virtual void BuildAsyncInput();
};
