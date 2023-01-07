// Lordmatics Games

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "AIWorldSubsystem.generated.h"

class AMapCheckpoint;

/**
 * Method 1: UWorld* World = ...
 * UMySystem* MySystem = World->GetSubsystem<UMySystem>();
 * 
 * Method 2: UWorld* World = ...
 * UMySystem* MySystem = UWorld::GetSubsystem<UMySystem>(World);
 * 
 * Use Method 2 if World is not already null checked.
 */
UCLASS()
class SNOWBOARDKIDS_API UAIWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

	UAIWorldSubsystem();

public:

	static UAIWorldSubsystem* GetAISystem(const UWorld* World);
	static UAIWorldSubsystem* GetAISystem(const UWorld& World);

	void RegisterCheckpoint(AMapCheckpoint* Checkpoint);
	void UnregisterCheckpoint(AMapCheckpoint* Checkpoint);

private:

	UPROPERTY(Transient)
	TArray<AMapCheckpoint*> Checkpoints;
};
