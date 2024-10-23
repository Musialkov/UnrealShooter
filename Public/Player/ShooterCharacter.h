#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"
#include "Enums/AmmoType.h"
#include "Enums/CombatState.h"
#include "Items/Weapon.h"
#include "ShooterCharacter.generated.h"

class UHealthComponent;
class UCustomMovementComponent;
class UMotionWarpingComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEquipItemDelegate, int32, CurrentSlotIndex, int32, NextSlotIndex)

UCLASS()
class SHOOTERV2_API AShooterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AShooterCharacter(const FObjectInitializer& ObjectInitializer);
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	void IncrementOverlappedItemCount(int Amount);
	bool IsInventoryFull() const;
	void FinishStun();

	UFUNCTION(BlueprintCallable)
	float GetCrosshairSpreadMultiplier() const {return CrossHairSpreadMultiplier;}

	FORCEINLINE USpringArmComponent* GetSpringArmComponent() const {return SpringArmComponent;}
	FORCEINLINE UCameraComponent* GetCameraComponent() const {return Camera;}
	FORCEINLINE UCustomMovementComponent* GetCustomMovementComponent() const {return CustomMovementComponent;}
	FORCEINLINE UMotionWarpingComponent* GetMotionWarpingComponent() const {return MotionWarpingComponent;}
	FORCEINLINE bool IsCrouching() const {return bIsCrouching;}
	FORCEINLINE bool IsAiming() const {return bAiming;}
	FORCEINLINE int GetOverlappedItemCount() const {return OverlappedItemCount;}
	FORCEINLINE ECombatState GetCombatState() const {return CombatState;}
	FORCEINLINE AWeapon* GetEquippedWeapon() const {return EquippedWeapon;}

protected:
#pragma region Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UHealthComponent* HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCustomMovementComponent* CustomMovementComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UMotionWarpingComponent* MotionWarpingComponent;
#pragma endregion 
	
	virtual void BeginPlay() override;
	virtual void Jump() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	void MoveForward(float Value);
	void MoveRight(float Value);
	void HandleGroundMovementInput(float Value, bool MovesInForwardAxis);
	void HandleClimbMovementInput(float Value, bool MovesInForwardAxis);

	void TurnAtRate(float Rate);
	void LookUpAtRate(float Rate);
	void Turn(float Value);
	void LookUp(float Value);

	void FireWeapon();
	void PlayFireSound() const;
	void SendBullet();
	void PlayGunFireMontage();
	void AimingButtonPressed();
	void AimingButtonReleased();
	void ZoomCamera(float DeltaTime);
	
	bool GetBeamEndLocation(const FVector& MuzzleSocketLocation, FHitResult& OutHitResult) const;
	bool TraceUnderCrosshairs(FHitResult& OutHitResult, FVector& OutHitLocation) const;
	void TraceForItems();

	void EquipDefaultWeapon();
	void EquipWeapon(AWeapon* WeaponToEquip);
	UFUNCTION()
	void OnStartClimbing();
	UFUNCTION()
	void OnStopClimbing();
	void DropWeapon();
	void SwapWeapon(AWeapon* WeaponToSwap);

	void SelectButtonPressed();
	void SelectButtonReleased();

	void CalculateCrosshairSpread(float DeltaTime);
	void CalculateCrosshairSpreadVelocity();
	void CalculateCrosshairInAirFactor(float DeltaTime);
	void CalculateCrosshairSpreadAiming(float DeltaTime);
	void CalculateCrosshairShootingFactor(float DeltaTime);

	void OnStartClimbingAction();

	UFUNCTION()
	void StartCrosshairBulletFire();
	UFUNCTION()
	void FinishCrosshairBulletFire();
	UFUNCTION()
	void AutoFireReset();

	void FireButtonPressed();
	void FireButtonReleased();
	void StartFireTimer();

	void ReloadButtonPressed();
	void ReloadWeapon();

	UFUNCTION()
	void Stun();

	void InitializeAmmoMap();
	bool WeaponHasAmmo() const;
	bool CarryingAmmo();
	void PickupAmmo(class AAmmo* Ammo);

	void CrouchButtonPressed();

	void OneKeyPressed();
	void TwoKeyPressed();
	void ThreeKeyPressed();
	void FourKeyPressed();
	void FiveKeyPressed();
	void SixKeyPressed();

	void ExchangeInventoryItems(int CurrentItemIndex, int NewItemIndex);

	UFUNCTION()
	void Die();
	
	UFUNCTION(BlueprintCallable)
	void FinishDie();

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Camera, meta=(AllowPrivateAccess = "true"))
	float ControllerTurnRate;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Camera, meta=(AllowPrivateAccess = "true"))
	float ControllerLookUpRate;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Camera, meta=(AllowPrivateAccess = "true"))
	float ControllerAimingRateFactor;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Camera, meta=(AllowPrivateAccess = "true"),
		meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseTurnRate;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Camera, meta=(AllowPrivateAccess = "true"),
		meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseLookUpRate;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Camera, meta=(AllowPrivateAccess = "true"))
	float MouseAimingRateFactor;
	UPROPERTY(EditAnywhere, Category = Camera, meta=(AllowPrivateAccess = "true"))
	float CameraZoomedFOV;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera, meta=(AllowPrivateAccess = "true"))
	float ZoomInterpolationSpeed;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta=(AllowPrivateAccess = "true"))
	USpringArmComponent* SpringArmComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta=(AllowPrivateAccess = "true"))
	UCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta=(AllowPrivateAccess = "true"))
	bool bIsCrouching;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta=(AllowPrivateAccess = "true"))
	float RegularSpeed;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta=(AllowPrivateAccess = "true"))
	float CrouchSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta=(AllowPrivateAccess = "true"))
	float RegularCapsuleHalfHeight;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta=(AllowPrivateAccess = "true"))
	float CrouchingCapsuleHalfHeight;

	float CameraDefaultFOV;
	float CameraCurrentFOV;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta=(AllowPrivateAccess = "true"))
	bool bAiming;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta=(AllowPrivateAccess = "true"))
	UParticleSystem* ImpactParticle;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta=(AllowPrivateAccess = "true"))
	UParticleSystem* BeamParticle;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta=(AllowPrivateAccess = "true"))
	UAnimMontage* HipFireMontage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta=(AllowPrivateAccess = "true"))
	UAnimMontage* ReloadMontage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta=(AllowPrivateAccess = "true"))
	UAnimMontage* EquipMontage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta=(AllowPrivateAccess = "true"))
	UAnimMontage* HitReactMontage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta=(AllowPrivateAccess = "true"))
	UAnimMontage* DeathMontage;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta=(AllowPrivateAccess = "true"))
	AWeapon* EquippedWeapon;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Combat, meta=(AllowPrivateAccess = "true"))
	TSubclassOf<AWeapon> DefaultWeapon;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category= Combat, meta=(AllowPrivateAccess = "true"))
	ECombatState CombatState;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= Combat, meta=(AllowPrivateAccess = "true"))
	float StunChance;

	UFUNCTION(BlueprintCallable)
	void FinishReloading();

	UFUNCTION(BlueprintCallable)
	void FinishEquipping();

	bool bIsFireButtonPressed;
	bool bShouldFire;
	bool bFiringBullet;
	float ShootTimeDuration;
	
	FTimerHandle AutoFireTimer;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta=(AllowPrivateAccess = "true"))
	float CrossHairSpreadMultiplier;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta=(AllowPrivateAccess = "true"))
	float CrosshairVelocityFactor;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta=(AllowPrivateAccess = "true"))
	float CrosshairInAirFactor;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta=(AllowPrivateAccess = "true"))
	float CrosshairAimFactor;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta=(AllowPrivateAccess = "true"))
	float CrosshairShootingFactor;
	
	FTimerHandle CrosshairShootTimer;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Items, meta=(AllowPrivateAccess = "true"))
	AItem* LastTracedItem;

	bool bShouldTraceForItems;
	int OverlappedItemCount;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Ammo, meta=(AllowPrivateAccess = "true"))
	TMap<EAmmoType, int32> AmmoMap;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Ammo, meta=(AllowPrivateAccess = "true"))
	int StartingPistolAmmo;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Ammo, meta=(AllowPrivateAccess = "true"))
	int StartingRifleAmmo;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Inventory, meta=(AllowPrivateAccess = "true"))
	TArray<AItem*> Inventory;
	const int32 INVENTORY_CAPACITY = 6;

	//delegate by wysłać informację o slot index podczas ekwipowania
	UPROPERTY(BlueprintAssignable, Category = "Delegates", meta=(AllowPrivateAccess = "true"))
	FEquipItemDelegate EquipItemDelegate;
};

