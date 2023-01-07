// Lordmatics Games


#include "SnowboardKids/CustomActors/AICheckpointNode.h"
#include <Components/SceneComponent.h>
#include <Components/BillboardComponent.h>

// Sets default values
AAICheckpointNode::AAICheckpointNode() :
	Super()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	Billboard = CreateDefaultSubobject<UBillboardComponent>(TEXT("Billboard"));
	Billboard->SetupAttachment(Root);
}

// Called when the game starts or when spawned
void AAICheckpointNode::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AAICheckpointNode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

