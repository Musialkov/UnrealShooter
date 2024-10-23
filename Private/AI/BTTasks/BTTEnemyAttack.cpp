#include "AI/BTTasks/BTTEnemyAttack.h"

#include "AIController.h"
#include "Enemies/EnemyCharacter.h"


UBTTEnemyAttack::UBTTEnemyAttack()
{
	NodeName = "Attack";
}

EBTNodeResult::Type UBTTEnemyAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	if(OwnerComp.GetAIOwner() == nullptr)
	{
		return EBTNodeResult::Failed;
	}
 
	auto Enemy = Cast<AEnemyCharacter>(OwnerComp.GetAIOwner()->GetPawn());
 
	if(Enemy == nullptr)
	{
		return EBTNodeResult::Failed;
	}
    
	Enemy->PlayAttackMontage();
    
	return EBTNodeResult::Succeeded;
}
