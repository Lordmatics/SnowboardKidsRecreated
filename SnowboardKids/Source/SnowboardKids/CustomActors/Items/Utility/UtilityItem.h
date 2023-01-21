// Lordmatics Games

#pragma once

#include "CoreMinimal.h"
#include "SnowboardKids/CustomActors/Items/ItemBase.h"
#include "UtilityItem.generated.h"

UENUM(BlueprintType)
enum class EUtilityType : uint8
{
	None,
	Rock,
	Ghost,
	TripleGhost,
	Invisibility,
	FryingPan,
	MoneyThief,
	SpeedFan,
	MAX UMETA(Hidden)
};

UCLASS()
class SNOWBOARDKIDS_API AUtilityItem : public AItemBase
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AUtilityItem();

	FORCEINLINE EUtilityType GetUtilityItemType() const { return UtilityItemType; }

	virtual void OnSpawned() override;
	
	static EUtilityType GenerateItemViaPosition(int Position);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	
	UPROPERTY(EditAnywhere, Category = "Utility")
		EUtilityType UtilityItemType;

private:
	static const TMap<EUtilityType, int> FirstPlaceWeights;
	static const TMap<EUtilityType, int> SecondPlaceWeights;
	static const TMap<EUtilityType, int> ThirdPlaceWeights;
	static const TMap<EUtilityType, int> FourthPlaceWeights;

	static const TMap<int, const TMap<EUtilityType, int>> ItemWeightMap;
};
