#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "ActivateWeaponNotify.generated.h"

UCLASS()
class SHOOTERV2_API UActivateWeaponNotify : public UAnimNotify
{
	GENERATED_BODY()
public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Initialization", meta=(AllowPrivateAccess = true))
	bool RightHand;
};
