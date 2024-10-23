#include "Player/ShooterCharacter.h"

#include "AI/EnemyAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Items/Ammo.h"
#include "Items/Weapon.h"
#include "Enums/AmmoType.h"

#include "Components/CapsuleComponent.h"
#include "Components/HealthComponent.h"
#include "Components/WidgetComponent.h"
#include "Enemies/EnemyCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/CustomMovementComponent.h"
#include "Helpers/DebugHelper.h"
#include "Interfaces/Damageable.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "MotionWarpingComponent.h"

AShooterCharacter::AShooterCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UCustomMovementComponent>(CharacterMovementComponentName))
{
	bAiming = false;
	PrimaryActorTick.bCanEverTick = true;
	ZoomInterpolationSpeed = 1.f;
	ControllerAimingRateFactor = 2;
	ShootTimeDuration = 0.05f;
	bShouldFire = true;
	bIsFireButtonPressed = false;
	bShouldTraceForItems = false;

	StartingPistolAmmo = 30;
	StartingRifleAmmo = 90;
	CombatState = ECombatState::ECS_Unoccupied;

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	CustomMovementComponent = Cast<UCustomMovementComponent>(GetCharacterMovement());

	MotionWarpingComponent = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarpingComponent"));

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComponent->SetupAttachment(RootComponent);
	SpringArmComponent->TargetArmLength = 300.f;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArmComponent, USpringArmComponent::SocketName);
	SpringArmComponent->bUsePawnControlRotation = false;

	bIsCrouching = false;
	RegularSpeed = 650.f;
	CrouchSpeed = 400.f;
	RegularCapsuleHalfHeight = 94.f;
	CrouchingCapsuleHalfHeight = 62.f;
	StunChance = 0.25f;

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = true;

	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);
}

void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	if(Camera)
	{
		CameraDefaultFOV = GetCameraComponent()->FieldOfView;
		CameraCurrentFOV = CameraDefaultFOV;
	}

	if(HealthComponent)
	{
		HealthComponent->OnDie.AddDynamic(this, &AShooterCharacter::Die);
		HealthComponent->OnHit.AddDynamic(this, &AShooterCharacter::Stun);
	}

	if(CustomMovementComponent)
	{
		CustomMovementComponent->OnStartClimbing.AddDynamic(this, &AShooterCharacter::OnStartClimbing);
		CustomMovementComponent->OnStopClimbing.AddDynamic(this, &AShooterCharacter::OnStopClimbing);
	}

	EquipDefaultWeapon();
	Inventory.Add(EquippedWeapon);
	EquippedWeapon->SetSlotIndex(0);
	InitializeAmmoMap();
	GetCharacterMovement()->MaxWalkSpeed = RegularSpeed;
}

void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ZoomCamera(DeltaTime);
	CalculateCrosshairSpread(DeltaTime);
	TraceForItems();
}

void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AShooterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AShooterCharacter::MoveRight);
	PlayerInputComponent->BindAxis("TurnRate", this, &AShooterCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AShooterCharacter::LookUpAtRate);
	PlayerInputComponent->BindAxis("Turn", this, &AShooterCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AShooterCharacter::LookUp);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AShooterCharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	
	PlayerInputComponent->BindAction("FireButton", IE_Pressed, this, &AShooterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("FireButton", IE_Released, this, &AShooterCharacter::FireButtonReleased);

	PlayerInputComponent->BindAction("AimingButton", IE_Pressed, this, &AShooterCharacter::AimingButtonPressed);
	PlayerInputComponent->BindAction("AimingButton", IE_Released, this, &AShooterCharacter::AimingButtonReleased);

	PlayerInputComponent->BindAction("Select", IE_Pressed, this, &AShooterCharacter::SelectButtonPressed);
	PlayerInputComponent->BindAction("Select", IE_Released, this, &AShooterCharacter::SelectButtonReleased);

	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &AShooterCharacter::ReloadButtonPressed);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AShooterCharacter::CrouchButtonPressed);
	
	PlayerInputComponent->BindAction("StartClimbing", IE_Pressed, this, &AShooterCharacter::OnStartClimbingAction);
	
	PlayerInputComponent->BindAction("1", IE_Pressed, this, &AShooterCharacter::OneKeyPressed);
	PlayerInputComponent->BindAction("2", IE_Pressed, this, &AShooterCharacter::TwoKeyPressed);
	PlayerInputComponent->BindAction("3", IE_Pressed, this, &AShooterCharacter::ThreeKeyPressed);
	PlayerInputComponent->BindAction("4", IE_Pressed, this, &AShooterCharacter::FourKeyPressed);
	PlayerInputComponent->BindAction("5", IE_Pressed, this, &AShooterCharacter::FiveKeyPressed);
	PlayerInputComponent->BindAction("6", IE_Pressed, this, &AShooterCharacter::SixKeyPressed);
}

float AShooterCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
									AActor* DamageCauser)
{
	HealthComponent->TakeDamage(DamageAmount);
	if(HealthComponent->IsDead())
	{
		AEnemyAIController* EnemyAIController = Cast<AEnemyAIController>(EventInstigator);
		if(EnemyAIController)
		{
			EnemyAIController->GetBlackboardComponent()->SetValueAsObject("Target", nullptr);
		}
		Die();
	}
	return DamageAmount;
}

void AShooterCharacter::Jump()
{
	if(CombatState == ECombatState::ECS_Climbing) return;
	
	if(CustomMovementComponent->CanStartVaulting())
	{
		CustomMovementComponent->TryStartVaulting();
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = RegularSpeed;
		if(bIsCrouching)
		{
			bIsCrouching = false;
		}
		else
		{
			Super::Jump();
		}
	}
}

void AShooterCharacter::MoveForward(float Value)
{
	if(!CustomMovementComponent) return;
	if(CustomMovementComponent->IsClimbing())
	{
		HandleClimbMovementInput(Value, true);
	}
	else
	{
		HandleGroundMovementInput(Value, true);
	}
}

void AShooterCharacter::MoveRight(float Value)
{
	if(!CustomMovementComponent) return;
	if(CustomMovementComponent->IsClimbing())
	{
		HandleClimbMovementInput(Value, false);
	}
	else
	{
		HandleGroundMovementInput(Value, false);
	}
}

void AShooterCharacter::HandleGroundMovementInput(float Value, bool MovesInForwardAxis)
{
	if(Controller == nullptr) return;
	if(Value == 0.0f) return;

	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation = FRotator(0, Rotation.Yaw, 0);

	const EAxis::Type Axis = MovesInForwardAxis ? EAxis::X : EAxis::Y;
	const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(Axis);
	AddMovementInput(Direction, Value);
}

void AShooterCharacter::HandleClimbMovementInput(float Value, bool MovesInForwardAxis)
{
	if(Controller == nullptr) return;
	if(Value == 0.0f) return;

	const FVector ForwardDirection = FVector::CrossProduct(-CustomMovementComponent->GetClimbableSurfaceNormal(), GetActorRightVector());
	const FVector RightDirection = FVector::CrossProduct(-CustomMovementComponent->GetClimbableSurfaceNormal(), -GetActorUpVector());
	if(MovesInForwardAxis)
	{
		AddMovementInput(ForwardDirection, Value);
	}
	else
	{
		AddMovementInput(RightDirection, Value);
	}
}

void AShooterCharacter::TurnAtRate(float Rate)
{
	float TurnRateFactor;
	if(bAiming)
	{
		TurnRateFactor = ControllerTurnRate / ControllerAimingRateFactor;
	}
	else
	{
		TurnRateFactor = ControllerTurnRate;
	}
	AddControllerYawInput(Rate * TurnRateFactor * GetWorld()->GetDeltaSeconds());
}

void AShooterCharacter::LookUpAtRate(float Rate)
{
	float LookUpRateFactor;
	if(bAiming)
	{
		LookUpRateFactor = ControllerLookUpRate / ControllerAimingRateFactor;
	}
	else
	{
		LookUpRateFactor = ControllerLookUpRate;
	}
	AddControllerPitchInput(Rate * LookUpRateFactor * GetWorld()->GetDeltaSeconds());
}

void AShooterCharacter::Turn(float Value)
{
	float TurnRateFactor;
	if(bAiming)
	{
		TurnRateFactor = MouseTurnRate / MouseAimingRateFactor;
	}
	else
	{
		TurnRateFactor = MouseTurnRate;
	}
	AddControllerYawInput(Value * TurnRateFactor);
}

void AShooterCharacter::LookUp(float Value)
{
	float LookUpRateFactor;
	if(bAiming)
	{
		LookUpRateFactor = MouseLookUpRate / MouseAimingRateFactor;
	}
	else
	{
		LookUpRateFactor = MouseLookUpRate;
	}
	AddControllerPitchInput(Value * LookUpRateFactor);
}

void AShooterCharacter::IncrementOverlappedItemCount(int Amount)
{
	if(OverlappedItemCount + Amount <= 0)
	{
		OverlappedItemCount = 0;
		bShouldTraceForItems = false;
	}
	else
	{
		OverlappedItemCount += Amount;
		bShouldTraceForItems = true;
	}
}

bool AShooterCharacter::IsInventoryFull() const
{
	return Inventory.Num() == INVENTORY_CAPACITY;
}

//TODO Przerobić całą tą funkcję żeby to broń wykonywała strzał a nie osoba
void AShooterCharacter::FireWeapon()
{
	if(EquippedWeapon == nullptr) return;
	if(CombatState != ECombatState::ECS_Unoccupied) return;
	if(!WeaponHasAmmo()) return;
	
	PlayFireSound();
	SendBullet();
	PlayGunFireMontage();
	StartCrosshairBulletFire();
	EquippedWeapon->DecrementAmmo();
	StartFireTimer();
	//TODO Funkcja która sprawdza czy robimy slide czy nie, czyli czy jakaś część broni się rusza przy strzale
	if(EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Pistol)
	{
		EquippedWeapon->StartSlideTimer();
	}
}

void AShooterCharacter::PlayFireSound() const
{
	if(EquippedWeapon->GetFireSound())
	{
		UGameplayStatics::PlaySound2D(this, EquippedWeapon->GetFireSound());
	}
}

void AShooterCharacter::SendBullet()
{
	if(const USkeletalMeshSocket* MuzzleSocket = EquippedWeapon->GetItemMesh()->GetSocketByName("Muzzle"))
	{
		const FTransform SocketTransform = MuzzleSocket->GetSocketTransform(EquippedWeapon->GetItemMesh());
		if(EquippedWeapon->GetMuzzleFlash())
		{
			UGameplayStatics::SpawnEmitterAttached(EquippedWeapon->GetMuzzleFlash(), EquippedWeapon->GetItemMesh(),
				MuzzleSocket->SocketName
			);
		}

		FHitResult HitResult;
		bool bBeamEnd = GetBeamEndLocation(SocketTransform.GetLocation(), HitResult);

		if(bBeamEnd)
		{
			//sprawdzamy czy uderzony aktor posiada interfejs z uderzeniem
			if(HitResult.GetActor()->GetClass()->ImplementsInterface(UDamageable::StaticClass()))
			{
				Cast<IDamageable>(HitResult.GetActor())->Hit_Implementation(HitResult, this, GetController());
			}
			else if(ImpactParticle)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticle, HitResult.Location);
			}

			AEnemyCharacter* HitEnemy = Cast<AEnemyCharacter>(HitResult.GetActor());
			if(HitEnemy)
			{
				const float Damage = HitResult.BoneName.ToString() == HitEnemy->GetHeadBone().ToString() ?
					EquippedWeapon->GetHeadShotDamage() : EquippedWeapon->GetDamage();
				
				UGameplayStatics::ApplyDamage(HitEnemy, Damage, GetController(), this, UDamageType::StaticClass());
			}

			if(BeamParticle)
			{
				UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticle, SocketTransform);
				if(Beam)
				{
					Beam->SetVectorParameter(FName("Target"), HitResult.Location);
				}
			}
		}
	}
}

void AShooterCharacter::PlayGunFireMontage()
{
	PlayAnimMontage(HipFireMontage, 1, FName("StartFire"));
}

void AShooterCharacter::AimingButtonPressed()
{
	bAiming = true;
	GetCharacterMovement()->MaxWalkSpeed = CrouchSpeed;
	GetCameraComponent()->SetFieldOfView(CameraZoomedFOV);
}

void AShooterCharacter::AimingButtonReleased()
{
	bAiming = false;
	if(bIsCrouching) GetCharacterMovement()->MaxWalkSpeed = CrouchSpeed;
	else GetCharacterMovement()->MaxWalkSpeed = RegularSpeed;
	GetCameraComponent()->SetFieldOfView(CameraDefaultFOV);
}

void AShooterCharacter::ZoomCamera(float DeltaTime)
{
	if(bAiming)
	{
		CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, CameraZoomedFOV, DeltaTime, ZoomInterpolationSpeed);
	}
	else
	{
		CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, CameraDefaultFOV, DeltaTime, ZoomInterpolationSpeed);
	}
	GetCameraComponent()->SetFieldOfView(CameraCurrentFOV);
}

bool AShooterCharacter::GetBeamEndLocation(const FVector& MuzzleSocketLocation, FHitResult& OutHitResult) const
{
	FHitResult CrosshairHitResult;
	FVector OutBeamLocation;
	TraceUnderCrosshairs(CrosshairHitResult, OutBeamLocation);

	const FVector WeaponStart {MuzzleSocketLocation};
	const FVector StartToEnd {OutBeamLocation - MuzzleSocketLocation};
	const FVector WeaponEnd {MuzzleSocketLocation + StartToEnd * 1.25f};
	
	GetWorld()->LineTraceSingleByChannel(OutHitResult, WeaponStart, WeaponEnd, ECC_Visibility);

	if(!OutHitResult.bBlockingHit)
	{
		OutHitResult.Location = OutBeamLocation;
		return false;
	}

	return true;
}

bool AShooterCharacter::TraceUnderCrosshairs(FHitResult& OutHitResult, FVector& OutHitLocation) const
{
	FVector2D ViewportSize;
	if(GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	const FVector2D CrossPosition(ViewportSize.X / 2, ViewportSize.Y / 2);
	FVector CrossWorldPosition;
	FVector CrossWorldDirection;

	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
			UGameplayStatics::GetPlayerController(this, 0),
			CrossPosition,
			CrossWorldPosition,
			CrossWorldDirection);

	const FVector Start = CrossWorldPosition;
	const FVector End = Start + CrossWorldDirection * 40'000.f;
	OutHitLocation = End;
	if(bScreenToWorld)
	{
		GetWorld()->LineTraceSingleByChannel(OutHitResult, Start, End, ECC_Visibility);

		if(OutHitResult.bBlockingHit)
		{
			OutHitLocation = OutHitResult.Location;
			return true;
		}
	}

	return false;
}

void AShooterCharacter::TraceForItems()
{
	if(bShouldTraceForItems)
	{
		FHitResult ItemTraceResult;
		FVector HitLocation;
		TraceUnderCrosshairs(ItemTraceResult, HitLocation);
		if(ItemTraceResult.bBlockingHit)
		{
			AItem* HitItem = Cast<AItem>(ItemTraceResult.GetActor());
			if(HitItem && HitItem->GetPickupComponent())
			{
				HitItem->GetPickupComponent()->SetVisibility(true);
			}

			if(LastTracedItem && LastTracedItem != HitItem)
			{
				LastTracedItem->GetPickupComponent()->SetVisibility(false);
			}
			LastTracedItem = HitItem;
		}
	}
	else if(LastTracedItem)
	{
		LastTracedItem->GetPickupComponent()->SetVisibility(false);
	}
}

void AShooterCharacter::EquipDefaultWeapon()
{
	if(DefaultWeapon)
	{
		AWeapon* Weapon = GetWorld()->SpawnActor<AWeapon>(DefaultWeapon);
		EquipWeapon(Weapon);
	}
}

void AShooterCharacter::EquipWeapon(AWeapon* WeaponToEquip)
{
	if(WeaponToEquip)
	{
		const USkeletalMeshSocket* HandSocket = GetMesh()->GetSocketByName(FName("RightHandSocket"));
		if(HandSocket)
		{
			HandSocket->AttachActor(WeaponToEquip, GetMesh());

			if(EquippedWeapon == nullptr)
			{
				EquipItemDelegate.Broadcast(-1, WeaponToEquip->GetSlotIndex());
			}
			else
			{
				EquipItemDelegate.Broadcast(EquippedWeapon->GetSlotIndex(), WeaponToEquip->GetSlotIndex());
			}
			EquippedWeapon = WeaponToEquip;
			EquippedWeapon->SetItemState(EItemState::EIS_Equipped);
		}
	}
}

void AShooterCharacter::OnStartClimbing()
{
	Debug::Print("Otrzymałem wiadomosć o wspicznacze");
	bUseControllerRotationYaw = false;
	CombatState = ECombatState::ECS_Climbing;
	if(EquippedWeapon)
	{
		EquippedWeapon->SetItemVisibility(false);
	}
}

void AShooterCharacter::OnStopClimbing()
{
	Debug::Print("Zatrzymuje wspinanie wiadomosć o wspicznacze");
	bUseControllerRotationYaw = true;
	CombatState = ECombatState::ECS_Unoccupied;
	if(EquippedWeapon)
	{
		EquippedWeapon->SetItemVisibility(true);
	}
}

void AShooterCharacter::DropWeapon()
{
	if(!EquippedWeapon) return;

	const FDetachmentTransformRules DetachmentTransformRules(EDetachmentRule::KeepWorld, true);
	EquippedWeapon->GetItemMesh()->DetachFromComponent(DetachmentTransformRules);
	EquippedWeapon->SetItemState(EItemState::EIS_Falling);
	EquippedWeapon->ThrowWeapon();
}

void AShooterCharacter::SwapWeapon(AWeapon* WeaponToSwap)
{
	if(Inventory.Num() - 1 > EquippedWeapon->GetSlotIndex())
	{
		Inventory[EquippedWeapon->GetSlotIndex()] = WeaponToSwap;
	}
	
	DropWeapon();
	EquipWeapon(WeaponToSwap);
	LastTracedItem = nullptr;
}

void AShooterCharacter::SelectButtonPressed()
{
	if(CombatState != ECombatState::ECS_Unoccupied) return;
	if(LastTracedItem)
	{
		LastTracedItem->PlaySelectSound();

		auto Weapon = Cast<AWeapon>(LastTracedItem);
		if(Weapon)
		{
			if(Inventory.Num() < INVENTORY_CAPACITY)
			{
				Weapon->SetSlotIndex(Inventory.Num());
				Inventory.Add(Weapon);
				Weapon->SetItemState(EItemState::EIS_PickedUp);
			}
			else
			{
				SwapWeapon(Weapon);
			}
		}

		auto Ammo = Cast<AAmmo>(LastTracedItem);
		if(Ammo)
		{
			PickupAmmo(Ammo);
		}
	}
}

void AShooterCharacter::SelectButtonReleased()
{
	LastTracedItem = nullptr;
}

void AShooterCharacter::CalculateCrosshairSpread(float DeltaTime)
{
	CalculateCrosshairSpreadVelocity();
	CalculateCrosshairInAirFactor(DeltaTime);
	CalculateCrosshairSpreadAiming(DeltaTime);
	CalculateCrosshairShootingFactor(DeltaTime);
	
	CrossHairSpreadMultiplier = 0.5f + CrosshairVelocityFactor + CrosshairInAirFactor + CrosshairAimFactor +  CrosshairShootingFactor;
}

void AShooterCharacter::CalculateCrosshairSpreadVelocity()
{
	const FVector2D WalkSpeedRange {0.f, this->GetCharacterMovement()->MaxWalkSpeed};
	const FVector2D VelocityMultiplier {0.f, 1};
	FVector Velocity = GetVelocity();
	Velocity.Z = 0;
	CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplier, Velocity.Size());
}

void AShooterCharacter::CalculateCrosshairInAirFactor(float DeltaTime)
{
	if(GetCharacterMovement()->IsFalling())
	{
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 1.75, DeltaTime, 4);
	}
	else
	{
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0, DeltaTime, 25);
	}
}

void AShooterCharacter::CalculateCrosshairSpreadAiming(float DeltaTime)
{
	if(bAiming)
	{
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, -0.5, DeltaTime, 30);
	}
	else
	{
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0, DeltaTime, 30);
	}
}

void AShooterCharacter::CalculateCrosshairShootingFactor(float DeltaTime)
{
	if(bFiringBullet)
	{
		CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.3f, DeltaTime, 60);
	}
	else
	{
		CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0, DeltaTime, 60);
	}
}

void AShooterCharacter::OnStartClimbingAction()
{
	if(!CustomMovementComponent) return;
	if(bIsCrouching) return;

	if(!CustomMovementComponent->IsClimbing())
	{
		CustomMovementComponent->ToggleClimbing(true);
	}
	else
	{
		CustomMovementComponent->ToggleClimbing(false);
	}
}

void AShooterCharacter::StartCrosshairBulletFire()
{
	bFiringBullet = true;
	GetWorldTimerManager().SetTimer(CrosshairShootTimer, this, &AShooterCharacter::FinishCrosshairBulletFire,
		ShootTimeDuration);
}

void AShooterCharacter::FinishCrosshairBulletFire()
{
	bFiringBullet = false;
}

void AShooterCharacter::FireButtonPressed()
{
	bIsFireButtonPressed = true;
	FireWeapon();
}

void AShooterCharacter::FireButtonReleased()
{
	bIsFireButtonPressed = false;
}

void AShooterCharacter::StartFireTimer()
{
	if(!EquippedWeapon) return;
	
	CombatState = ECombatState::ECS_FireTimerInProgress;
	GetWorldTimerManager().SetTimer(AutoFireTimer, this, &AShooterCharacter::AutoFireReset, EquippedWeapon->GetAutoFireRate());
}

void AShooterCharacter::ReloadButtonPressed()
{
	ReloadWeapon();
}

void AShooterCharacter::ReloadWeapon()
{
	if(CombatState != ECombatState::ECS_Unoccupied) return;
	if(!EquippedWeapon) return;

	if(EquippedWeapon->ClipIsFull())
	{
		//TODO daj tu jakiś dźwięk że magazynek jest pełny
		return;
	}

	if(CarryingAmmo())
	{
		CombatState = ECombatState::ECS_Reloading;
		PlayAnimMontage(ReloadMontage, 1, EquippedWeapon->GetReloadMontageSection());
	}
		
}

void AShooterCharacter::Stun()
{
	if(CombatState == ECombatState::ECS_Dead) return;
	if(FMath::RandRange(0.f, 1.f) >= StunChance) return;
	
	CombatState = ECombatState::ECS_Stunned;
	PlayAnimMontage(HitReactMontage);
}

void AShooterCharacter::FinishStun()
{
	CombatState = ECombatState::ECS_Unoccupied;
}

void AShooterCharacter::InitializeAmmoMap()
{
	AmmoMap.Add(EAmmoType::EAT_Pistol, StartingPistolAmmo);
	AmmoMap.Add(EAmmoType::EAT_Rifle, StartingRifleAmmo);
}

bool AShooterCharacter::WeaponHasAmmo() const
{
	if(EquippedWeapon == nullptr) return false;

	return EquippedWeapon->GetAmmoCount() > 0;
}

bool AShooterCharacter::CarryingAmmo()
{
	if(!EquippedWeapon) return false;
	const EAmmoType AmmoType = EquippedWeapon->GetAmmoType();

	if(AmmoMap.Contains(AmmoType))
	{
		return AmmoMap[AmmoType] > 0;
	}
	
	return false;
}

void AShooterCharacter::PickupAmmo(AAmmo* Ammo)
{
	if(AmmoMap.Find(Ammo->GetAmmoType()))
	{
		AmmoMap[Ammo->GetAmmoType()] += Ammo->GetItemCount();;
	}
	
	if(EquippedWeapon->GetAmmoType() == Ammo->GetAmmoType())
	{
		if(EquippedWeapon->GetAmmoCount() == 0)
		{
			ReloadWeapon();
		}
	}

	Ammo->Destroy();
}

void AShooterCharacter::CrouchButtonPressed()
{
	if(CombatState == ECombatState::ECS_Climbing) return;
	
	if(!GetCharacterMovement()->IsFalling())
	{
		bIsCrouching = !bIsCrouching;
	}

	bIsCrouching ? GetCharacterMovement()->MaxWalkSpeed = CrouchSpeed : GetCharacterMovement()->MaxWalkSpeed = RegularSpeed;
	const float TargetCapsuleHalfHeight = bIsCrouching ? CrouchingCapsuleHalfHeight : RegularCapsuleHalfHeight;
	float DeltaCapsuleHalfHeight = TargetCapsuleHalfHeight - GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	FVector MeshOffset = FVector(0.f, 0.f, -DeltaCapsuleHalfHeight);
	GetMesh()->AddLocalOffset(MeshOffset);
	GetCapsuleComponent()->SetCapsuleHalfHeight(TargetCapsuleHalfHeight);
}

void AShooterCharacter::OneKeyPressed()
{
	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 0);
}

void AShooterCharacter::TwoKeyPressed()
{
	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 1);
}

void AShooterCharacter::ThreeKeyPressed()
{
	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 2);
}

void AShooterCharacter::FourKeyPressed()
{
	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 3);
}

void AShooterCharacter::FiveKeyPressed()
{
	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 4);
}

void AShooterCharacter::SixKeyPressed()
{
	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 5);
}

void AShooterCharacter::ExchangeInventoryItems(int CurrentItemIndex, int NewItemIndex)
{
	if(CurrentItemIndex == NewItemIndex || NewItemIndex >= Inventory.Num()) return;
    if(CombatState != ECombatState::ECS_Unoccupied) return;

	AWeapon* OldEquippedWeapon = EquippedWeapon;
	AWeapon* NewWeapon = Cast<AWeapon>(Inventory[NewItemIndex]);
	EquipWeapon(NewWeapon);

	OldEquippedWeapon->SetItemState(EItemState::EIS_PickedUp);
	NewWeapon->SetItemState(EItemState::EIS_Equipped);

	CombatState = ECombatState::ECS_Equipping;
	PlayAnimMontage(EquipMontage, 1, FName("Equip"));
	NewWeapon->PlaySelectSound();
}

void AShooterCharacter::Die()
{
	if(CombatState == ECombatState::ECS_Dead) return;
	CombatState = ECombatState::ECS_Dead;

	PlayAnimMontage(DeathMontage);
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if(PC)
	{
		DisableInput(PC);
	}
}

void AShooterCharacter::FinishDie()
{
	GetMesh()->bPauseAnims = true;
}

void AShooterCharacter::FinishReloading()
{
	if(CombatState == ECombatState::ECS_Stunned) return;
	
	CombatState = ECombatState::ECS_Unoccupied;
	if(!EquippedWeapon) return;
	EAmmoType AmmoType = EquippedWeapon->GetAmmoType();
	if(!AmmoMap.Contains(AmmoType)) return;

	int32 CarriedAmmo = AmmoMap[AmmoType];
	int32 MagazineEmptySpace = EquippedWeapon->GetMagazineCapacity() - EquippedWeapon->GetAmmoCount();

	if(MagazineEmptySpace > CarriedAmmo)
	{
		EquippedWeapon->ReloadAmmo(CarriedAmmo);
		CarriedAmmo = 0;
	}
	else
	{
		EquippedWeapon->ReloadAmmo(MagazineEmptySpace);
		CarriedAmmo -= MagazineEmptySpace;
	}

	AmmoMap.Add(AmmoType, CarriedAmmo);
}

void AShooterCharacter::FinishEquipping()
{
	if(CombatState == ECombatState::ECS_Stunned) return;
	
	CombatState = ECombatState::ECS_Unoccupied;
}

void AShooterCharacter::AutoFireReset()
{
	if(CombatState == ECombatState::ECS_Stunned) return;
	
	CombatState = ECombatState::ECS_Unoccupied;
	if(EquippedWeapon == nullptr) return;
	if(WeaponHasAmmo())
	{
		if(bIsFireButtonPressed && EquippedWeapon->GetAutomatic())
		{
			FireWeapon();
		}
	}
	else
	{
		ReloadWeapon();
	}
}

