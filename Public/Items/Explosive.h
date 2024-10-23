#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/Damageable.h"
#include "Explosive.generated.h"

UCLASS()
class SHOOTERV2_API AExplosive : public AActor, public IDamageable
{
	GENERATED_BODY()
	
public:	
	AExplosive();
	virtual void Tick(float DeltaTime) override;
	virtual void Hit_Implementation(FHitResult HitResult, AActor* DamageCauser, AController* ControllerInstigator) override;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Combat", meta=(AllowPrivateAccess = true))
	float Damage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Combat", meta=(AllowPrivateAccess = true))
	UParticleSystem* ExplosiveParticle;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Combat", meta=(AllowPrivateAccess = true))
	class USoundCue* ExplosiveSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Combat", meta=(AllowPrivateAccess = true))
	class USphereComponent* SphereComponent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Combat", meta=(AllowPrivateAccess = true))
	UStaticMeshComponent* ExplosiveMesh;
};
