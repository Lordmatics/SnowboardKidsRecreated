// Lordmatics Games

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "UtilityItem.h"
#include "UtilityTable.generated.h"

USTRUCT(BlueprintType)
struct FUtilityTableRow : public FTableRowBase
{
	GENERATED_BODY()

	FUtilityTableRow();

	UPROPERTY(EditAnywhere, Category = "Utility")
	EUtilityType UtilityType;

	UPROPERTY(EditAnywhere, Category = "Utility")
	TSubclassOf<AUtilityItem> UtilityClass;
};

/**
 * 
 */
UCLASS()
class SNOWBOARDKIDS_API UUtilityTable : public UDataTable
{
	GENERATED_BODY()

	UUtilityTable();
};
