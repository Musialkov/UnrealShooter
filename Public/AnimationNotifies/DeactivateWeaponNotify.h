#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "DeactivateWeaponNotify.generated.h"

UCLASS()
class SHOOTERV2_API UDeactivateWeaponNotify : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Initialization", meta=(AllowPrivateAccess = true))
	bool RightHand;
	
};
