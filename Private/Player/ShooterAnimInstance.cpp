#include "Player/ShooterAnimInstance.h"

#include "Components/CustomMovementComponent.h"
#include "Player/ShooterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "Kismet/KismetMathLibrary.h"

UShooterAnimInstance::UShooterAnimInstance()
{
	Speed = 0;
	bIsJumping = false;
	bIsAccelerating = false;
	MovementOffsetYaw = 0;
	LastMovementOffsetYaw = 0.f;
	TurnInPlaceCharacterYaw = 0.f;
	TurnInPlaceCharacterYawLastFrame = 0.f;
	RootYawOffset = 0.f;
	CharacterRotation = FRotator(0.f);
	CharacterRotationLastFrame = FRotator(0.f);
	Pitch = 0.0f;
	bIsReloading = false;
	RecoilWeight = 1.0f;
	bIsTurningInPlace = false;
	EquippedWeaponType = EWeaponType::EWT_Rifle;
	bShouldUseFabrik = false;
}

void UShooterAnimInstance::UpdateAnimationProperties(float DeltaTime)
{
	if(ShooterCharacter == nullptr)
	{
		ShooterCharacter = Cast<AShooterCharacter>( TryGetPawnOwner());
	}

	if(!ShooterCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("Charater not found in animation instance class"));
		return;
	}
	
	Speed = GetLateralSpeedOfCharacter();
	bIsJumping = ShooterCharacter->GetCharacterMovement()->IsFalling();
	bIsAccelerating = IsCharacterMoving();

	bIsReloading = ShooterCharacter->GetCombatState() == ECombatState::ECS_Reloading;
	bIsCrouching = ShooterCharacter->IsCrouching();
	bIsEquipping = ShooterCharacter->GetCombatState() == ECombatState::ECS_Equipping;
	bIsAiming = ShooterCharacter->IsAiming();
	bShouldUseFabrik = ShooterCharacter->GetCombatState() == ECombatState::ECS_Unoccupied || ShooterCharacter->GetCombatState() == ECombatState::ECS_FireTimerInProgress;
	

	const FRotator AimRotation = ShooterCharacter->GetBaseAimRotation();
	const FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(ShooterCharacter->GetVelocity());
	MovementOffsetYaw = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation).Yaw;

	if(ShooterCharacter->GetVelocity().Size() > 0.f)
	{
		LastMovementOffsetYaw = MovementOffsetYaw;
	}

	if(ShooterCharacter->GetEquippedWeapon())
	{
		EquippedWeaponType = ShooterCharacter->GetEquippedWeapon()->GetWeaponType();
	}

	if(!CustomMovementComponent->IsClimbing())
	{
		TurnInPlace();
	}
	
	Lean(DeltaTime);
	SetRecoil();
}

void UShooterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	ShooterCharacter = Cast<AShooterCharacter>( TryGetPawnOwner());
	if(ShooterCharacter)
	{
		CustomMovementComponent = ShooterCharacter->GetCustomMovementComponent();
	}
}

void UShooterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if(!ShooterCharacter || !CustomMovementComponent) return;

	UpdateGroundSpeed();
	UpdateAirSpeed();
	UpdateClimbVelocity();
	UpdateShouldMove();
	UpdateIsFalling();
	UpdateIsClimbing();
}

void UShooterAnimInstance::TurnInPlace()
{
	if(!ShooterCharacter) return;
	
	Pitch = ShooterCharacter->GetBaseAimRotation().Pitch;
	
	if(Speed > 0 || bIsJumping || bIsRunning)
	{
		RootYawOffset = 0;
		TurnInPlaceCharacterYaw = ShooterCharacter->GetActorRotation().Yaw;
		TurnInPlaceCharacterYawLastFrame = TurnInPlaceCharacterYaw;
		RotationCurveLastFrame = 0;
		RotationCurve = 0;
	}
	else
	{
		TurnInPlaceCharacterYawLastFrame = TurnInPlaceCharacterYaw;
		TurnInPlaceCharacterYaw = ShooterCharacter->GetActorRotation().Yaw;
		const float TurnInPlaceYawDelta = TurnInPlaceCharacterYaw - TurnInPlaceCharacterYawLastFrame;
	
		RootYawOffset = UKismetMathLibrary::NormalizeAxis(RootYawOffset - TurnInPlaceYawDelta);

		const float Turning = GetCurveValue(TEXT("Turning"));
		if(Turning > 0)
		{
			bIsTurningInPlace = true;
			RotationCurveLastFrame = RotationCurve;
			RotationCurve = GetCurveValue(TEXT("Rotation"));

			const float RotationDelta = RotationCurve - RotationCurveLastFrame;
			RootYawOffset > 0 ? RootYawOffset -= RotationDelta : RootYawOffset += RotationDelta;

			const float AbsoluteYawOffset = FMath::Abs(RootYawOffset);
			if(AbsoluteYawOffset > 90.f)
			{
				const float YawExcess = AbsoluteYawOffset - 90.f;
				RootYawOffset > 0 ? RootYawOffset -= YawExcess : RootYawOffset += YawExcess;
			}
		}
		else
		{
			bIsTurningInPlace = false;
		}
	}
}

void UShooterAnimInstance::Lean(float DeltaTime)
{
	if(!ShooterCharacter) return;

	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = ShooterCharacter->GetActorRotation();

	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);

	const float Target = (Delta.Yaw / DeltaTime);
	const float Interp = FMath::FInterpTo(YawDelta, Target, DeltaTime, 6.f);

	YawDelta = FMath::Clamp(Interp, -90.f, 90.f);
}

void UShooterAnimInstance::SetRecoil()
{
	if(bIsReloading || bIsEquipping)
	{
		RecoilWeight = 1.f;
		return;
	}
	
	if(bIsTurningInPlace || bIsCrouching)
	{
		RecoilWeight = 0.2f;
	}
	else
	{
		RecoilWeight = 0.5f;
	}
}

float UShooterAnimInstance::GetLateralSpeedOfCharacter() const
{
	FVector Velocity = ShooterCharacter->GetVelocity();
	Velocity.Z = 0;
	return Velocity.Size();
}

bool UShooterAnimInstance::IsCharacterMoving() const
{
	if(ShooterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f)
	{
		return true;
	}
	return false;
}

void UShooterAnimInstance::UpdateGroundSpeed()
{
	GroundSpeed = UKismetMathLibrary::VSizeXY(ShooterCharacter->GetVelocity());
}

void UShooterAnimInstance::UpdateAirSpeed()
{
	AirSpeed = ShooterCharacter->GetVelocity().Z;
}

void UShooterAnimInstance::UpdateClimbVelocity()
{
	ClimbVelocity = CustomMovementComponent->GetUnrotatedClimbVelocity();
}

void UShooterAnimInstance::UpdateShouldMove()
{
	bShouldMove = CustomMovementComponent->GetCurrentAcceleration().Size() > 0 && GroundSpeed > 5.f && !bIsFalling;
}

void UShooterAnimInstance::UpdateIsFalling()
{
	bIsFalling = CustomMovementComponent->IsFalling();
}

void UShooterAnimInstance::UpdateIsClimbing()
{
	bIsClimbing = CustomMovementComponent->IsClimbing();
}
