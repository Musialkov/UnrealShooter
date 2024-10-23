#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Enums/WeaponType.h"
#include "ShooterAnimInstance.generated.h"

class AShooterCharacter;
class UCustomMovementComponent;

UCLASS()
class SHOOTERV2_API UShooterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UShooterAnimInstance();
	
	UFUNCTION(BlueprintCallable)
	void UpdateAnimationProperties(float DeltaTime);

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	void TurnInPlace();
	void Lean(float DeltaTime);
	void SetRecoil();

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Movement, meta=(AllowPrivateAccess="true"))
	AShooterCharacter* ShooterCharacter;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Movement, meta=(AllowPrivateAccess="true"))
	float Speed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Movement, meta=(AllowPrivateAccess="true"))
	float MovementOffsetYaw;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Movement, meta=(AllowPrivateAccess="true"))
	float LastMovementOffsetYaw;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Movement, meta=(AllowPrivateAccess="true"))
	bool bIsJumping;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Movement, meta=(AllowPrivateAccess="true"))
	bool bIsAccelerating;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Turn in place", meta=(AllowPrivateAccess="true"))
	float RootYawOffset;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Turn in place", meta=(AllowPrivateAccess="true"))
	float Pitch;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Turn in place", meta=(AllowPrivateAccess="true"))
	bool bIsReloading;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Turn in place", meta=(AllowPrivateAccess="true"))
	bool bIsTurningInPlace;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Turn in place", meta=(AllowPrivateAccess="true"))
	//Is set in anim BP
	bool bIsRunning;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Crouching", meta=(AllowPrivateAccess="true"))
	bool bIsCrouching;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Crouching", meta=(AllowPrivateAccess="true"))
	bool bIsEquipping;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Combat", meta=(AllowPrivateAccess="true"))
	float RecoilWeight;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Combat", meta=(AllowPrivateAccess="true"))
	EWeaponType EquippedWeaponType;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Combat", meta=(AllowPrivateAccess="true"))
	bool bShouldUseFabrik;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Combat", meta=(AllowPrivateAccess="true"))
	bool bIsAiming;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Lean", meta=(AllowPrivateAccess="true"))
	float YawDelta;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Climbing", meta=(AllowPrivateAccess="true"))
	UCustomMovementComponent* CustomMovementComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Climbing", meta=(AllowPrivateAccess="true"))
	float GroundSpeed;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Climbing", meta=(AllowPrivateAccess="true"))
	float AirSpeed;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Climbing", meta=(AllowPrivateAccess="true"))
	FVector ClimbVelocity;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Climbing", meta=(AllowPrivateAccess="true"))
	bool bShouldMove;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Climbing", meta=(AllowPrivateAccess="true"))
	bool bIsFalling;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Climbing", meta=(AllowPrivateAccess="true"))
	bool bIsClimbing;
	
	float TurnInPlaceCharacterYaw;
	float TurnInPlaceCharacterYawLastFrame;

	FRotator CharacterRotation;
	FRotator CharacterRotationLastFrame;
	
	float RotationCurve;
	float RotationCurveLastFrame;
	
	
	float GetLateralSpeedOfCharacter() const;
	bool IsCharacterMoving() const;
	void UpdateGroundSpeed();
	void UpdateAirSpeed();
	void UpdateClimbVelocity();
	void UpdateShouldMove();
	void UpdateIsFalling();
	void UpdateIsClimbing();
};
