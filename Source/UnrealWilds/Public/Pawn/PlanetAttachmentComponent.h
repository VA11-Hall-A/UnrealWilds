#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlanetAttachmentComponent.generated.h"

class APlanet;
class UPrimitiveComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAttachedToPlanet, APlanet*, Planet);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDetachedFromPlanet, FVector, OrbitalVelocity);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UNREALWILDS_API UPlanetAttachmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlanetAttachmentComponent();

	UFUNCTION(BlueprintCallable, Category="Planet")
	void Initialize(UPrimitiveComponent* InPhysicsPrimitive);

	void AttachToPlanet(APlanet* Planet);
	void DetachFromPlanet();

	FVector GetOrbitalVelocity() const;

	UFUNCTION(BlueprintCallable, Category="Planet")
	APlanet* GetCurrentPlanet() const { return CurrentPlanet; }

	UFUNCTION(BlueprintCallable, Category="Planet")
	bool IsAttachedToPlanet() const { return CurrentPlanet != nullptr; }

	UFUNCTION(BlueprintCallable, Category="Planet")
	void CheckInitialPlanetState();

	UPROPERTY(BlueprintAssignable, Category="Planet") 
	FOnAttachedToPlanet OnAttachedToPlanet;

	UPROPERTY(BlueprintAssignable, Category="Planet")
	FOnDetachedFromPlanet OnDetachedFromPlanet;

private:
	UPROPERTY()
	TObjectPtr<APlanet> CurrentPlanet;

	UPROPERTY()
	TObjectPtr<UPrimitiveComponent> PhysicsPrimitive;
};
