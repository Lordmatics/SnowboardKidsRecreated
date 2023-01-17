// Lordmatics Games

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ProjectileBase.h"
#include "ProjectileTable.generated.h"

class AProjectileBase;

USTRUCT(BlueprintType)
struct FProjectileTableRow : public FTableRowBase
{
	GENERATED_BODY()

	FProjectileTableRow();

	UPROPERTY(EditAnywhere, Category = "Projectile")
	EProjectileType ProjectileType;

	UPROPERTY(EditAnywhere, Category = "Projectile")
	TSubclassOf<AProjectileBase> ProjectileClass;
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
