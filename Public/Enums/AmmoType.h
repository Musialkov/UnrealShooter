#pragma once

UENUM(BlueprintType)
enum class EAmmoType : uint8
{
	EAT_Rifle UMETA(displayName = "RifleAmmo"),
	EAT_Pistol UMETA(displayName = "PistolAmmo"),
};