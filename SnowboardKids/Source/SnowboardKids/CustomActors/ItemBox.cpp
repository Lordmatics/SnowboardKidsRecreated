// Lordmatics Games


#include "SnowboardKids/CustomActors/ItemBox.h"
#include <Components/StaticMeshComponent.h>
#include "SnowboardKids/CustomCharacters/SnowboardCharacterBase.h"
#include <Materials/MaterialInterface.h>

// Sets default values
AItemBox::AItemBox() :
	ItemBoxType(EItemBoxType::Red),
	YawRot(100.0f),	
	HitYawRot(-250.0f),
	MinVariance(10.0f),
	MaxVariance(30.0f),
	CoinCost(100.0f),
	InitialYawRot(100.0f),
	bTriggered(false)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	ItemBoxMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemBox"));
	ItemBoxMesh->SetupAttachment(Root);
	ItemBoxMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	ItemBoxMesh->SetGenerateOverlapEvents(true);

	ItemBoxHeaderMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemBoxHeader"));
	ItemBoxHeaderMesh->SetupAttachment(Root);
	ItemBoxHeaderMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	ItemBoxHeaderMesh->SetGenerateOverlapEvents(false);
}

// Called when the game starts or when spawned
void AItemBox::BeginPlay()
{
	Super::BeginPlay();
	
	if (ItemBoxMesh)
	{
		ItemBoxMesh->OnComponentBeginOverlap.AddDynamic(this, &AItemBox::OnTriggerOverlap);
		ItemBoxMesh->OnComponentEndOverlap.AddDynamic(this, &AItemBox::OnTriggerOverlapEnd);
	}

	const float Variance = FMath::RandRange(MinVariance, MaxVariance);
	const float RandomiseYawRot = FMath::RandRange(YawRot - Variance, YawRot + Variance);
	const float RotDif = YawRot - RandomiseYawRot;
	YawRot = RandomiseYawRot;

	InitialYawRot = YawRot;
}

void AItemBox::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (ItemBoxMesh)
	{
		ItemBoxMesh->OnComponentBeginOverlap.RemoveDynamic(this, &AItemBox::OnTriggerOverlap);
		ItemBoxMesh->OnComponentEndOverlap.RemoveDynamic(this, &AItemBox::OnTriggerOverlapEnd);
	}

	ResetItemBox();
}

void AItemBox::Tick(float DeltaSeconds)
{
	// Gently rotate the header mesh.

	// When someone interacts with it, change the material, and rotate fast for a brief period of time.

	if (ItemBoxHeaderMesh)
	{
		FRotator CurrentRotation = ItemBoxHeaderMesh->GetComponentRotation();
		CurrentRotation.Add(0.0f, DeltaSeconds * YawRot, 0.0f);
		ItemBoxHeaderMesh->SetWorldRotation(CurrentRotation);
	}
}

void AItemBox::OnTriggerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bTriggered)
	{
		return;
	}

	if (!OtherActor)
	{
		return;
	}

	ASnowboardCharacterBase* SnowboardCharacter = Cast<ASnowboardCharacterBase>(OtherActor);
	if (!SnowboardCharacter)
	{
		return;
	}
	
	const bool bCanAfford = SnowboardCharacter->CanAfford(CoinCost);
	if (!bCanAfford)
	{
		SnowboardCharacter->TriggerCrash();
		return;
	}

	CollectItem(*SnowboardCharacter);
}

void AItemBox::OnTriggerOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{

}

void AItemBox::ResetItemBox()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ItemBoxTimerHandle);
	}	

	if (ItemBoxMesh)
	{
		ItemBoxMesh->SetHiddenInGame(false);
		ItemBoxMesh->SetGenerateOverlapEvents(true);
	}

	if (ItemBoxHeaderMesh)
	{
		ItemBoxHeaderMesh->SetMaterial(0, MainMaterial);
	}

	bTriggered = false;
	YawRot = InitialYawRot;
}

void AItemBox::CollectItem(ASnowboardCharacterBase& OverlappedCharacter)
{
	OverlappedCharacter.CollectItem(ItemBoxType);

	OverlappedCharacter.RemoveCoins(CoinCost);

	bTriggered = true;
	YawRot = HitYawRot;

	if (ItemBoxHeaderMesh)
	{
		ItemBoxHeaderMesh->SetMaterial(0, ClosedMaterial);
	}
	// Give player the item.
	//ItemBoxType;

	// hide mesh via animation ?

	//UE_LOG(LogTemp, Log, TEXT("Item For: %s"), *SnowboardCharacter->GetName());

	if (ItemBoxMesh)
	{
		ItemBoxMesh->SetHiddenInGame(true);
		ItemBoxMesh->SetGenerateOverlapEvents(false);

		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(ItemBoxTimerHandle, this, &AItemBox::ResetItemBox, 2.0f);
		}
	}
}

