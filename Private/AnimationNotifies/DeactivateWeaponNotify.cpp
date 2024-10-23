#include "AnimationNotifies/DeactivateWeaponNotify.h"

#include "Enemies/EnemyCharacter.h"

void UDeactivateWeaponNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                     const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	auto Enemy = Cast<AEnemyCharacter>(MeshComp->GetOwner());
	if(Enemy)
	{
		Enemy->DeactivateWeapon(RightHand);
	}
}
