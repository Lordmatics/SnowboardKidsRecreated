// Lordmatics Games

#include "SnowboardKids/CustomCharacters/SnowboardCharacterBase.h"
#include <Engine/CollisionProfile.h>
#include <Components/CapsuleComponent.h>
#include "SnowboardKids/CustomComponents/CustomCharacterMovementComponent.h"
#include <Components/SkeletalMeshComponent.h>
#include <GameFramework/SpringArmComponent.h>
#include <Camera/CameraComponent.h>
#include <Components/StaticMeshComponent.h>
#include <Materials/MaterialInstanceDynamic.h>
#include <Materials/MaterialInterface.h>
#include <Engine/Engine.h>
#include <Animation/AnimInstance.h>
#include "SnowboardKids/CustomComponents/CustomPawnMovementComponent.h"
#include "SnowboardKids/Animation/SnowboarderAnimInstance.h"
#include <Animation/AnimMontage.h>
#include "SnowboardKids/Controllers/SnowboardAIController.h"
#include <DrawDebugHelpers.h>
#include "SnowboardKids/CustomActors/Projectiles/ProjectileTable.h"
#include "SnowboardKids/Utils/GameUtils.h"
#include "SnowboardKids/Systems/WorldSystems/SnowboardCharacterSubsystem.h"
#include <Components/WidgetComponent.h>
#include "SnowboardKids/CustomWidgets/PlayerWidget.h"
#include "../CustomActors/Items/Offensive/OffensiveItem.h"
#include "../CustomActors/Items/Utility/UtilityTable.h"

// Sets default values
ASnowboardCharacterBase::ASnowboardCharacterBase(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer),
	BaseTurnRate(45.0f),
	BaseLookUpRate(45.0f),
	bMovementDisabled(false),
	bRotationDisabled(false),
	bIsAIControlled(false),
	BoardMeshes(),
	UtilityTable(nullptr),
	OffensiveTable(nullptr),
	bOverlappedFinishLine(false)
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;	
	PrimaryActorTick.bStartWithTickEnabled = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	CapsuleComponent->InitCapsuleSize(42.0f, 48.0f);
	CapsuleComponent->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);
	
	CapsuleComponent->CanCharacterStepUpOn = ECB_No;
	CapsuleComponent->SetShouldUpdatePhysicsVolume(true);
	CapsuleComponent->SetCanEverAffectNavigation(false);
	CapsuleComponent->bDynamicObstacle = true;
	RootComponent = CapsuleComponent;

	CharacterMovement = CreateDefaultSubobject<UCustomPawnMovementComponent>(TEXT("Movement"));
	CharacterMovement->UpdatedComponent = CapsuleComponent;
	//CrouchedEyeHeight = CharacterMovement->CrouchedHalfHeight * 0.80f;
	//CharacterMovement->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	//CharacterMovement->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	//CharacterMovement->JumpZVelocity = 600.f;
	//CharacterMovement->AirControl = 0.2f;
	
	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeltalMesh"));
	SkeletalMesh->AlwaysLoadOnClient = true;
	SkeletalMesh->AlwaysLoadOnServer = true;
	SkeletalMesh->bOwnerNoSee = false;
	SkeletalMesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPose;
	SkeletalMesh->bCastDynamicShadow = true;
	SkeletalMesh->bAffectDynamicIndirectLighting = true;
	SkeletalMesh->PrimaryComponentTick.TickGroup = TG_PrePhysics;
	SkeletalMesh->SetupAttachment(CapsuleComponent);
	SkeletalMesh->SetCollisionProfileName(TEXT("CharacterMesh"));
	SkeletalMesh->SetGenerateOverlapEvents(false);
	SkeletalMesh->SetCanEverAffectNavigation(false);
	SkeletalMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -45.0f), false);
	
	SnowboardMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SnowboardMesh"));
	SnowboardMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	PlayerWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PlayerWidget"));
	PlayerWidget->SetupAttachment(RootComponent);
}

USnowboarderAnimInstance* ASnowboardCharacterBase::GetAnimInstance() const
{
	if (!SkeletalMesh)
	{
		return nullptr;
	}
		
	USnowboarderAnimInstance* AnimInstance = Cast<USnowboarderAnimInstance>(SkeletalMesh->GetAnimInstance());
	return AnimInstance;
}

UStaticMesh* ASnowboardCharacterBase::GetBoardFromType(EBoardType BoardType) const
{
	return BoardMeshes.GetMeshFromType(BoardType);
}

void ASnowboardCharacterBase::OnLanded(const FHitResult& Hit)
{
	// TODO: Implement.
}

void ASnowboardCharacterBase::PlayAnimation(UAnimMontage* Montage)
{	
	USnowboarderAnimInstance* AnimInstance = GetAnimInstance();
	if (!AnimInstance)
	{
		return;
	}

	if (!Montage)
	{
		return;
	}

	if (AnimInstance->Montage_IsPlaying(Montage))
	{
		return;
	}

	AnimInstance->Montage_Play(Montage);
}

void ASnowboardCharacterBase::OnHitByProjectile(EOffensiveType OffensiveType)
{
	UE_LOG(LogTemp, Log, TEXT("%s Hit By Projectile!"), *GetName());

	UCapsuleComponent* Capsule = GetCapsuleComponent();
	if (CharacterMovement && Capsule)
	{
		if (!CharacterMovement->HasCrashed())
		{
			const FRotator& Rotation = Capsule->GetComponentRotation();
			CharacterMovement->SetProcessTrick(true);
			CharacterMovement->TriggerCrash(Rotation);
		}		
	}
}

// Called when the game starts or when spawned
void ASnowboardCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
	if (PlayerWidget)
	{
		if (UPlayerWidget* CustomWidget = Cast<UPlayerWidget>(PlayerWidget->GetWidget()))
		{
			const int TotalLaps = 3;
			// Initialise any image switches before adding to viewport.
			CustomWidget->UpdateLap(TotalLaps);

			CustomWidget->AddToPlayerScreen();
		}
	}

	if (DynamicMaterials.Num() > 0)
	{
		DynamicMaterials.Empty();
	}

	if (!SkeletalMesh)
	{
		return;
	}

	if (SnowboardMesh)
	{
		SnowboardMesh->AttachToComponent(SkeletalMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketNames::FootSocket);
	}
	const TArray<UMaterialInterface*>& Materials = SkeletalMesh->GetMaterials();
	int Index = 0;
	for (UMaterialInterface* Material : Materials)
	{
		if (!Material)
		{
			++Index;
			continue;
		}

		if (UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(Material, this))
		{
			DynamicMaterials.Push(DynamicMaterial);
			SkeletalMesh->SetMaterial(Index, DynamicMaterial);
		}
		++Index;
	}

	if (USnowboardCharacterSubsystem* SnowboardCharacterSystem = USnowboardCharacterSubsystem::GetCharacterSystem(GetWorld()))
	{
		SnowboardCharacterSystem->RegisterCharacter(this);
	}
}

void ASnowboardCharacterBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (USnowboardCharacterSubsystem* SnowboardCharacterSystem = USnowboardCharacterSubsystem::GetCharacterSystem(GetWorld()))
	{
		SnowboardCharacterSystem->UnRegisterCharacter(this);
	}
}

// Called every frame
void ASnowboardCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (UCustomPawnMovementComponent* CharacterMovementComponent = GetCharacterMovement())
	{
		const bool bMovementIgnored = CharacterMovementComponent->IsMoveInputIgnored();

		const FVector& Vel = CharacterMovementComponent->Velocity;

		const FQuat& Rotation = GetCapsuleComponent()->GetComponentQuat();
		CharacterMovementComponent->ProcessMovement(DeltaTime, Rotation);

#if defined DEBUG_SNOWBOARD_KIDS
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("Ignored: %s"), bMovementIgnored ? TEXT("True") : TEXT("False")));
			GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("Vel: X: %.1f, Y: %.1f, Z: %.1f"), Vel.X, Vel.Y, Vel.Z));
		}
#endif

		const float CurrentSpeed = CharacterMovementComponent->GetCurrentSpeed();
		//+ FVector(0.0f, 0.0f, 50.0f)
		if (!IsAIControlled())
		{
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Red, FString::Printf(TEXT("Speed: %.1f"), CurrentSpeed));
			}
		}

		//DrawDebugString(GetWorld(), GetActorLocation(), FString::Printf(TEXT("Speed: %.1f"), CurrentSpeed), this, FColor::White, DeltaTime);
	}
}

// Called to bind functionality to input
void ASnowboardCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

UPawnMovementComponent* ASnowboardCharacterBase::GetMovementComponent() const
{
	return GetCharacterMovement();
}

void ASnowboardCharacterBase::PossessedBy(AController* NewController)
{

}

void ASnowboardCharacterBase::UnPossessed()
{

}

void ASnowboardCharacterBase::ResetFinishLineOverlap()
{
	bOverlappedFinishLine = false;	

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ResetFinishLineOverlapHandle);
	}
}

void ASnowboardCharacterBase::OnRightTriggerPressed()
{
	const bool bItemShot = ConsumeItemOffensive();
}

void ASnowboardCharacterBase::OnRightTriggerReleased()
{

}

void ASnowboardCharacterBase::OnNorthPressed()
{
	if (UCustomPawnMovementComponent* CharacterMovementComponent = GetCharacterMovement())
	{
		//CharacterMovementComponent->UpdateLastKnownInput();
	}
}

void ASnowboardCharacterBase::OnNorthReleased()
{

}

void ASnowboardCharacterBase::OnEastPressed()
{

}

void ASnowboardCharacterBase::OnEastReleased()
{

}

void ASnowboardCharacterBase::OnSouthPressed()
{
	if (CharacterMovement)
	{
		CharacterMovement->TriggerCharge();
	}
}

void ASnowboardCharacterBase::OnSouthReleased()
{
	// Jump
	if (CharacterMovement)
	{
		CharacterMovement->TriggerJump();
	}
}

void ASnowboardCharacterBase::OnWestPressed()
{
	const bool bItemUsed = ConsumeItemUtility();
}

void ASnowboardCharacterBase::OnWestReleased()
{

}

void ASnowboardCharacterBase::OnDPadNorthPressed()
{
	if (CharacterMovement)
	{
		CharacterMovement->RequestGrab(ETrickDirection::North);
	}
}

void ASnowboardCharacterBase::OnDPadNorthReleased()
{
	if (CharacterMovement)
	{
		CharacterMovement->CancelGrab(ETrickDirection::North);
	}
}

void ASnowboardCharacterBase::OnDPadEastPressed()
{
	if (CharacterMovement)
	{
		CharacterMovement->RequestGrab(ETrickDirection::East);
	}
}

void ASnowboardCharacterBase::OnDPadEastReleased()
{
	if (CharacterMovement)
	{
		CharacterMovement->CancelGrab(ETrickDirection::East);
	}
}

void ASnowboardCharacterBase::OnDPadSouthPressed()
{
	if (CharacterMovement)
	{
		CharacterMovement->RequestGrab(ETrickDirection::South);
	}
}

void ASnowboardCharacterBase::OnDPadSouthReleased()
{
	if (CharacterMovement)
	{
		CharacterMovement->CancelGrab(ETrickDirection::South);
	}
}

void ASnowboardCharacterBase::OnDPadWestPressed()
{
	if (CharacterMovement)
	{
		CharacterMovement->RequestGrab(ETrickDirection::West);
	}
}

void ASnowboardCharacterBase::OnDPadWestReleased()
{
	if (CharacterMovement)
	{
		CharacterMovement->CancelGrab(ETrickDirection::West);
	}
}

void ASnowboardCharacterBase::MoveInDirection(EAxis::Type Axis, const float Value)
{
	if (!Controller)
	{
		return;
	}

	if (Value == 0.0f)
	{
		return;
	}

	if (bMovementDisabled)
	{
		return;
	}

	if (!CharacterMovement)
	{
		return;
	}

	// find out which way is forward
	const FRotator& Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	// get forward vector
	const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(Axis);

	const FVector& Velocity = GetVelocity();
	const float VectorLength = Velocity.Size();
	
	const bool bMovementIgnored = CharacterMovement->IsMoveInputIgnored();
	CharacterMovement->AddInputVector(Direction * Value, true);
}

void ASnowboardCharacterBase::MoveForward(float Value)
{
	if (!CharacterMovement)
	{
		return;
	}

	CharacterMovement->SetVerticalTrickVector(Value);
}

void ASnowboardCharacterBase::MoveRight(float Value)
{
	if (!Controller)
	{
		return;
	}

	if (bMovementDisabled)
	{
		return;
	}

	if (!CharacterMovement)
	{
		return;
	}

	//if (Value < 0.0f)
	//{
	//	UE_LOG(LogTemp, Log, TEXT("MoveRightValue: %.1f"), Value);
	//}
	
	CharacterMovement->SetHorizontalTrickVector(Value);

	if (Value == 0.0f)
	{
		return;
	}

	if (!CharacterMovement->CanTurn())
	{
		return;
	}

	FVector InputVec = FVector(0.0f, 1.0f, 0.0f);
	CharacterMovement->AddInputVector(InputVec * Value, true);
}

void ASnowboardCharacterBase::TurnAtRate(float Rate)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const float DeltaSeconds = World->GetDeltaSeconds();
	const float YawInput = Rate * BaseTurnRate * DeltaSeconds;
	AddControllerYawInput(YawInput);
}

void ASnowboardCharacterBase::LookUpAtRate(float Rate)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const float DeltaSeconds = World->GetDeltaSeconds();
	const float PitchInput = Rate * BaseLookUpRate * DeltaSeconds;

	AddControllerPitchInput(PitchInput);
}

bool ASnowboardCharacterBase::ConsumeItemOffensive()
{
	UE_LOG(LogTemp, Log, TEXT("ConsumeItemOffensive"));
	if (OffensiveTable)
	{
		FName RowName;
		GameUtils::EnumString(CurrentOffensive, RowName);		

		FString ContextString = FString::Printf(TEXT("%s"), ANSI_TO_TCHAR(__FUNCTION__));
		FProjectileTableRow* FoundRow = OffensiveTable->FindRow<FProjectileTableRow>(RowName, ContextString, false);
		if (!FoundRow)
		{
			UE_LOG(LogTemp, Log, TEXT("No Row Found with name: %s"), *RowName.ToString());
			return false;
		}

		const TSubclassOf<AOffensiveItem>& ProjectileClassToInstantiate = FoundRow->OffensiveClass;
		UWorld* World = GetWorld();
		if (!World)
		{
			UE_LOG(LogTemp, Log, TEXT("No World Found with Row Name: %s"), *RowName.ToString());
			return false;
		}

		const FVector& OwnerLocation = GetActorLocation();
		const FVector& ForwardVector = GetActorForwardVector();
		FVector SpawnLocation = OwnerLocation + (ForwardVector * 100.0f) - (FVector::UpVector * 45.0f); // Just in front of us.
		FRotator SpawnRotation = GetActorRotation();
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AOffensiveItem* SpawnedProjectile = World->SpawnActor<AOffensiveItem>(ProjectileClassToInstantiate, SpawnLocation, SpawnRotation, SpawnParameters);
		if (SpawnedProjectile)
		{
			// Deduct ammo ?
			SpawnedProjectile->SetShooter(this);
			SpawnedProjectile->OnSpawned();
			return true;
		}
	}
	return false;
}

bool ASnowboardCharacterBase::ConsumeItemUtility()
{
	UE_LOG(LogTemp, Log, TEXT("ConsumeItemUtility"));
	if (UtilityTable)
	{
		FName RowName;
		GameUtils::EnumString(CurrentUtility, RowName);

		FString ContextString = FString::Printf(TEXT("%s"), ANSI_TO_TCHAR(__FUNCTION__));
		FUtilityTableRow* FoundRow = UtilityTable->FindRow<FUtilityTableRow>(RowName, ContextString, false);
		if (!FoundRow)
		{
			UE_LOG(LogTemp, Log, TEXT("No Row Found with name: %s"), *RowName.ToString());
			return false;
		}

		const TSubclassOf<AUtilityItem>& UtilityClassToInstantiate = FoundRow->UtilityClass;
		UWorld* World = GetWorld();
		if (!World)
		{
			UE_LOG(LogTemp, Log, TEXT("No World Found with Row Name: %s"), *RowName.ToString());
			return false;
		}

		EUtilityType UtilityType = FoundRow->UtilityType;
		switch (UtilityType)
		{
			// Spawn
		case EUtilityType::Rock:
			break;
			// Spawn
		case EUtilityType::Ghost:
			break;
			// Spawn
		case EUtilityType::TripleGhost:
			break;
			// Customize Self - Suppose i could spawn it for the SFX / VFX
		case EUtilityType::Invisibility:
			break;
			// Spawn
		case EUtilityType::FryingPan:
			break;
			// Spawn
		case EUtilityType::MoneyThief:
			break;
			// Spawn
		case EUtilityType::SpeedFan:
			break;
		default:
			checkNoEntry()
			break;
		}

		const FVector& OwnerLocation = GetActorLocation();
		const FVector& ForwardVector = GetActorForwardVector();
		FVector SpawnLocation = OwnerLocation + (ForwardVector * -100.0f) - (FVector::UpVector * 45.0f); // Just in front of us.
		FRotator SpawnRotation = GetActorRotation();
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AUtilityItem* SpawnedUtility = World->SpawnActor<AUtilityItem>(UtilityClassToInstantiate, SpawnLocation, SpawnRotation, SpawnParameters);
		if (SpawnedUtility)
		{
			SpawnedUtility->SetShooter(this);
			SpawnedUtility->OnSpawned();
			return true;
		}
	}
	return false;
}

void ASnowboardCharacterBase::SetController(AController* InController)
{
	Controller = InController;	
}

bool ASnowboardCharacterBase::IsTargetable() const
{
	if (CharacterMovement)
	{
		return !CharacterMovement->HasCrashed();
	}
	return true;
}

void ASnowboardCharacterBase::CollectItem(EItemBoxType ItemType, int CoinCost)
{
	if (!PlayerWidget)
	{
		return;
	}

	UPlayerWidget* CustomWidget = Cast<UPlayerWidget>(PlayerWidget->GetWidget());
	if (!CustomWidget)
	{
		return;
	}

	const int Position = GetPositionInRace();
	switch (ItemType)
	{
	case EItemBoxType::Blue:
		// Add Random Utility - Weighted by Position In Race
		CurrentUtility = AUtilityItem::GenerateItemViaPosition(Position);
		CustomWidget->UpdateUtility(CurrentUtility);
		UE_LOG(LogTemp, Log, TEXT("CollectItem: Utility"));
		break;
	case EItemBoxType::Red:
		// Add Random Offensive - Weighted by Position In Race
		CurrentOffensive = AOffensiveItem::GenerateItemViaPosition(Position);
		CustomWidget->UpdateOffensive(CurrentOffensive);
		UE_LOG(LogTemp, Log, TEXT("CollectItem: Offensive"));
		break;
	default:
		checkNoEntry()
		break;
	}

	
	RemoveCoins(CoinCost);
}

void ASnowboardCharacterBase::OnFinishLineCrossed()
{
	if (!PlayerWidget)
	{
		return;
	}

	// Guard against multiple components overlapping simultaneously.
	if (bOverlappedFinishLine)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		bOverlappedFinishLine = true;
		World->GetTimerManager().SetTimer(ResetFinishLineOverlapHandle, this, &ASnowboardCharacterBase::ResetFinishLineOverlap, 2.0f);
	}
	
	// NOTE: This is not full proof in the sense, that you can reverse, and go through it
	// Could fix that by checking last known checkpoint.
	// BUT - Won't be a problem once we implement the lift.
	if (UPlayerWidget* CustomWidget = Cast<UPlayerWidget>(PlayerWidget->GetWidget()))
	{
		const int TotalLaps = 3;
		// Initialise any image switches before adding to viewport.
		CustomWidget->UpdateLap(TotalLaps);		
	}
}

int ASnowboardCharacterBase::GetPositionInRace() const
{
	if (PlayerWidget)
	{
		if (UPlayerWidget* CustomWidget = Cast<UPlayerWidget>(PlayerWidget->GetWidget()))
		{
			return CustomWidget->GetPosition();
		}
	}
	return -1;
}

bool ASnowboardCharacterBase::CanAfford(int Cost)
{
	if (PlayerWidget)
	{
		if (UPlayerWidget* CustomWidget = Cast<UPlayerWidget>(PlayerWidget->GetWidget()))
		{
			return CustomWidget->CanAfford(Cost);
		}
	}
	return false;
}

void ASnowboardCharacterBase::AddCoins(int Coins)
{
	if (PlayerWidget)
	{
		if (UPlayerWidget* CustomWidget = Cast<UPlayerWidget>(PlayerWidget->GetWidget()))
		{
			CustomWidget->AddCoins(Coins);
		}
	}
}

void ASnowboardCharacterBase::RemoveCoins(int Coins)
{
	if (PlayerWidget)
	{
		if (UPlayerWidget* CustomWidget = Cast<UPlayerWidget>(PlayerWidget->GetWidget()))
		{
			CustomWidget->RemoveCoins(Coins);
		}
	}
}

void ASnowboardCharacterBase::TriggerCrash()
{
	UCapsuleComponent* Capsule = GetCapsuleComponent();
	if (CharacterMovement && Capsule)
	{
		if (!CharacterMovement->HasCrashed())
		{
			const FRotator& Rotation = Capsule->GetComponentRotation();
			//CharacterMovement->SetProcessTrick(true);
			CharacterMovement->TriggerCrash(Rotation);
		}
	}
}
