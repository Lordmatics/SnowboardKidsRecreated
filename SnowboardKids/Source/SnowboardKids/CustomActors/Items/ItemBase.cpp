// Lordmatics Games


#include "SnowboardKids/CustomActors/Items/ItemBase.h"
#include "SnowboardKids/CustomCharacters/SnowboardCharacterBase.h"

// Sets default values
AItemBase::AItemBase() :
	Shooter(nullptr)
{	
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Item Mesh"));
	ProjectileMesh->SetupAttachment(Root);
	ProjectileMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	ProjectileMesh->SetGenerateOverlapEvents(true);
}

void AItemBase::OnSpawned()
{
	UE_LOG(LogTemp, Log, TEXT("ItemSpawned: %s"), *GetName());
}

// Called when the game starts or when spawned
void AItemBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AItemBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

