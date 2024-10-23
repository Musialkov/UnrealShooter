#pragma once

UENUM(BlueprintType)
enum class ECombatState : uint8
{
	ECS_Unoccupied UMETA(displayName = "Unoccupied"),
	ECS_Climbing UMETA(displayName = "Climbing"),
	ECS_FireTimerInProgress UMETA(displayName = "FireTimerInProgress"),
	ECS_Reloading UMETA(displayName = "Reloading"),
	ECS_Equipping UMETA(displayName = "Equipping"),
	ECS_Stunned UMETA(displayName = "Stunned"),
	ECS_Dead UMETA(displayName = "Dead"),
};
