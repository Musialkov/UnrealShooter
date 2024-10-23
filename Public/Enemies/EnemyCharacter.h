// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/Damageable.h"
#include "EnemyCharacter.generated.h"

enum class ECombatState : uint8;
class USphereComponent;
class UBehaviorTree;
class UBoxComponent;
class UHealthComponent;
class UWidgetComponent;

UCLASS()
class SHOOTERV2_API AEnemyCharacter : public ACharacter, public IDamageable
{
	GENERATED_BODY()

public:
	AEnemyCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void Hit_Implementation(FHitResult HitResult, AActor* DamageCauser, AController* ControllerInstigator) override;
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	
	void SetIsStunned(const bool IsStunned);
	void PlayAttackMontage(float PlayRate = 1.0f);
	void ActivateWeapon(bool RightHand);
	void DeactivateWeapon(bool RightHand);
	void FinishDeath();

	FORCEINLINE FName GetHeadBone() const {return HeadBone;}
	FORCEINLINE UBehaviorTree* GetBehaviourTree() const {return BehaviorTree;}
	FORCEINLINE UHealthComponent* GetHealthComponent() const {return HealthComponent;}
	FORCEINLINE UWidgetComponent* GetHealthBarEnemy() const {return HealthBarEnemy;}

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UHealthComponent* HealthComponent;
	
	virtual void BeginPlay() override;
	
	UFUNCTION()
	void AgroSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void CombatRangeOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void CombatRangeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 otherBodyIndex);
	UFUNCTION()
	void OnLeftWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnRightWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void PlayHitMontage(FName Section, float PlayRate = 1.0f);
	FName GetRandomAttackSection();
	void ResetHitReactTimer();
	void ShowHealthBar();
	void HideHealthBar();
	void ApplyDamage(AActor* OtherActor);
	void DestroyCharacter();
	
	UFUNCTION()
	void Die();

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Combat", meta=(AllowPrivateAccess = true))
	UParticleSystem* ImpactParticles;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Combat", meta=(AllowPrivateAccess = true))
	class USoundCue* ImpactSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Combat", meta=(AllowPrivateAccess = true))
	float BaseDamage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Combat", meta=(AllowPrivateAccess = true))
	FName HeadBone;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Combat", meta=(AllowPrivateAccess = true))
	UAnimMontage* HitMontage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Combat", meta=(AllowPrivateAccess = true))
	UAnimMontage* AttackMontage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Combat", meta=(AllowPrivateAccess = true))
	UAnimMontage* DeathMontage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta=(AllowPrivateAccess = true))
	TArray<FName> AttackMontageSections;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Combat", meta=(AllowPrivateAccess = true, MakeEditWidget = true))
	USphereComponent* AgroSphere;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Combat", meta=(AllowPrivateAccess = true, MakeEditWidget = true))
	USphereComponent* CombatRangeSphere;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category= "Combat", meta=(AllowPrivateAccess = true))
	bool bIsStunned;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category= "Combat", meta=(AllowPrivateAccess = true))
	bool bInAttackRange;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Combat", meta=(AllowPrivateAccess = true))
	float StunChance;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Combat", meta=(AllowPrivateAccess = true))
	UBoxComponent* LeftWeaponCollision;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Combat", meta=(AllowPrivateAccess = true))
	UBoxComponent* RightWeaponCollision;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Combat", meta=(AllowPrivateAccess = true))
	USoundCue* MeleeImpactSound;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI", meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* HealthBarEnemy;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "AI", meta=(AllowPrivateAccess = true))
	UBehaviorTree* BehaviorTree;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "AI", meta=(AllowPrivateAccess = true, MakeEditWidget = true))
	FVector PatrolPoint;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "AI", meta=(AllowPrivateAccess = true, MakeEditWidget = true))
	FVector PatrolPoint2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Combat", meta=(AllowPrivateAccess = true))
	float HealthBarDisplayTime;
	FTimerHandle HealthBarTimer;
	FTimerHandle HitTimer;
	FTimerHandle DeathHandle;
	bool CanHitReact;
	float DeathTime;

	ECombatState CombatState;
	class AEnemyAIController* EnemyAIController;
};
