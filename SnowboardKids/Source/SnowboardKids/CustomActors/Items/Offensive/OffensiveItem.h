// Lordmatics Games

#pragma once

#include "CoreMinimal.h"
#include "SnowboardKids/CustomActors/Items/ItemBase.h"
#include "OffensiveItem.generated.h"

UENUM(BlueprintType)
enum class EOffensiveType : uint8
{
	None,
	Tornado,
	Ice,	
	Parachute,
	Hands,	
	Snowmen,
	Bombs,
	MAX UMETA(Hidden)
};

UCLASS()
class SNOWBOARDKIDS_API AOffensiveItem : public AItemBase
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AOffensiveItem();

	FORCEINLINE EOffensiveType GetOffensiveItemType() const { return OffensiveItemType; }

	virtual void OnSpawned() override;

	static EOffensiveType GenerateItemViaPosition(int Position);	

protected:

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

protected:
	UPROPERTY(EditAnywhere, Category = "Offensive")
		EOffensiveType OffensiveItemType;

private:
	
	static const TMap<EOffensiveType, int> FirstPlaceWeights;
	static const TMap<EOffensiveType, int> SecondPlaceWeights;
	static const TMap<EOffensiveType, int> ThirdPlaceWeights;
	static const TMap<EOffensiveType, int> FourthPlaceWeights;

	static const TMap<int, const TMap<EOffensiveType, int>> ItemWeightMap;

};
