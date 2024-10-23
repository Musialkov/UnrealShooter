#include "Enemies/EnemyAnimInstance.h"

#include "Enemies/EnemyCharacter.h"

void UEnemyAnimInstance::UpdateAnimationProperties(float DeltaTime)
{
	if(!EnemyCharacter)
	{
		EnemyCharacter = Cast<AEnemyCharacter>(TryGetPawnOwner());
	}

	if(!EnemyCharacter) return;

	FVector Velocity = EnemyCharacter->GetVelocity();
	Velocity.Z = 0;
	Speed = Velocity.Size();
}
