#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "Enums/AmmoType.h"
#include "Ammo.generated.h"

UCLASS()
class SHOOTERV2_API AAmmo : public AItem
{
	GENERATED_BODY()

public:
	AAmmo();
	virtual void Tick(float DeltaTime) override;
	
	FORCEINLINE UStaticMeshComponent* GetAmmoMesh() const {return AmmoMesh;}
	FORCEINLINE EAmmoType GetAmmoType() const {return AmmoType;}

protected:
	virtual void BeginPlay() override;

	virtual void SetItemProperties(EItemState State) override;
	
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ammo", meta=(AllowPrivateAccess = "true"))
	UStaticMeshComponent* AmmoMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ammo", meta=(AllowPrivateAccess = "true"))
	EAmmoType AmmoType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ammo", meta=(AllowPrivateAccess = "true"))
	UTexture2D* AmmoIcon;
};
