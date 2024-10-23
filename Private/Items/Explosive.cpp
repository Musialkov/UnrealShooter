#include "Items/Explosive.h"

#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

AExplosive::AExplosive()
{
	PrimaryActorTick.bCanEverTick = true;

	Damage = 20.f;

	ExplosiveMesh = CreateDefaultSubobject<UStaticMeshComponent>("Mesh");
	SetRootComponent(ExplosiveMesh);

	SphereComponent = CreateDefaultSubobject<USphereComponent>("Sphere");
	SphereComponent->SetupAttachment(RootComponent);

}

void AExplosive::BeginPlay()
{
	Super::BeginPlay();
	
}

void AExplosive::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AExplosive::Hit_Implementation(FHitResult HitResult, AActor* DamageCauser, AController* ControllerInstigator)
{
	if(ExplosiveSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ExplosiveSound, GetActorLocation());
	}

	if(ExplosiveParticle)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosiveParticle, HitResult.Location);
	}

	TArray<AActor*> OverlappingActors;
	GetOverlappingActors(OverlappingActors, ACharacter::StaticClass());

	for(auto Actor : OverlappingActors)
	{
		UGameplayStatics::ApplyDamage(Actor, Damage, ControllerInstigator, DamageCauser, UDamageType::StaticClass());
	}
	Destroy();
}

