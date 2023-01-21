// Lordmatics Games

#pragma once

#include "CoreMinimal.h"
#include "SnowboardKids/CustomActors/Items/ItemBase.h"
#include "UtilityItem.generated.h"

UCLASS()
class SNOWBOARDKIDS_API AUtilityItem : public AItemBase
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AUtilityItem();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
