#include "AnimationNotifies/PlayerStunFinishNotify.h"

#include "Player/ShooterCharacter.h"

void UPlayerStunFinishNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                     const FAnimNotifyEventReference& EventReference)
{
	const auto Player = Cast<AShooterCharacter>(MeshComp->GetOwner());
	if(Player)
	{
		Player->FinishStun();
	}
}
