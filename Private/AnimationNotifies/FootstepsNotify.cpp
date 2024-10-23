#include "AnimationNotifies/FootstepsNotify.h"
#include "NiagaraFunctionLibrary.h"
#include "DataTable/FootstepsDataTable.h"
#include "Engine/DataTable.h"
#include "Enums/PhysicalSurface.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

UFootstepsNotify::UFootstepsNotify()
{
	const FString WeaponTablePath{ TEXT("/Script/Engine.DataTable'/Game/AA_Game/DataTable/FootstepsDataTable.FootstepsDataTable'") };
	FootstepsDataTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *WeaponTablePath));
}

void UFootstepsNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                              const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	
	const EPhysicalSurface PhysicalSurface = GetSurfaceType(MeshComp->GetOwner());
	const FFootstepsDataTable* FootstepsDataRow = nullptr;
	const FString RowName = GetSurfaceName(PhysicalSurface);;
	
	if (!RowName.IsEmpty())
	{
		FootstepsDataRow = FootstepsDataTable->FindRow<FFootstepsDataTable>(*RowName, "");
	}
	
	if (FootstepsDataRow)
	{
		const FVector FootLocation = MeshComp->GetSocketLocation(BoneName);
		if (FootstepsDataRow->FootstepSound)
		{
			UGameplayStatics::PlaySoundAtLocation(MeshComp->GetWorld(), FootstepsDataRow->FootstepSound, FootLocation);
		}
		if (FootstepsDataRow->NiagaraEffect)
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(MeshComp->GetWorld(), FootstepsDataRow->NiagaraEffect, FootLocation);
		}
	}
}

EPhysicalSurface UFootstepsNotify::GetSurfaceType(AActor* Actor)
{
	FHitResult HitResult;
	const FVector StartLocation = Actor->GetActorLocation();
	const FVector EndLocation = StartLocation + FVector(0.f, 0.f, -400.f);
	FCollisionQueryParams QueryParams;
	QueryParams.bReturnPhysicalMaterial = true;
	
	Actor->GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, QueryParams);
	return UPhysicalMaterial::DetermineSurfaceType(HitResult.PhysMaterial.Get());
}

FString UFootstepsNotify::GetSurfaceName(EPhysicalSurface PhysicalSurface)
{
	FString RowName;
	switch (PhysicalSurface)
	{
	case METAL_PHYSICAL_SURFACE:
		RowName = TEXT("Metal");
		break;
	case STONE_PHYSICAL_SURFACE:
		RowName = TEXT("Stone");
		break;
	case GRASS_PHYSICAL_SURFACE:
		RowName = TEXT("Grass");
		break;
	case WATER_PHYSICAL_SURFACE:
		RowName = TEXT("Water");
		break;
	}

	return RowName;
}
