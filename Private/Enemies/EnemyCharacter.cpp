#include "Enemies/EnemyCharacter.h"

#include "AI/EnemyAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Blueprint/UserWidget.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/HealthComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Player/ShooterCharacter.h"
#include "Sound/SoundCue.h"

AEnemyCharacter::AEnemyCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	
	HealthBarDisplayTime = 4.f;
	CanHitReact = true;
	bIsStunned = false;
	StunChance = .5f;
	BaseDamage = 20.f;
	CombatState = ECombatState::ECS_Unoccupied;
	DeathTime = 5.f;

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	
	AgroSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AgroSphere"));
	AgroSphere->SetupAttachment(GetRootComponent());

	CombatRangeSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CombatRangeSphere"));
	CombatRangeSphere->SetupAttachment(GetRootComponent());

	HealthBarEnemy = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBar"));
	HealthBarEnemy->SetupAttachment(RootComponent);

	LeftWeaponCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftWeaponBox"));
	LeftWeaponCollision->SetupAttachment(GetMesh(), "LeftWeaponSocket");
	RightWeaponCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("RightWeaponBox"));
	RightWeaponCollision->SetupAttachment(GetMesh(), "RightWeaponSocket");
}

void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	
	LeftWeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LeftWeaponCollision->SetCollisionObjectType(ECC_WorldDynamic);
	LeftWeaponCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	LeftWeaponCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	RightWeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RightWeaponCollision->SetCollisionObjectType(ECC_WorldDynamic);
	RightWeaponCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	RightWeaponCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	
	AgroSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemyCharacter::AgroSphereOverlap);
	CombatRangeSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemyCharacter::CombatRangeOverlap);
	CombatRangeSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemyCharacter::CombatRangeEndOverlap);
	LeftWeaponCollision->OnComponentBeginOverlap.AddDynamic(this, &AEnemyCharacter::OnLeftWeaponOverlap);
	RightWeaponCollision->OnComponentBeginOverlap.AddDynamic(this, &AEnemyCharacter::OnRightWeaponOverlap);
	
	EnemyAIController = Cast<AEnemyAIController>(GetController());
	const FVector WorldPatrolPoint = UKismetMathLibrary::TransformLocation(GetActorTransform(), PatrolPoint);
	const FVector WorldPatrolPoint2 = UKismetMathLibrary::TransformLocation(GetActorTransform(), PatrolPoint2);
	//DrawDebugSphere(GetWorld(), WorldPatrolPoint, 25.f, 12, FColor::Red, true);
	//DrawDebugSphere(GetWorld(), WorldPatrolPoint2, 25.f, 12, FColor::Red, true);
	
	if(EnemyAIController)
	{
		EnemyAIController->GetBlackboardComponent()->SetValueAsVector(TEXT("PatrolPoint"), WorldPatrolPoint);
		EnemyAIController->GetBlackboardComponent()->SetValueAsVector(TEXT("PatrolPoint2"), WorldPatrolPoint2);
		EnemyAIController->RunBehaviorTree(BehaviorTree);
	}

	if(HealthComponent)
	{
		HealthComponent->OnDie.AddDynamic(this, &AEnemyCharacter::Die);
	}

	if (HealthBarEnemy)
	{
		UUserWidget* HealthBarWidget = Cast<UUserWidget>(HealthBarEnemy->GetUserWidgetObject());
		if (HealthBarWidget)
		{
			HealthBarWidget->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void AEnemyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AEnemyCharacter::SetIsStunned(const bool IsStunned)
{
	bIsStunned = IsStunned;
	if(EnemyAIController)
	{
		EnemyAIController->GetBlackboardComponent()->SetValueAsBool("Stunned", bIsStunned);
	}
}

void AEnemyCharacter::PlayHitMontage(const FName Section, const float PlayRate)
{
	if(!CanHitReact) return;
	
	PlayAnimMontage(HitMontage, PlayRate, Section);

	CanHitReact = false;
	const float HiTReactTime = FMath::FRandRange(0.4f, 0.8f);
	GetWorldTimerManager().SetTimer(HitTimer, this, &AEnemyCharacter::ResetHitReactTimer, HiTReactTime);
}

void AEnemyCharacter::PlayAttackMontage(float PlayRate)
{
	const auto Section = GetRandomAttackSection();
	PlayAnimMontage(AttackMontage, PlayRate, Section);
}

FName AEnemyCharacter::GetRandomAttackSection()
{
	const int SectionIndex = FMath::RandRange(0, AttackMontageSections.Num() - 1);
	return AttackMontageSections[SectionIndex];
}

void AEnemyCharacter::ActivateWeapon(bool RightHand)
{
	if(RightHand)
	{
		RightWeaponCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	else
	{
		LeftWeaponCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
}

void AEnemyCharacter::DeactivateWeapon(bool RightHand)
{
	if(RightHand)
	{
		RightWeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	else
	{
		LeftWeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void AEnemyCharacter::ResetHitReactTimer()
{
	CanHitReact = true;
}

void AEnemyCharacter::ShowHealthBar()
{
	if (HealthBarEnemy)
	{
		GetWorldTimerManager().ClearTimer(HealthBarTimer);
		GetWorldTimerManager().SetTimer(HealthBarTimer, this, &AEnemyCharacter::HideHealthBar, HealthBarDisplayTime);
		UUserWidget* HealthBarWidget = Cast<UUserWidget>(HealthBarEnemy->GetUserWidgetObject());
		if (HealthBarWidget)
		{
			HealthBarWidget->SetVisibility(ESlateVisibility::Visible);
		}
	}
}

void AEnemyCharacter::HideHealthBar()
{
	if (HealthBarEnemy)
	{
		UUserWidget* HealthBarWidget = Cast<UUserWidget>(HealthBarEnemy->GetUserWidgetObject());
		if (HealthBarWidget)
		{
			HealthBarWidget->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void AEnemyCharacter::AgroSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(!OtherActor) return;
	
	auto Character = Cast<AShooterCharacter>(OtherActor);
	if(Character && EnemyAIController && EnemyAIController->GetBlackboardComponent())
	{
		EnemyAIController->GetBlackboardComponent()->SetValueAsObject("Target", Character);
	}
}

void AEnemyCharacter::CombatRangeOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(!OtherActor) return;
	auto Character = Cast<AShooterCharacter>(OtherActor);
	
	bInAttackRange = true;
	if(EnemyAIController && Character)
	{
		EnemyAIController->GetBlackboardComponent()->SetValueAsBool("InAttackRange", bInAttackRange);
	}
}

void AEnemyCharacter::CombatRangeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 otherBodyIndex)
{
	if(!OtherActor) return;
	auto Character = Cast<AShooterCharacter>(OtherActor);
	
	bInAttackRange = false;
	if(EnemyAIController && Character)
	{
		EnemyAIController->GetBlackboardComponent()->SetValueAsBool("InAttackRange", bInAttackRange);
	}
}

void AEnemyCharacter::ApplyDamage(AActor* OtherActor)
{
	auto Character = Cast<AShooterCharacter>(OtherActor);
	if(Character)
	{
		UGameplayStatics::ApplyDamage(Character, BaseDamage, EnemyAIController, this, UDamageType::StaticClass());
	}
	if(MeleeImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, MeleeImpactSound, GetActorLocation());
	}
}

void AEnemyCharacter::DestroyCharacter()
{
	Destroy();
}

void AEnemyCharacter::OnLeftWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                          UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(!OtherActor) return;
	ApplyDamage(OtherActor);
}

void AEnemyCharacter::OnRightWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(!OtherActor) return;
	ApplyDamage(OtherActor);
}


void AEnemyCharacter::Die()
{
	if(CombatState == ECombatState::ECS_Dead) return;
	
	HideHealthBar();
	CombatState = ECombatState::ECS_Dead;
	PlayAnimMontage(DeathMontage, 1, "DeathFront");
	if(EnemyAIController)
	{
		EnemyAIController->GetBlackboardComponent()->SetValueAsBool("IsDead", true);
	}
	EnemyAIController->StopMovement();
}

void AEnemyCharacter::FinishDeath()
{
	GetMesh()->bPauseAnims = true;
	DetachFromControllerPendingDestroy();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetWorldTimerManager().SetTimer(DeathHandle, this, &AEnemyCharacter::DestroyCharacter, DeathTime);
}

void AEnemyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AEnemyCharacter::Hit_Implementation(FHitResult HitResult, AActor* DamageCauser, AController* ControllerInstigator)
{
	if(ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}

	if(ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, HitResult.Location);
	}
}

float AEnemyCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
	AActor* DamageCauser)
{
	if(CombatState == ECombatState::ECS_Dead) return 0;
	
	if(EnemyAIController)
	{
		EnemyAIController->GetBlackboardComponent()->SetValueAsObject("Target", DamageCauser);
	}
	HealthComponent->TakeDamage(DamageAmount);
	
	if(CombatState == ECombatState::ECS_Dead) return 0;
	ShowHealthBar();
	const float RandomStunChance = FMath::RandRange(0.f, 1.f);
	if(StunChance <= RandomStunChance)
	{
		PlayHitMontage(FName("HitFront"));
		SetIsStunned(true);
	}

	return DamageAmount;
}

