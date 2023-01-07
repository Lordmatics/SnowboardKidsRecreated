// Lordmatics Games

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AICheckpointNode.generated.h"

class USceneComponent;
class UBillboardComponent;

UCLASS()
class SNOWBOARDKIDS_API AAICheckpointNode : public AActor
{
	GENERATED_BODY()
	
public:	

	AAICheckpointNode();

protected:

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Checkpoint", meta = (AllowPrivateAccess = "true"))
		USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Checkpoint", meta = (AllowPrivateAccess = "true"))
		UBillboardComponent* Billboard;

};
