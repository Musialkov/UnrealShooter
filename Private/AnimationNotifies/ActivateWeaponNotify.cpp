#include "AnimationNotifies/ActivateWeaponNotify.h"

#include "Enemies/EnemyCharacter.h"

void UActivateWeaponNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                   const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	auto Enemy = Cast<AEnemyCharacter>(MeshComp->GetOwner());
	if(Enemy)
	{
		Enemy->ActivateWeapon(RightHand);
	}
}
