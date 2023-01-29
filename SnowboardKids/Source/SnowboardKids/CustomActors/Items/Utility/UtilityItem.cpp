// Lordmatics Games


#include "SnowboardKids/CustomActors/Items/Utility/UtilityItem.h"
#include "SnowboardKids/Utils/GameUtils.h"

//Rock,
//Ghost,
//TripleGhost,
//Invisibility,
//FryingPan,
//MoneyThief,
//SpeedFan,

const TMap<EUtilityType, int> AUtilityItem::FirstPlaceWeights =
{		
	{ EUtilityType::SpeedFan, 1 },
	{ EUtilityType::FryingPan, 1 },
	{ EUtilityType::MoneyThief, 1 },
	{ EUtilityType::TripleGhost, 2 },
	{ EUtilityType::Ghost, 5 },
	{ EUtilityType::Invisibility, 25 },
	{ EUtilityType::Rock, 100 },
};

const TMap<EUtilityType, int> AUtilityItem::SecondPlaceWeights =
{
	{ EUtilityType::SpeedFan, 5 },
	{ EUtilityType::FryingPan, 5 },
	{ EUtilityType::MoneyThief, 1 },
	{ EUtilityType::TripleGhost, 12 },
	{ EUtilityType::Ghost, 25 },
	{ EUtilityType::Invisibility, 30 },
	{ EUtilityType::Rock, 50 },
};

const TMap<EUtilityType, int> AUtilityItem::ThirdPlaceWeights =
{
	{ EUtilityType::SpeedFan, 35 },
	{ EUtilityType::FryingPan, 45 },
	{ EUtilityType::MoneyThief, 1 },
	{ EUtilityType::TripleGhost, 25 },
	{ EUtilityType::Ghost, 50 },
	{ EUtilityType::Invisibility, 25 },
	{ EUtilityType::Rock, 0 },
};

const TMap<EUtilityType, int> AUtilityItem::FourthPlaceWeights =
{
	{ EUtilityType::SpeedFan, 85 },
	{ EUtilityType::FryingPan, 75 },
	{ EUtilityType::MoneyThief, 1 },
	{ EUtilityType::TripleGhost, 45 },
	{ EUtilityType::Ghost, 20 },
	{ EUtilityType::Invisibility, 50 },
	{ EUtilityType::Rock, 0 },
};

const TMap<int, const TMap<EUtilityType, int>> AUtilityItem::ItemWeightMap =
{
	{ 1, FirstPlaceWeights },
	{ 2, SecondPlaceWeights },
	{ 3, ThirdPlaceWeights },
	{ 4, FourthPlaceWeights },
};

// Sets default values
AUtilityItem::AUtilityItem() :
	Super(),
	UtilityItemType(EUtilityType::Rock)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void AUtilityItem::OnSpawned()
{
	Super::OnSpawned();
}

EUtilityType AUtilityItem::GenerateItemViaPosition(int Position)
{
	const bool bFoundMap = ItemWeightMap.Contains(Position);
	if (bFoundMap)
	{
		const TMap<EUtilityType, int>& TargetMap = ItemWeightMap[Position];
		int Iterations = 0;
		const int TotalIterations = 3;
		while (Iterations < TotalIterations)
		{
			for (const TPair<EUtilityType, int>& Current : TargetMap)
			{
				EUtilityType Type = Current.Key;
				const int Chance = Current.Value;
				const int RandomChance = FMath::RandRange(0, 100);
				if (RandomChance <= Chance )
				{
					FName EnumName;
					GameUtils::EnumString<EUtilityType>(Type, EnumName);
					UE_LOG(LogTemp, Log, TEXT("Generated Utility %s, via chance: %d <= %d"), *EnumName.ToString(), RandomChance, Chance);
					return Type;
				}
			}
			Iterations++;
		}
	}
	// If after a few attempts, nothing was picked, default to Hands.	
	UE_LOG(LogTemp, Log, TEXT("Generated Default Utility Rock"));
	return EUtilityType::Rock;
}

// Called when the game starts or when spawned
void AUtilityItem::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AUtilityItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

