// Lordmatics Games

#include "SnowboardKids/CustomActors/Coins.h"
#include "SnowboardKids/CustomCharacters/SnowboardCharacterBase.h"
#include <Components/SceneComponent.h>
#include <Components/StaticMeshComponent.h>

// Sets default values
ACoins::ACoins() :
	Super(),
	CoinAmount(100)
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	CoinMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Coin Mesh"));
	CoinMesh->SetupAttachment(Root);
	CoinMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
}

// Called when the game starts or when spawned
void ACoins::BeginPlay()
{
	Super::BeginPlay();

	if (CoinMesh)
	{
		CoinMesh->OnComponentBeginOverlap.AddDynamic(this, &ACoins::OnTriggerOverlap);
	}
}

void ACoins::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (CoinMesh)
	{
		CoinMesh->OnComponentBeginOverlap.RemoveDynamic(this, &ACoins::OnTriggerOverlap);
	}
}

void ACoins::OnTriggerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (ASnowboardCharacterBase* SnowboardCharacter = Cast<ASnowboardCharacterBase>(OtherActor))
	{
		SnowboardCharacter->AddCoins(CoinAmount);
		Destroy();
	}
}

