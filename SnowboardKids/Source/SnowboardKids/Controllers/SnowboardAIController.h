// Lordmatics Games

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "SnowboardAIController.generated.h"

class ASnowboardCharacterBase;

/**
 * 
 */
UCLASS()
class SNOWBOARDKIDS_API ASnowboardAIController : public AAIController
{
	GENERATED_BODY()

	ASnowboardAIController();

public:

protected:

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

private:

	UPROPERTY(Transient)
	ASnowboardCharacterBase* PossessedPawn;
};
