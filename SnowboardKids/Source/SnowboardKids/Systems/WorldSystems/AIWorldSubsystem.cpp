// Lordmatics Games


#include "SnowboardKids/Systems/WorldSystems/AIWorldSubsystem.h"
#include "SnowboardKids/CustomActors/MapCheckpoint.h"

UAIWorldSubsystem::UAIWorldSubsystem() :
	UWorldSubsystem(),
	Checkpoints()
{

}

UAIWorldSubsystem* UAIWorldSubsystem::GetAISystem(const UWorld* World)
{
	return UWorld::GetSubsystem<UAIWorldSubsystem>(World);
}

UAIWorldSubsystem* UAIWorldSubsystem::GetAISystem(const UWorld& World)
{
	return World.GetSubsystem<UAIWorldSubsystem>();
}

void UAIWorldSubsystem::RegisterCheckpoint(AMapCheckpoint* Checkpoint)
{
	Checkpoints.Push(Checkpoint);
	Checkpoints.Sort();
}

void UAIWorldSubsystem::UnregisterCheckpoint(AMapCheckpoint* Checkpoint)
{
	Checkpoints.RemoveSingle(Checkpoint);
}

AMapCheckpoint* UAIWorldSubsystem::GetCheckpointAtIndex(const int Index) const
{
	const int Size = Checkpoints.Num();
	if (Index >= Size || Index < 0)
	{
		return nullptr;
	}

	return Checkpoints[Index];
}
