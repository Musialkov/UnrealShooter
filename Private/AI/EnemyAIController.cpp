
#include "AI/EnemyAIController.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Enemies/EnemyCharacter.h"

AEnemyAIController::AEnemyAIController()
{
	BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("Blackboard"));
	check(BlackboardComponent);

	BehaviourTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviourTree"));
	check(BehaviourTreeComponent);
}

void AEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	if(!InPawn) return;

	AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(InPawn);
	if(!Enemy) return;

	if(Enemy->GetBehaviourTree())
	{
		BlackboardComponent->InitializeBlackboard(*Enemy->GetBehaviourTree()->BlackboardAsset);
	}
	
}
