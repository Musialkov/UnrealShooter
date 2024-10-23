#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTEnemyAttack.generated.h"

UCLASS()
class SHOOTERV2_API UBTTEnemyAttack : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTTEnemyAttack();
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
};
