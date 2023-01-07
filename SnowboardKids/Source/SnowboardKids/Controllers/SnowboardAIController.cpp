// Lordmatics Games


#include "SnowboardKids/Controllers/SnowboardAIController.h"
#include "../CustomCharacters/SnowboardCharacterBase.h"
#include "../CustomActors/AICheckpointNode.h"
#include "../Systems/WorldSystems/AIWorldSubsystem.h"
#include <DrawDebugHelpers.h>
#include "../CustomActors/MapCheckpoint.h"

ASnowboardAIController::ASnowboardAIController() :
	Super(),
	PossessedPawn(nullptr),
	CurrentCheckpointIndex(-1)
{

}

void ASnowboardAIController::BeginPlay()
{
	Super::BeginPlay();

	PossessedPawn = Cast<ASnowboardCharacterBase>(GetPawn());

	if (CurrentCheckpointIndex < 0)
	{
		UpdateCheckpoint();
	}	
}

void ASnowboardAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!PossessedPawn)
	{
		PossessedPawn = Cast<ASnowboardCharacterBase>(GetPawn());
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (!PossessedPawn)
	{
		return;
	}

	if (!CurrentCheckpoint || !CurrentNode)
	{
		return;
	}

	// Steer towards current checkpoint
	const FVector& NodeLocation = CurrentNodeLocation;
	DrawDebugString(World, NodeLocation + FVector(0.0f, 0.0f, 50.0f), FString::Printf(TEXT("Index: %d"), CurrentCheckpointIndex));
	DrawDebugSphere(World, NodeLocation, 50.0f, 4, FColor::Green, true, DeltaSeconds);

	float NormalisedTurnValue = 0.0f;
	const FVector& CurrentPawnLocation = PossessedPawn->GetActorLocation();
	FVector DirectionToNode = CurrentNodeLocation - CurrentPawnLocation;
	DirectionToNode.Normalize();

	const FVector& ForwardVector = PossessedPawn->GetActorRightVector();
	const float Dot = FVector::DotProduct(DirectionToNode, ForwardVector);

	GEngine->AddOnScreenDebugMessage(-1, DeltaSeconds, FColor::Blue, FString::Printf(TEXT("AIDot: %.1f"), Dot));
	// Calculate turn value based on direction to node location versus current location of AI character.
	NormalisedTurnValue = FMath::Clamp(Dot, -1.0f, 1.0f);
	
	const float Threshold = 0.0f;// 05f;
	if (NormalisedTurnValue < Threshold && NormalisedTurnValue >= -Threshold)
	{
		NormalisedTurnValue = 0.0f;
	}
	PossessedPawn->MoveRight(NormalisedTurnValue);
}

void ASnowboardAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	PossessedPawn = Cast<ASnowboardCharacterBase>(InPawn);	
	PossessedPawn->SetIsAI(true);
	PossessedPawn->SetController(this);

	if (CurrentCheckpointIndex < 0)
	{
		UpdateCheckpoint();
	}	

	UE_LOG(LogTemp, Log, TEXT("Pawn Possessed By AIController"));

	auto b = PossessedPawn->GetOwner();
	if (!b)
	{
		UE_LOG(LogTemp, Log, TEXT("Pawn has no owner"));
		return;
	}

	auto a = b->GetInstigatorController();
	//auto a = PossessedPawn->GetController();
	if (a)
	{
		UE_LOG(LogTemp, Log, TEXT("controller valid"));
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("controller null"));
	}
	
}

void ASnowboardAIController::OnUnPossess()
{
	Super::OnUnPossess();

	UE_LOG(LogTemp, Log, TEXT("Pawn UnPossessed By AIController"));
}

void ASnowboardAIController::UpdateCheckpoint()
{
	UAIWorldSubsystem* AIWorldSystem = UAIWorldSubsystem::GetAISystem(GetWorld());
	if (!AIWorldSystem)
	{
		return;
	}

	const int TotalCheckpoints = AIWorldSystem->GetTotalCheckpoints();
	if (TotalCheckpoints <= 0)
	{
		UE_LOG(LogTemp, Error, TEXT("NO CHECKPOINTS"));
		return;
	}

	CurrentCheckpointIndex++;
	CurrentCheckpointIndex = CurrentCheckpointIndex % TotalCheckpoints;

	CurrentCheckpoint = AIWorldSystem->GetCheckpointAtIndex(CurrentCheckpointIndex);
	// Determine which node to steer towards.
	const TArray<AAICheckpointNode*>& Nodes = CurrentCheckpoint->GetAINodes();
	const int Size = Nodes.Num();
	const int RandomIndex = FMath::RandRange(0, Size - 1);
	CurrentNode = Nodes[RandomIndex];
	if (!CurrentNode)
	{
		return;
	}

	CurrentNodeLocation = GetFloorPositionFromNode(*CurrentNode);
}

void ASnowboardAIController::OnCheckpointReached()
{
	UpdateCheckpoint();
}

const FVector ASnowboardAIController::GetFloorPositionFromNode(const AAICheckpointNode& Node)
{
	
	FVector NodeLocation = Node.GetActorLocation();
	UWorld* World = GetWorld();
	if (!World)
	{
		return NodeLocation;
	}

	FHitResult HitResult;
	const float RayLength = 1000.0f;
	const FVector End = NodeLocation - FVector(0.0f, 0.0f, RayLength);
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(PossessedPawn);
	const bool bHit = World->LineTraceSingleByChannel(HitResult, NodeLocation, End, ECollisionChannel::ECC_WorldDynamic, QueryParams);
	bool bPersistent = true;
	float LifeTime = 0.0f;
	if (bHit && HitResult.bBlockingHit)
	{
		NodeLocation = HitResult.ImpactPoint;
		static float UpOffset = 100.0f;
		NodeLocation += FVector(0.0f, 0.0f, UpOffset);
	}
	return NodeLocation;
}
