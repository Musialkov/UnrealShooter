#include "Components/CustomMovementComponent.h"

#include "MotionWarpingComponent.h"
#include "Enums/CustomMovementMode.h"
#include "Kismet/KismetMathLibrary.h" 
#include "Player/ShooterCharacter.h"
#include "Kismet/KismetSystemLibrary.h"

#pragma region PublicSection
FVector UCustomMovementComponent::GetUnrotatedClimbVelocity() const
{
	return UKismetMathLibrary::Quat_UnrotateVector(UpdatedComponent->GetComponentQuat(), Velocity);
}

void UCustomMovementComponent::ToggleClimbing(bool bEnableClimb)
{
	if(bEnableClimb)
	{
		if(CanStartClimbing())
		{
			if(!IsFalling())
			{
				PlayClimbMontage(IdleToCLimbMontage);
			}
			StartClimbing();
		}
		else if(CanClimbDownLedge())
		{
			PlayClimbMontage(CLimbDownLedgeMontage);
			StartClimbing();
		}
	}
	
	if(!bEnableClimb)
	{
		StopClimbing();
	}
}

bool UCustomMovementComponent::IsClimbing() const
{
	return MovementMode == MOVE_Custom && CustomMovementMode ==  ECustomMovementMode::MOVE_Climb;
}

bool UCustomMovementComponent::CanStartVaulting()
{
	FVector VaultStartPosition;
	FVector VaultEndPosition;
	if(CanStartClimbing()) return false;
	if(CanClimbDownLedge()) return false;
	return CanStartVaulting(VaultStartPosition, VaultEndPosition);
}

bool UCustomMovementComponent::TryStartVaulting()
{
	FVector VaultStartPosition;
	FVector VaultEndPosition;
	if(!CanStartVaulting(VaultStartPosition, VaultEndPosition)) return false;

	SetMotionWarpTarget(FName("VaultStartPoint"), VaultStartPosition);
	SetMotionWarpTarget(FName("VaultEndPoint"), VaultEndPosition);

	StartClimbing();
	PlayClimbMontage(VaultMontage);

	return true;
}
#pragma endregion

#pragma region OverridenFunctions
void UCustomMovementComponent::BeginPlay()
{
	Super::BeginPlay();
	OwningAnimInstance = CharacterOwner->GetMesh()->GetAnimInstance();

	if(OwningAnimInstance)
	{
		OwningAnimInstance->OnMontageEnded.AddDynamic(this, &UCustomMovementComponent::OnClimbMontageEnded);
		OwningAnimInstance->OnMontageBlendingOut.AddDynamic(this, &UCustomMovementComponent::OnClimbMontageEnded);
	}

	OwningPlayerCharacter = Cast<AShooterCharacter>(CharacterOwner);
}

void UCustomMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                             FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	CanClimbDownLedge();
}

void UCustomMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

	if(PreviousMovementMode == MOVE_Custom && PreviousCustomMode == ECustomMovementMode::MOVE_Climb)
	{
		const FRotator DirtyRotation = UpdatedComponent->GetComponentRotation();
		const FRotator CleanRotation = FRotator(0.f, DirtyRotation.Yaw, 0.f);
		UpdatedComponent->SetRelativeRotation(CleanRotation);
		
		StopMovementImmediately();
	}
}

void UCustomMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{
	if(IsClimbing())
	{
		PhysClimb(deltaTime, Iterations);
	}
	
	Super::PhysCustom(deltaTime, Iterations);
}

float UCustomMovementComponent::GetMaxSpeed() const
{
	if(IsClimbing())
	{
		return MaxClimbingSpeed;
	}
	
	return Super::GetMaxSpeed();
}

float UCustomMovementComponent::GetMaxAcceleration() const
{
	if(IsClimbing())
	{
		return MaxClimbingAcceleration;
	}
	return Super::GetMaxAcceleration();
}

FVector UCustomMovementComponent::ConstrainAnimRootMotionVelocity(const FVector& RootMotionVelocity,
	const FVector& CurrentVelocity) const
{
	const bool bIsPayingRootMotionMontage = IsFalling() && OwningAnimInstance && OwningAnimInstance->IsAnyMontagePlaying();

	if(bIsPayingRootMotionMontage)
	{
		return RootMotionVelocity;
	}

	return Super::ConstrainAnimRootMotionVelocity(RootMotionVelocity, CurrentVelocity);
}

#pragma endregion 

#pragma region ClimbTraces
TArray<FHitResult> UCustomMovementComponent::DoCapsuleTraceMultiByObject(const FVector& Start, const FVector& End,
bool bShouldShowCapsuleDebug, bool bDrawPersistantShapes)
{
	TArray<FHitResult> OutCapsuleTraceHitResults;
	EDrawDebugTrace::Type DebugTraceType = EDrawDebugTrace::None;
	if(bShouldShowCapsuleDebug)
	{
		DebugTraceType = EDrawDebugTrace::ForOneFrame;
		if(bDrawPersistantShapes)
		{
			DebugTraceType = EDrawDebugTrace::Persistent;
		}
	}
	
	UKismetSystemLibrary::CapsuleTraceMultiForObjects(
		this,
		Start,
		End,
		ClimbCapsuleTraceRadius,
		ClimbCapsuleTraceHalfHeight,
		ClimbableSurfaceTraceTypes,
		false,
		TArray<AActor*>(),
		DebugTraceType,
		OutCapsuleTraceHitResults,
		false);
	
	return OutCapsuleTraceHitResults;
}

FHitResult UCustomMovementComponent::DoLineTraceSingleByObject(const FVector& Start, const FVector& End,
	bool bShouldShowCapsuleDebug,  bool bDrawPersistantShapes)
{
	FHitResult OutHit;
	EDrawDebugTrace::Type DebugTraceType = EDrawDebugTrace::None;
	if(bShouldShowCapsuleDebug)
	{
		DebugTraceType = EDrawDebugTrace::ForOneFrame;
		if(bDrawPersistantShapes)
		{
			DebugTraceType = EDrawDebugTrace::Persistent;
		}
	}
	
	UKismetSystemLibrary::LineTraceSingleForObjects(
	this,
	Start,
	End,
	ClimbableSurfaceTraceTypes,
	false,
	TArray<AActor*>(),
	DebugTraceType,
	OutHit,
	false);

	return OutHit;
}

#pragma endregion

#pragma region ClimbCore
bool UCustomMovementComponent::TraceClimbableSurfaces()
{
	const FVector StartOffset = UpdatedComponent->GetForwardVector() * 30.0f;
	const FVector Start = UpdatedComponent->GetComponentLocation() + StartOffset;
	const FVector End = Start + UpdatedComponent->GetForwardVector();
	
	ClimbableSurfacesTracedResults = DoCapsuleTraceMultiByObject(Start, End);

	return !ClimbableSurfacesTracedResults.IsEmpty();
}

FHitResult UCustomMovementComponent::TraceFromEyeHeight(float TraceDistance, float TraceStartOffset)
{
	const FVector EyeHeightOffset = UpdatedComponent->GetUpVector() * (CharacterOwner->BaseEyeHeight + TraceStartOffset);

	const FVector Start = UpdatedComponent->GetComponentLocation() + EyeHeightOffset;
	const FVector End = Start + UpdatedComponent->GetForwardVector() * TraceDistance;

	FHitResult HitResult = DoLineTraceSingleByObject(Start, End);

	return HitResult;
}

bool UCustomMovementComponent::CanStartClimbing()
{
	if(!TraceClimbableSurfaces()) return false;
	if(!TraceFromEyeHeight(100).bBlockingHit) return false;

	return true;
}

bool UCustomMovementComponent::CanClimbDownLedge()
{
	if(IsFalling()) return false;

	const FVector ComponentLocation = UpdatedComponent->GetComponentLocation();
	const FVector ComponentForward = UpdatedComponent->GetForwardVector();
	const FVector DownVector = -UpdatedComponent->GetUpVector();

	const FVector WalkableSurfaceTraceStart = ComponentLocation + ComponentForward * ClimbDawnWalkableSurfaceOffset;
	const FVector WalkableSurfaceTraceEnd = WalkableSurfaceTraceStart + DownVector * 100.f;

	FHitResult WalkableSurfaceHit = DoLineTraceSingleByObject(WalkableSurfaceTraceStart, WalkableSurfaceTraceEnd);

	const FVector LedgeTraceStart = WalkableSurfaceHit.TraceStart + ComponentForward * ClimbDawnLedgeOffset;
	const FVector LedgeTraceEnd = LedgeTraceStart + DownVector * 300.f;

	FHitResult LedgeHit = DoLineTraceSingleByObject(LedgeTraceStart, LedgeTraceEnd);

	if(WalkableSurfaceHit.bBlockingHit && !LedgeHit.bBlockingHit)
	{
		return true;
	}

	return false;
}

bool UCustomMovementComponent::CheckShouldStopClimbing()
{
	if(ClimbableSurfacesTracedResults.IsEmpty()) return true;

	const float DotResult = FVector::DotProduct(CurrentClimbableSurfaceNormal, FVector::UpVector);
	const float DegreeDiff = FMath::RadiansToDegrees(FMath::Acos(DotResult));

	if(DegreeDiff <= 70) return true;

	return false;
}

bool UCustomMovementComponent::CheckHasReachedFloor()
{
	const FVector DownVector = -UpdatedComponent->GetUpVector();
	const FVector StartOffset = DownVector * 20;

	const FVector Start = UpdatedComponent->GetComponentLocation() + StartOffset;
	const FVector End = Start + DownVector;

	TArray<FHitResult> PossibleFloorHits = DoCapsuleTraceMultiByObject(Start, End, false);
	if(PossibleFloorHits.IsEmpty()) return false;

	for(const FHitResult& PossibleFloorHit : PossibleFloorHits)
	{
		const bool bFloorReached = FVector::Parallel(-PossibleFloorHit.ImpactNormal, FVector::UpVector) &&
			GetUnrotatedClimbVelocity().Z <= 10.f;
		if(bFloorReached)
		{
			return true;
		}
	}

	return false;
}

bool UCustomMovementComponent::CheckHasReachedLedge()
{
	FHitResult LedgeHit = TraceFromEyeHeight(100.f, 25.f);
	if(!LedgeHit.bBlockingHit)
	{
		const FVector Start = LedgeHit.TraceEnd;
		const FVector DownVector = -UpdatedComponent->GetUpVector();
		const FVector End = Start + DownVector * 100.f;

		const FHitResult WalkableSurfaceHitResult = DoLineTraceSingleByObject(Start, End);
		if(WalkableSurfaceHitResult.bBlockingHit || GetUnrotatedClimbVelocity().Z > 10)
		{
			return true;
		}
	}

	return false;
}

bool UCustomMovementComponent::CanStartVaulting(FVector& OutVaultStartPosition, FVector& OutVaultLandPosition)
{
	if(IsFalling()) return false;

	OutVaultStartPosition = FVector::ZeroVector;
	OutVaultLandPosition = FVector::ZeroVector;
	
	const FVector ComponentLocation = UpdatedComponent->GetComponentLocation();
	const FVector ComponentForward = UpdatedComponent->GetForwardVector();
	const FVector ComponentUp = UpdatedComponent->GetUpVector();
	const FVector ComponentDown = -UpdatedComponent->GetUpVector();

	for(int i = 0; i < 4; i++)
	{
		const FVector Start = ComponentLocation + ComponentUp * 100 + ComponentForward * 90 * (i+1);
		const FVector End = Start + ComponentDown * 100 * (i+1);

		FHitResult VaultHitResult = DoLineTraceSingleByObject(Start, End);

		if(i == 0 && VaultHitResult.bBlockingHit)
		{
			OutVaultStartPosition = VaultHitResult.ImpactPoint;
		}

		if(i == 3 && VaultHitResult.bBlockingHit)
		{
			OutVaultLandPosition = VaultHitResult.ImpactPoint;
		}
	}

	return OutVaultStartPosition != FVector::ZeroVector && OutVaultLandPosition != FVector::ZeroVector;
}

void UCustomMovementComponent::StartClimbing()
{
	SetMovementMode(MOVE_Custom, ECustomMovementMode::MOVE_Climb);
	OnStartClimbing.Broadcast();
}

void UCustomMovementComponent::StopClimbing()
{
	SetMovementMode(MOVE_Walking);
	OnStopClimbing.Broadcast();
}

void UCustomMovementComponent::PhysClimb(float deltaTime, int32 Iterations)
{
	if (deltaTime < MIN_TICK_TIME)
	{
		return;
	}

	TraceClimbableSurfaces();
	ProcessClimbableSurfaceInfo();

	if(OwningAnimInstance->GetCurrentActiveMontage() != CLimbDownLedgeMontage &&
		OwningAnimInstance->GetCurrentActiveMontage() != VaultMontage &&
		OwningAnimInstance->GetCurrentActiveMontage() != CLimbToTopMontage &&
		(CheckShouldStopClimbing() || CheckHasReachedFloor()))
	{
		StopClimbing();
	}
	
	RestorePreAdditiveRootMotionVelocity();

	if(!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() )
	{
		CalcVelocity(deltaTime, 0.f, true, MaxBreakClimbDeceleration);
	}

	ApplyRootMotionToVelocity(deltaTime);

	FVector OldLocation = UpdatedComponent->GetComponentLocation();
	const FVector Adjusted = Velocity * deltaTime;
	FHitResult Hit(1.f);

	SafeMoveUpdatedComponent(Adjusted, GetClimbRotation(deltaTime), true, Hit);
	
	if (Hit.Time < 1.f)
	{
		HandleImpact(Hit, deltaTime, Adjusted);
		SlideAlongSurface(Adjusted, (1.f - Hit.Time), Hit.Normal, Hit, true);
	}

	if(!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
	{
		Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / deltaTime;
	}

	SnapMovementToClimbableSurfaces(deltaTime);

	if(CheckHasReachedLedge())
	{
		PlayClimbMontage(CLimbToTopMontage);
	}
}

void UCustomMovementComponent::ProcessClimbableSurfaceInfo()
{
	CurrentClimbableSurfaceLocation = FVector::ZeroVector;
	CurrentClimbableSurfaceNormal = FVector::ZeroVector;

	if(ClimbableSurfacesTracedResults.IsEmpty()) return;

	for(const FHitResult& TracedHitResults : ClimbableSurfacesTracedResults)
	{
		CurrentClimbableSurfaceLocation += TracedHitResults.ImpactPoint;
		CurrentClimbableSurfaceNormal += TracedHitResults.ImpactNormal;
	}

	CurrentClimbableSurfaceLocation /= ClimbableSurfacesTracedResults.Num();
	CurrentClimbableSurfaceNormal = CurrentClimbableSurfaceNormal.GetSafeNormal();
}

void UCustomMovementComponent::SnapMovementToClimbableSurfaces(float DeltaTime)
{
	const FVector ComponentForward = UpdatedComponent->GetForwardVector();
	const FVector ComponentLocation = UpdatedComponent->GetComponentLocation();

	const FVector ProjectedCharacterToSurface = (CurrentClimbableSurfaceLocation - ComponentLocation).ProjectOnTo(ComponentForward);
	const FVector SnapVector = -CurrentClimbableSurfaceNormal * ProjectedCharacterToSurface.Length();

	UpdatedComponent->MoveComponent(SnapVector * DeltaTime * MaxClimbingSpeed, UpdatedComponent->GetComponentQuat(), true);
	
}

void UCustomMovementComponent::PlayClimbMontage(UAnimMontage* MontageToPlay)
{
	if(!MontageToPlay) return;
	if(!OwningAnimInstance) return;
	if(OwningAnimInstance->IsAnyMontagePlaying()) return;

	OwningAnimInstance->Montage_Play(MontageToPlay);
}

void UCustomMovementComponent::SetMotionWarpTarget(const FName& InWarpTargetName, const FVector& InTargetPosition)
{
	if(!OwningPlayerCharacter) return;

	OwningPlayerCharacter->GetMotionWarpingComponent()->AddOrUpdateWarpTargetFromLocation(InWarpTargetName, InTargetPosition);
}

FQuat UCustomMovementComponent::GetClimbRotation(float DeltaTime)
{
	const FQuat CurrentQuat = UpdatedComponent->GetComponentQuat();
	if(HasAnimRootMotion() || CurrentRootMotion.HasOverrideVelocity())
	{
		return CurrentQuat;
	}

	const FQuat TargetQuat = FRotationMatrix::MakeFromX(-CurrentClimbableSurfaceNormal).ToQuat();
	
	return FMath::QInterpTo(CurrentQuat, TargetQuat, DeltaTime, 5.f);;
}

void UCustomMovementComponent::OnClimbMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if(Montage == IdleToCLimbMontage || Montage == CLimbDownLedgeMontage)
	{
		//StopMovementImmediately();
	}
	
	if(Montage == CLimbToTopMontage || Montage == VaultMontage)
	{
		SetMovementMode(MOVE_Walking);
	}
}

#pragma endregion 
