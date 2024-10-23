#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "Engine/DataTable.h"
#include "Enums/AmmoType.h"
#include "Enums/WeaponType.h"
#include "Weapon.generated.h"

UCLASS()
class SHOOTERV2_API AWeapon : public AItem
{
	GENERATED_BODY()

public:
	AWeapon();

	virtual void Tick(float DeltaTime) override;

	FORCEINLINE int32 GetAmmoCount() const {return AmmoCount;}
	FORCEINLINE int32 GetMagazineCapacity() const {return MagazineCapacity;}
	FORCEINLINE EAmmoType GetAmmoType() const {return AmmoType;}
	FORCEINLINE EWeaponType GetWeaponType() const {return WeaponType;}
	FORCEINLINE FName GetReloadMontageSection() const {return ReloadMontageSection;}
	FORCEINLINE USoundCue* GetFireSound() const {return FireSound;}
	FORCEINLINE UParticleSystem* GetMuzzleFlash() const {return MuzzleFlash;}
	FORCEINLINE float GetAutoFireRate() const {return AutoFireRate;}
	FORCEINLINE bool GetAutomatic() const {return bAutomatic;}
	FORCEINLINE float GetDamage() const {return Damage;}
	FORCEINLINE float GetHeadShotDamage() const {return HeadShotDamage;}
	
	void DecrementAmmo();
	void ReloadAmmo(int32 Amount);
	void ThrowWeapon();
	void StartSlideTimer();

	bool ClipIsFull() const;

protected:
	void StopFalling();
	void FinishMovingSlide();
	void UpdateSlideDisplacement();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	
private:
	FTimerHandle ThrowWeaponTimer;
	FTimerHandle SlideTimer;
	float ThrowWeaponTime;
	float SlideDisplacementTime;
	bool bIsFalling;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Weapon Properties", meta=(AllowPrivateAccess = "true"))
	int32 AmmoCount;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Weapon Properties", meta=(AllowPrivateAccess = "true"))
	int32 MagazineCapacity;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Weapon Properties", meta=(AllowPrivateAccess = "true"))
	float AutoFireRate;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Weapon Properties", meta=(AllowPrivateAccess = "true"))
	EAmmoType AmmoType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon Properties", meta=(AllowPrivateAccess = "true"))
	EWeaponType WeaponType;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Weapon Properties", meta=(AllowPrivateAccess = "true"))
	USoundCue* FireSound;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Weapon Properties", meta=(AllowPrivateAccess = "true"))
	UParticleSystem* MuzzleFlash;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Weapon Properties", meta=(AllowPrivateAccess = "true"))
	FName ReloadMontageSection;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Weapon Properties", meta=(AllowPrivateAccess = "true"))
	FName BoneToHide;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon Properties", meta=(AllowPrivateAccess = "true"))
	UTexture2D* CrosshairMiddle;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon Properties", meta=(AllowPrivateAccess = "true"))
	UTexture2D* CrosshairRight;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon Properties", meta=(AllowPrivateAccess = "true"))
	UTexture2D* CrosshairLeft;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon Properties", meta=(AllowPrivateAccess = "true"))
	UTexture2D* CrosshairTop;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon Properties", meta=(AllowPrivateAccess = "true"))
	UTexture2D* CrosshairBottom;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon Properties", meta=(AllowPrivateAccess = "true"))
	bool bAutomatic;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon Properties", meta=(AllowPrivateAccess = "true"))
	float Damage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon Properties", meta=(AllowPrivateAccess = "true"))
	float HeadShotDamage;

	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Slide", meta=(AllowPrivateAccess = "true"))
	float SlideDisplacement;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Slide", meta=(AllowPrivateAccess = "true"))
	UCurveFloat* SlideDisplacementCurve;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Slide", meta=(AllowPrivateAccess = "true"))
	bool bMovingSlide;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Slide", meta=(AllowPrivateAccess = "true"))
	float MaxSlideDisplacement;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Slide", meta=(AllowPrivateAccess = "true"))
	float MaxRecoilRotation;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Slide", meta=(AllowPrivateAccess = "true"))
	float RecoilRotation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Data Table", meta=(AllowPrivateAccess = "true"))
	UDataTable* WeaponDataTable;
};
