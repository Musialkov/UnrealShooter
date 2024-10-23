#include "AnimationNotifies/EnemyStunFinishNotify.h"

#include "Enemies/EnemyCharacter.h"

void UEnemyStunFinishNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                    const FAnimNotifyEventReference& EventReference)
{
	const auto EnemyCharacter = Cast<AEnemyCharacter>(MeshComp->GetOwner());
	if(EnemyCharacter)
	{
		EnemyCharacter->SetIsStunned(false);
	}
}

