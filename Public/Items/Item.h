#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Item.generated.h"

UENUM(BlueprintType)
enum class EItemState : uint8
{
	EIS_Pickup UMETA(DisplayName = "Pickup"),
	EIS_Equipped UMETA(DisplayName = "Equipped"),
	EIS_PickedUp UMETA(DisplayName = "PickedUp"),
	EIS_Falling UMETA(DisplayName = "Falling"),
	
	EIS_MAX UMETA(DisplayName = "Default")
};

UCLASS()
class SHOOTERV2_API AItem : public AActor
{
	GENERATED_BODY()
	
public:	
	AItem();

	virtual void Tick(float DeltaTime) override;

	FORCEINLINE USkeletalMeshComponent* GetItemMesh() const {return ItemMesh;}
	FORCEINLINE class UWidgetComponent* GetPickupComponent() const {return PickupComponent;}
	FORCEINLINE class UBoxComponent* GetCollisionBox() const {return CollisionBox;}
	FORCEINLINE class USphereComponent* GetSphereComponent() const {return SphereComponent;}
	FORCEINLINE EItemState GetItemState() const {return ItemState;}
	FORCEINLINE class USoundCue* GetPickupSound() const {return PickupSound;}
	FORCEINLINE void SetPickupSound(USoundCue* PickupSoundValue) {this->PickupSound = PickupSoundValue;}
	FORCEINLINE int32 GetItemCount() const {return ItemCount;}
	FORCEINLINE int32 GetSlotIndex() const {return SlotIndex;}
	FORCEINLINE void SetSlotIndex(int32 Index) { SlotIndex = Index;}
	FORCEINLINE FString GetItemName() const {return ItemName;}
	FORCEINLINE void SetItemName(FString ItemNameValue) { this->ItemName = ItemNameValue;}
	FORCEINLINE UTexture2D* GetItemIcon() const {return ItemIcon;}
	FORCEINLINE void SetItemIcon(UTexture2D* ItemIconValue) { this->ItemIcon = ItemIconValue;}
	
	void SetItemState(const EItemState State);
	void PlaySelectSound() const;
	void SetItemVisibility(bool bIsVisible);

protected:
	virtual void BeginPlay() override;
	
	UFUNCTION()
	void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 otherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 otherBodyIndex);

	virtual void SetItemProperties(EItemState State);

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Item Properties", meta = (AllowPrivateAccess="true"))
	USkeletalMeshComponent* ItemMesh;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item Properties", meta = (AllowPrivateAccess="true"))
	UBoxComponent* CollisionBox;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item Properties", meta = (AllowPrivateAccess="true"))
	UWidgetComponent* PickupComponent;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item Properties", meta = (AllowPrivateAccess="true"))
	USphereComponent* SphereComponent;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item Properties", meta = (AllowPrivateAccess="true"))
	FString ItemName;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item Properties", meta = (AllowPrivateAccess="true"))
	int32 ItemCount;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Item Properties", meta = (AllowPrivateAccess="true"))
	EItemState ItemState;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item Properties", meta = (AllowPrivateAccess="true"))
	USoundCue* PickupSound;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Inventory", meta = (AllowPrivateAccess="true"))
	UTexture2D* IconBackground;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Inventory", meta = (AllowPrivateAccess="true"))
	UTexture2D* ItemIcon;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory", meta = (AllowPrivateAccess="true"))
	int32 SlotIndex;
 };
