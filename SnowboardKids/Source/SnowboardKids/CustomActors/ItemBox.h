// Lordmatics Games

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemBox.generated.h"

class UStaticMeshComponent;
class UMaterialInterface;

UENUM(BlueprintType)
enum class EItemBoxType : uint8
{
	Blue,
	Red
};

UCLASS()
class SNOWBOARDKIDS_API AItemBox : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AItemBox();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION()
	void OnTriggerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnTriggerOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void ResetItemBox();

private:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item", meta = (AllowPrivateAccess = "true"))
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* ItemBoxMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* ItemBoxHeaderMesh;

	UPROPERTY(EditAnywhere, Category = "Item", meta = (AllowPrivateAccess = "true"))
	EItemBoxType ItemBoxType;

	UPROPERTY(EditAnywhere, Category = "Item", meta = (AllowPrivateAccess = "true"))
	UMaterialInterface* MainMaterial;

	UPROPERTY(EditAnywhere, Category = "Item", meta = (AllowPrivateAccess = "true"))
	UMaterialInterface* ClosedMaterial;

	UPROPERTY(EditAnywhere, Category = "Item", meta = (AllowPrivateAccess = "true"))
	float YawRot;

	UPROPERTY(EditAnywhere, Category = "Item", meta = (AllowPrivateAccess = "true"))
	float HitYawRot;

	UPROPERTY(EditAnywhere, Category = "Item", meta = (AllowPrivateAccess = "true"))
	float MinVariance;

	UPROPERTY(EditAnywhere, Category = "Item", meta = (AllowPrivateAccess = "true"))
	float MaxVariance;

	float InitialYawRot;

	FTimerHandle ItemBoxTimerHandle;

	bool bTriggered;
};
