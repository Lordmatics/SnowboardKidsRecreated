// Lordmatics Games

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Coins.generated.h"

class USceneComponent;
class UStaticMeshComponent;

UCLASS()
class SNOWBOARDKIDS_API ACoins : public AActor
{
	GENERATED_BODY()

public:
	ACoins();

protected:

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason);

	UFUNCTION()
		void OnTriggerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	int GetCoinAmount() const { return CoinAmount; }
	void SetCoinAmount(int Amount) { CoinAmount = Amount; }

private:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Coins", meta = (AllowPrivateAccess = "true"))
		USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Coins", meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* CoinMesh;

	UPROPERTY(EditAnywhere, Category = "Coins")
		int CoinAmount;
};
