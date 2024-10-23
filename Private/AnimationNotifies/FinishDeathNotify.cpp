#include "AnimationNotifies/FinishDeathNotify.h"

#include "Enemies/EnemyCharacter.h"

void UFinishDeathNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	auto Enemy = Cast<AEnemyCharacter>(MeshComp->GetOwner());
	if(Enemy)
	{
		Enemy->FinishDeath();
	}
}
