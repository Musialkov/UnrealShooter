#pragma once

#include "NiagaraSystem.h"
#include "Engine/DataTable.h"
#include "Sound/SoundCue.h"

#include "FootstepsDataTable.generated.h"

USTRUCT(BlueprintType)
struct FFootstepsDataTable : public FTableRowBase
{
	GENERATED_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundCue* FootstepSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UNiagaraSystem* NiagaraEffect;
};
