// Lordmatics Games


#include "SnowboardKids/CustomActors/Items/Offensive/OffensiveItem.h"
#include "SnowboardKids/Utils/GameUtils.h"


//Tornado,
//Ice,
//Parachute,
//Hands,
//Snowmen,
//Bombs,
const TMap<EOffensiveType, int> AOffensiveItem::FirstPlaceWeights =
{
	{ EOffensiveType::Tornado, 1 },
	{ EOffensiveType::Ice, 1 },
	{ EOffensiveType::Parachute, 12 },
	{ EOffensiveType::Hands, 25 },
	{ EOffensiveType::Snowmen, 50 },
	{ EOffensiveType::Bombs, 100 },
};

const TMap<EOffensiveType, int> AOffensiveItem::SecondPlaceWeights =
{
	{ EOffensiveType::Tornado, 6 },
	{ EOffensiveType::Ice, 12 },
	{ EOffensiveType::Parachute, 24 },
	{ EOffensiveType::Hands, 50 },
	{ EOffensiveType::Snowmen, 25 },
	{ EOffensiveType::Bombs, 25 },
};

const TMap<EOffensiveType, int> AOffensiveItem::ThirdPlaceWeights =
{
	{ EOffensiveType::Tornado, 25 },
	{ EOffensiveType::Ice, 45 },
	{ EOffensiveType::Parachute, 45 },
	{ EOffensiveType::Hands, 35 },
	{ EOffensiveType::Snowmen, 5 },
	{ EOffensiveType::Bombs, 1 },
};

const TMap<EOffensiveType, int> AOffensiveItem::FourthPlaceWeights =
{
	{ EOffensiveType::Tornado, 66 },
	{ EOffensiveType::Ice, 88 },
	{ EOffensiveType::Parachute, 100 },
	{ EOffensiveType::Hands, 0 },
	{ EOffensiveType::Snowmen, 0 },
	{ EOffensiveType::Bombs, 0 },
};

const TMap<int, const TMap<EOffensiveType, int>> AOffensiveItem::ItemWeightMap =
{
	{ 1, FirstPlaceWeights },
	{ 2, SecondPlaceWeights },
	{ 3, ThirdPlaceWeights },
	{ 4, FourthPlaceWeights },
};

// Sets default values
AOffensiveItem::AOffensiveItem() :
	Super(),
	OffensiveItemType(EOffensiveType::Hands)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void AOffensiveItem::OnSpawned()
{
	Super::OnSpawned();
}

EOffensiveType AOffensiveItem::GenerateItemViaPosition(int Position)
{
	const bool bFoundMap = ItemWeightMap.Contains(Position);
	if (bFoundMap)
	{		
		const TMap<EOffensiveType, int>& TargetMap = ItemWeightMap[Position];
		int Iterations = 0;
		const int TotalIterations = 3;
		while (Iterations < TotalIterations)
		{
			for (const TPair<EOffensiveType, int>& Current : TargetMap)
			{
				EOffensiveType Type = Current.Key;
				const int Chance = Current.Value;
				const int RandomChance = FMath::RandRange(0, 100);
				if (RandomChance <= Chance )
				{
					FName EnumName;
					GameUtils::EnumString<EOffensiveType>(Type, EnumName);
					UE_LOG(LogTemp, Log, TEXT("Generated Offensive %s, via chance: %d <= %d"), *EnumName.ToString(), RandomChance, Chance);
					return Type;
				}
			}
			Iterations++;
		}
	}
	// If after a few attempts, nothing was picked, default to Hands.
	UE_LOG(LogTemp, Log, TEXT("Generated Default Offensive Hands"));
	return EOffensiveType::Hands;
}

// Called when the game starts or when spawned
void AOffensiveItem::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AOffensiveItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}