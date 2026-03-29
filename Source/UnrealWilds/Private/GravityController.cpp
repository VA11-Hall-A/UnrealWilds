#include "GravityController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Character/UWCharacter.h"

void AGravityController::UpdateRotation(float DeltaTime)
{
	FVector GravityDirection = FVector::DownVector;
	bool bIsZeroG = false;
	bool bIsTransitionToSurface = false;

	APawn* CurrentPawn = GetPawn();
	if (CurrentPawn && !Cast<ACharacter>(CurrentPawn))
	{
		SetControlRotation(CurrentPawn->GetActorRotation());
		return;
	}

	if (ACharacter* PlayerCharacter = Cast<ACharacter>(CurrentPawn))
	{
		if (UCharacterMovementComponent* MoveComp = PlayerCharacter->GetCharacterMovement())
		{
			GravityDirection = MoveComp->GetGravityDirection();
		}

		if (AUWCharacter* UWCharacter = Cast<AUWCharacter>(PlayerCharacter))
		{
			const ECharacterMovementState MovementState = UWCharacter->GetCurrentMovementState();
			bIsZeroG = (MovementState == ECharacterMovementState::ZeroG);
			bIsTransitionToSurface = (MovementState == ECharacterMovementState::TransitionToSurface);
		}
	}

	if (bIsZeroG)
	{
		if (APawn* const P = GetPawnOrSpectator())
		{
			SetControlRotation(P->GetActorRotation());
		}
		return;
	}

	if (bIsTransitionToSurface)
	{
		LastFrameGravity = GravityDirection;
		return;
	}

	FRotator ViewRotation = GetControlRotation();

	if (!LastFrameGravity.Equals(FVector::ZeroVector))
	{
		const FQuat DeltaGravityRotation = FQuat::FindBetweenNormals(LastFrameGravity, GravityDirection);
		const FQuat WarpedCameraRotation = DeltaGravityRotation * FQuat(ViewRotation);

		ViewRotation = WarpedCameraRotation.Rotator();	
	}
	LastFrameGravity = GravityDirection;

	ViewRotation = GetGravityRelativeRotation(ViewRotation, GravityDirection);

	FRotator DeltaRot(RotationInput);

	if (PlayerCameraManager)
	{
		PlayerCameraManager->ProcessViewRotation(DeltaTime, ViewRotation, DeltaRot);
		ViewRotation.Roll = 0;
		SetControlRotation(GetGravityWorldRotation(ViewRotation, GravityDirection));
	}

	// APawn* const P = GetPawnOrSpectator();
	// if (P)
	// {
	// 	P->FaceRotation(ViewRotation, DeltaTime);
	// }
}

FRotator AGravityController::GetGravityRelativeRotation(FRotator Rotation, FVector GravityDirection)
{
	if (!GravityDirection.Equals(FVector::DownVector))
	{
		FQuat GravityRotation = FQuat::FindBetweenNormals(GravityDirection, FVector::DownVector);
		return (GravityRotation * Rotation.Quaternion()).Rotator();
	}

	return Rotation;
}

FRotator AGravityController::GetGravityWorldRotation(FRotator Rotation, FVector GravityDirection)
{
	if (!GravityDirection.Equals(FVector::DownVector))
	{
		FQuat GravityRotation = FQuat::FindBetweenNormals(FVector::DownVector, GravityDirection);
		return (GravityRotation * Rotation.Quaternion()).Rotator();
	}

	return Rotation;
}
