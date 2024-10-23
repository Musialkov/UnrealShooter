#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CustomMovementComponent.generated.h"

class UAnimMontage;
class UAnimInstance;
class AShooterCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStartClimbingDelegate)
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStopClimbingDelegate)

UCLASS()
class SHOOTERV2_API UCustomMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
#pragma region PublicSection
	FORCEINLINE FVector GetClimbableSurfaceNormal() const {return CurrentClimbableSurfaceNormal;}
	FVector GetUnrotatedClimbVelocity() const;
	void ToggleClimbing(bool bEnableClimb);
	bool TryStartVaulting();
	bool IsClimbing() const;
	bool CanStartVaulting();
#pragma endregion

#pragma region Delegates
	FOnStartClimbingDelegate OnStartClimbing;
	FOnStopClimbingDelegate OnStopClimbing;
#pragma endregion

protected:
#pragma region OverridenFunctions
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;
	virtual void PhysCustom(float deltaTime, int32 Iterations) override;
	virtual float GetMaxSpeed() const override;
	virtual float GetMaxAcceleration() const override;
	virtual FVector ConstrainAnimRootMotionVelocity(const FVector& RootMotionVelocity, const FVector& CurrentVelocity) const override;
#pragma endregion 

private:
#pragma region ClimbBPVariables
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Climbing", meta=(AllowPrivateAccess = true))
	TArray<TEnumAsByte<EObjectTypeQuery>> ClimbableSurfaceTraceTypes;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Climbing", meta=(AllowPrivateAccess = true))
	float ClimbCapsuleTraceRadius = 50.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Climbing", meta=(AllowPrivateAccess = true))
	float ClimbCapsuleTraceHalfHeight = 75.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Climbing", meta=(AllowPrivateAccess = true))
	float MaxBreakClimbDeceleration = 400.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Climbing", meta=(AllowPrivateAccess = true))
	float MaxClimbingSpeed = 100.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Climbing", meta=(AllowPrivateAccess = true))
	float MaxClimbingAcceleration = 300.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Climbing", meta=(AllowPrivateAccess = true))
	float ClimbDawnWalkableSurfaceOffset = 100.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Climbing", meta=(AllowPrivateAccess = true))
	float ClimbDawnLedgeOffset = 50.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Climbing", meta=(AllowPrivateAccess = true))
	UAnimMontage* IdleToCLimbMontage;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Climbing", meta=(AllowPrivateAccess = true))
	UAnimMontage* CLimbToTopMontage;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Climbing", meta=(AllowPrivateAccess = true))
	UAnimMontage* CLimbDownLedgeMontage;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Climbing", meta=(AllowPrivateAccess = true))
	UAnimMontage* VaultMontage;
#pragma endregion

#pragma region ClimbVariables
	TArray<FHitResult> ClimbableSurfacesTracedResults;
	FVector CurrentClimbableSurfaceLocation;
	FVector CurrentClimbableSurfaceNormal;
	UPROPERTY()
	UAnimInstance* OwningAnimInstance;
	UPROPERTY()
	AShooterCharacter* OwningPlayerCharacter;
#pragma endregion
	
#pragma region ClimbTraces
	TArray<FHitResult> DoCapsuleTraceMultiByObject(const FVector& Start, const FVector& End, bool bShouldShowCapsuleDebug = false,
		bool bDrawPersistantShapes = false);
	FHitResult DoLineTraceSingleByObject(const FVector& Start, const FVector& End, bool bShouldShowCapsuleDebug = false,
		bool bDrawPersistantShapes = false);
#pragma endregion
	
#pragma region ClimbCore
	bool TraceClimbableSurfaces();
	FHitResult TraceFromEyeHeight(float TraceDistance, float TraceStartOffset = 0.0f);
	bool CanStartClimbing();
	bool CanClimbDownLedge();
	bool CheckShouldStopClimbing();
	bool CheckHasReachedFloor();
	bool CheckHasReachedLedge();
	bool CanStartVaulting(FVector& OutVaultStartPosition, FVector& OutVaultLandPosition);
	
	void StartClimbing();
	void StopClimbing();
	void PhysClimb(float deltaTime, int32 Iterations);
	void ProcessClimbableSurfaceInfo();
	void SnapMovementToClimbableSurfaces(float DeltaTime);
	void PlayClimbMontage(UAnimMontage* MontageToPlay);
	void SetMotionWarpTarget(const FName& InWarpTargetName, const FVector& InTargetPosition);
	FQuat GetClimbRotation(float DeltaTime);

	UFUNCTION()
	void OnClimbMontageEnded(UAnimMontage* Montage, bool bInterrupted);
#pragma endregion 
};

