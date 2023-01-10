// Lordmatics Games

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "SnowboardAIController.generated.h"

class ASnowboardCharacterBase;
class AMapCheckpoint;
class AAICheckpointNode;

/**
 * 
 */
UCLASS()
class SNOWBOARDKIDS_API ASnowboardAIController : public AAIController
{
	GENERATED_BODY()

	ASnowboardAIController();

public:

	void OnCheckpointReached();

	FORCEINLINE bool IsOverlapped() const { return bIsOverlapped; }
	FORCEINLINE void SetOverlapped(bool Value) { bIsOverlapped = Value; }
protected:

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	void UpdateCheckpoint();

	const FVector GetFloorPositionFromNode(const AAICheckpointNode& Node);
private:

	UPROPERTY(Transient)
	ASnowboardCharacterBase* PossessedPawn;

	UPROPERTY(Transient, VisibleAnywhere, Category = "Navigation")
	AMapCheckpoint* CurrentCheckpoint;

	UPROPERTY(Transient, VisibleAnywhere, Category = "Navigation")
	AAICheckpointNode* CurrentNode;

	int CurrentCheckpointIndex;
	bool bIsOverlapped;
	FVector CurrentNodeLocation;
};
