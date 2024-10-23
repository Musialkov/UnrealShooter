#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDieDelegate)
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHitDelegate)

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SHOOTERV2_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UHealthComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	FORCEINLINE bool IsDead() const { return CurrentHealth <= 0; }
	
	void TakeDamage(float DamageAmount);
	
	FOnDieDelegate OnDie;
	FOnHitDelegate OnHit;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health", meta=(AllowPrivateAccess = true))
	float MaxHealth;

	UPROPERTY(BlueprintReadOnly, Category = "Health", meta=(AllowPrivateAccess = true))
	float CurrentHealth;
};
