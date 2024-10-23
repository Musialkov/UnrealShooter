#include "Items/Weapon.h"

#include "DataTable/WeaponDataTable.h"
#include "Enums/AmmoType.h"

AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = true;
	
	ThrowWeaponTime = 1.0f;
	bIsFalling = false;
	AmmoCount = 12;
	MagazineCapacity = 12;
	AmmoType = EAmmoType::EAT_Pistol;
	WeaponType = EWeaponType::EWT_Rifle;
	SlideDisplacement = 0;
	SlideDisplacementTime = 1.0f;
	MaxSlideDisplacement = 4.0f;
	MaxRecoilRotation = 20;
	bAutomatic = true;
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if(GetItemState() == EItemState::EIS_Falling && bIsFalling)
	{
		const FRotator MeshRotation{0.f, GetItemMesh()->GetComponentRotation().Yaw, 0.f};
		GetItemMesh()->SetWorldRotation(MeshRotation, false, nullptr, ETeleportType::TeleportPhysics);
	}

	UpdateSlideDisplacement();
}

void AWeapon::DecrementAmmo()
{
	if(AmmoCount - 1 <= 0) AmmoCount = 0;
	else AmmoCount--;
}

void AWeapon::ReloadAmmo(int32 Amount)
{
	AmmoCount += Amount;
}

void AWeapon::ThrowWeapon()
{
	FRotator MeshRotation{0.f, GetItemMesh()->GetComponentRotation().Yaw, 0.f};
	GetItemMesh()->SetWorldRotation(MeshRotation, false, nullptr, ETeleportType::TeleportPhysics);

	const FVector MeshVectorForward {GetItemMesh()->GetForwardVector()};
	const FVector MeshVectorRight {GetItemMesh()->GetRightVector()};

	FVector ImpulseDirection = MeshVectorRight.RotateAngleAxis(-20.f, MeshVectorForward);

	float Random = FMath::FRandRange(0.f, 30.0f);
	ImpulseDirection = ImpulseDirection.RotateAngleAxis(Random, FVector(0.f, 0.f, 1.f));
	ImpulseDirection *= 4'000.f;

	GetItemMesh()->AddImpulse(ImpulseDirection);

	bIsFalling = true;
	GetWorldTimerManager().SetTimer(ThrowWeaponTimer, this, &AWeapon::StopFalling, ThrowWeaponTime);
}

void AWeapon::StartSlideTimer()
{
	bMovingSlide = true;
	GetWorldTimerManager().SetTimer(SlideTimer, this, &AWeapon::FinishMovingSlide, SlideDisplacementTime);
}

bool AWeapon::ClipIsFull() const
{
	return AmmoCount >= MagazineCapacity;
}

void AWeapon::StopFalling()
{
	bIsFalling = false;
	SetItemState(EItemState::EIS_Pickup);
}

void AWeapon::FinishMovingSlide()
{
	bMovingSlide = false;
}

void AWeapon::UpdateSlideDisplacement()
{
	if(!SlideDisplacementCurve || !bMovingSlide) return;
	
	const float ElapsedTime = GetWorldTimerManager().GetTimerElapsed(SlideTimer);
	const float CurveValue = SlideDisplacementCurve->GetFloatValue(ElapsedTime);
	SlideDisplacement = CurveValue * MaxSlideDisplacement;
	RecoilRotation = CurveValue * MaxRecoilRotation;
}

void AWeapon::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
	if(!WeaponDataTable) return;

	const FWeaponDataTable* WeaponDataRow = nullptr;
	switch (WeaponType)
	{
	case EWeaponType::EWT_Rifle:
		WeaponDataRow = WeaponDataTable->FindRow<FWeaponDataTable>(FName("Rifle"), "");
		break;
	case EWeaponType::EWT_Pistol:
		WeaponDataRow = WeaponDataTable->FindRow<FWeaponDataTable>(FName("Pistol"), "");
		break;
	}

	if(WeaponDataRow)
	{
		AmmoType = WeaponDataRow->AmmoType;
		AmmoCount = WeaponDataRow->WeaponAmmo;
		AutoFireRate = WeaponDataRow->AutoFireRate;
		MagazineCapacity = WeaponDataRow->MagazineCapacity;
		BoneToHide = WeaponDataRow->BoneToHide;
		CrosshairMiddle = WeaponDataRow->CrosshairMiddle;
		CrosshairRight = WeaponDataRow->CrosshairRight;
		CrosshairLeft = WeaponDataRow->CrosshairLeft;
		CrosshairTop = WeaponDataRow->CrosshairTop;
		CrosshairBottom = WeaponDataRow->CrosshairBottom;
		FireSound = WeaponDataRow->FireSound;
		MuzzleFlash = WeaponDataRow->MuzzleFlash;
		ReloadMontageSection = WeaponDataRow->ReloadMontageSection;
		bAutomatic = WeaponDataRow->bAutomatic;
		Damage = WeaponDataRow->Damage;
		HeadShotDamage = WeaponDataRow->HeadShotDamage;
		SetPickupSound(WeaponDataRow->PickupSound);
		GetItemMesh()->SetSkeletalMesh(WeaponDataRow->ItemMesh);
		SetItemName(WeaponDataRow->ItemName);
		SetItemIcon(WeaponDataRow->InventoryIcon);
		GetItemMesh()->SetAnimInstanceClass(WeaponDataRow->AnimBP);
	}
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	if(BoneToHide == FName("")) return;

	GetItemMesh()->HideBoneByName(BoneToHide, PBO_None);
}
