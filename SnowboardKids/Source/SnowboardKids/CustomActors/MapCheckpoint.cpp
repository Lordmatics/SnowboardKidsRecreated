// Lordmatics Games


#include "SnowboardKids/CustomActors/MapCheckpoint.h"
#include "SnowboardKids/CustomCharacters/SnowboardCharacterBase.h"
#include <Components/BoxComponent.h>
#include "../Systems/WorldSystems/AIWorldSubsystem.h"
#include "../Controllers/SnowboardAIController.h"

// Sets default values
AMapCheckpoint::AMapCheckpoint() :
	AINodes()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	TriggerArea = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerArea"));
	TriggerArea->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	TriggerArea->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerArea->SetGenerateOverlapEvents(true);
	TriggerArea->SetHiddenInGame(false);
	TriggerArea->SetVisibility(true);	
	TriggerArea->SetBoxExtent(FVector(350.0f, 1100.0f, 1100.0f));
	TriggerArea->SetLineThickness(50.0f);
	TriggerArea->SetComponentTickEnabled(false);
	//TriggerArea->SetNavigationAreaClass(UNavArea_Default::StaticClass());
	RootComponent = TriggerArea;

}

// Called when the game starts or when spawned
void AMapCheckpoint::BeginPlay()
{
	Super::BeginPlay();
	
	if (TriggerArea)
	{
		TriggerArea->OnComponentBeginOverlap.AddDynamic(this, &AMapCheckpoint::OnTriggerOverlap);
		TriggerArea->OnComponentEndOverlap.AddDynamic(this, &AMapCheckpoint::OnTriggerOverlapEnd);
	}

	if (UAIWorldSubsystem* AIWorldSystem = UAIWorldSubsystem::GetAISystem(GetWorld()))
	{
		AIWorldSystem->RegisterCheckpoint(this);
	}
}

void AMapCheckpoint::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (TriggerArea)
	{
		TriggerArea->OnComponentBeginOverlap.RemoveDynamic(this, &AMapCheckpoint::OnTriggerOverlap);
		TriggerArea->OnComponentEndOverlap.RemoveDynamic(this, &AMapCheckpoint::OnTriggerOverlapEnd);
	}

	if (UAIWorldSubsystem* AIWorldSystem = UAIWorldSubsystem::GetAISystem(GetWorld()))
	{
		AIWorldSystem->UnregisterCheckpoint(this);
	}
}

// Called every frame
void AMapCheckpoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMapCheckpoint::OnTriggerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor)
	{
		return;
	}

	ASnowboardCharacterBase* SnowboardCharacter = Cast<ASnowboardCharacterBase>(OtherActor);
	if (!SnowboardCharacter)
	{
		return;
	}
	
	AController* Controller = SnowboardCharacter->GetController();
	ASnowboardAIController* AIController = Cast<ASnowboardAIController>(Controller);
	if (!AIController)
	{
		return;
	}

	const bool bIsOverlapped = AIController->IsOverlapped();
	if (bIsOverlapped)
	{
		return;
	}

	AIController->SetOverlapped(true);

	UE_LOG(LogTemp, Log, TEXT("Overlap Start"));
	AIController->OnCheckpointReached();
	// Tell AI to Update their heading to the next checkpoint.
}

void AMapCheckpoint::OnTriggerOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	UE_LOG(LogTemp, Log, TEXT("Overlap End"));
	if (!OtherActor)
	{
		return;
	}

	ASnowboardCharacterBase* SnowboardCharacter = Cast<ASnowboardCharacterBase>(OtherActor);
	if (!SnowboardCharacter)
	{
		return;
	}

	AController* Controller = SnowboardCharacter->GetController();
	ASnowboardAIController* AIController = Cast<ASnowboardAIController>(Controller);
	if (!AIController)
	{
		return;
	}

	const bool bIsOverlapped = AIController->IsOverlapped();
	if (!bIsOverlapped)
	{
		return;
	}

	AIController->SetOverlapped(false);
}

