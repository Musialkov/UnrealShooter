#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"

#include "FootstepsNotify.generated.h"

class UDataTable;

UCLASS()
class SHOOTERV2_API UFootstepsNotify : public UAnimNotify
{
	GENERATED_BODY()

public:
	UFootstepsNotify();
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
protected:
	static EPhysicalSurface GetSurfaceType(AActor* Actor);
private:
	static FString GetSurfaceName(EPhysicalSurface PhysicalSurface);
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Properties", meta=(AllowPrivateAccess = "true"))
	FName BoneName;
	UPROPERTY()
	UDataTable* FootstepsDataTable;
};
