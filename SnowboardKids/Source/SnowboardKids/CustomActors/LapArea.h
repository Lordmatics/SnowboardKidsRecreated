// Lordmatics Games

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LapArea.generated.h"

class USceneComponent;
class UBoxComponent;

UCLASS()
class SNOWBOARDKIDS_API ALapArea : public AActor
{
	GENERATED_BODY()
	
public:	
	ALapArea();

protected:

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason);

	UFUNCTION()
		void OnTriggerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lap", meta = (AllowPrivateAccess = "true"))
		USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lap", meta = (AllowPrivateAccess = "true"))
		UBoxComponent* TriggerArea;

};
