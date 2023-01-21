// Lordmatics Games

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Snowboardkids/CustomActors/Items/Offensive/OffensiveItem.h"
#include "ProjectileTable.generated.h"

class AProjectileBase;

USTRUCT(BlueprintType)
struct FProjectileTableRow : public FTableRowBase
{
	GENERATED_BODY()

	FProjectileTableRow();

	UPROPERTY(EditAnywhere, Category = "Projectile")
	EOffensiveType OffensiveItemType;

	UPROPERTY(EditAnywhere, Category = "Projectile")
	TSubclassOf<AOffensiveItem> OffensiveClass;
};

/**
 * 
 */
UCLASS()
class SNOWBOARDKIDS_API UProjectileTable : public UDataTable
{
	GENERATED_BODY()

	UProjectileTable();
};
