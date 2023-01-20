// Lordmatics Games


#include "SnowboardKids/CustomActors/LapArea.h"
#include "SnowboardKids/CustomCharacters/SnowboardCharacterBase.h"
#include <Components/SceneComponent.h>
#include <Components/BoxComponent.h>

// Sets default values
ALapArea::ALapArea() :
	Super()
{ 	
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	TriggerArea = CreateDefaultSubobject<UBoxComponent>(TEXT("Finish Line Area"));
	TriggerArea->SetupAttachment(Root);
	TriggerArea->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
}

// Called when the game starts or when spawned
void ALapArea::BeginPlay()
{
	Super::BeginPlay();
	
	if (TriggerArea)
	{
		TriggerArea->OnComponentBeginOverlap.AddDynamic(this, &ALapArea::OnTriggerOverlap);
	}
}

void ALapArea::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (TriggerArea)
	{
		TriggerArea->OnComponentBeginOverlap.RemoveDynamic(this, &ALapArea::OnTriggerOverlap);
	}
}

void ALapArea::OnTriggerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (ASnowboardCharacterBase* SnowboardCharacter = Cast<ASnowboardCharacterBase>(OtherActor))
	{
		SnowboardCharacter->OnFinishLineCrossed();
	}
}

