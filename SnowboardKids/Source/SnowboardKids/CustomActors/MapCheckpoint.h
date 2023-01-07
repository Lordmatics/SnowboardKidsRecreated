// Lordmatics Games

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MapCheckpoint.generated.h"

class UBoxComponent;
class AAICheckpointNode;

UCLASS()
class SNOWBOARDKIDS_API AMapCheckpoint : public AActor
{
	GENERATED_BODY()
	
public:	

	AMapCheckpoint();

	FORCEINLINE int GetCheckpointIndex() const { return CheckpointIndex; }
	FORCEINLINE const TArray<AAICheckpointNode*>& GetAINodes() const { return AINodes; }

	FORCEINLINE bool operator<(const AMapCheckpoint& B) const
	{
		const int IndexA = GetCheckpointIndex();
		const int IndexB = B.GetCheckpointIndex();
		return IndexA < IndexB;
	}

protected:

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void OnTriggerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnTriggerOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Checkpoint", meta = (AllowPrivateAccess = "true"))
	UBoxComponent* TriggerArea;

	UPROPERTY(EditAnywhere, Category = "Checkpoint")
	TArray<AAICheckpointNode*> AINodes;

	/*
	* What order in the map is this checkpoint for.
	* Subsystem will organise the list of checkpoints to track an AIs position in the map.
	*/
	UPROPERTY(EditAnywhere, Category = "Checkpoint")
	int CheckpointIndex;

	UPROPERTY(Transient)
	bool bOverlapped;
};
