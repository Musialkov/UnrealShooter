#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyAIController.generated.h"

class UBehaviorTreeComponent;

UCLASS()
class SHOOTERV2_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
	AEnemyAIController();
	virtual void OnPossess(APawn* InPawn) override;

	FORCEINLINE UBlackboardComponent* GetBlackboard() const {return BlackboardComponent;}

private:
	UPROPERTY(BlueprintReadWrite, Category= "AI", meta=(AllowPrivateAccess = true))
	UBlackboardComponent* BlackboardComponent;
	UPROPERTY(BlueprintReadWrite, Category= "AI", meta=(AllowPrivateAccess = true))
	UBehaviorTreeComponent* BehaviourTreeComponent;
};
