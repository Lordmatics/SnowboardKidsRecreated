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
#include "../Controllers/SnowboardAIController.h"
#include <DrawDebugHelpers.h>

// Sets default values
ASnowboardCharacterBase::ASnowboardCharacterBase(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer),
	BaseTurnRate(45.0f),
	BaseLookUpRate(45.0f),
	bMovementDisabled(false),
	bRotationDisabled(false),
	bIsAIControlled(false),
	BoardMeshes()
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
	
	ThirdPersonSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	ThirdPersonSpringArm->SetupAttachment(RootComponent);
	ThirdPersonSpringArm->TargetArmLength = 300.0f;
	ThirdPersonSpringArm->bUsePawnControlRotation = true;
	ThirdPersonSpringArm->SetRelativeLocation(FVector(0.0f, 0.0f, 43.0f), false);

	ThirdPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	ThirdPersonCamera->SetupAttachment(ThirdPersonSpringArm, USpringArmComponent::SocketName);
	ThirdPersonCamera->bUsePawnControlRotation = false;
	
	SnowboardMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SnowboardMesh"));
	//SnowboardMesh->AttachToComponent(SkeletalMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketNames::FootSocket);
	//SnowboardMesh->SetAttachSocketName(SocketNames::FootSocket);
	//SnowboardMesh->SetAttachParent(SkeletalMesh);
	//SnowboardMesh->SetupAttachment(SkeletalMesh, SocketNames::FootSocket);
	//SnowboardMesh->SetRelativeScale3D(FVector(0.05f));
	//SnowboardMesh->SetRelativeLocation(FVector(0.326331f, 0.0f, 0.978994f));
	SnowboardMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
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

// Called when the game starts or when spawned
void ASnowboardCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	

	auto Control = GetController();

	auto CastControl = Cast<ASnowboardAIController>(GetController());

	auto SecondCast = GetController<ASnowboardAIController>();

	if (DynamicMaterials.Num() > 0)
	{
		DynamicMaterials.Empty();
	}

	if (!SkeletalMesh)
	{
		return;
	}

	if (SnowboardMesh)
	{//	SnowboardMesh->SetupAttachment(SkeletalMesh, SocketNames::FootSocket);
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

	//if (UCharacterMovementComponent* CharacterMovementComp = GetCharacterMovement())
	//{
	//	CharacterMovementComp->MaxWalkSpeed = RunSpeed;
	//	const float GravityScale = CharacterMovementComp->GravityScale;
	//	InitialGravityScale = GravityScale;

	//	const float JumpVel = CharacterMovementComp->JumpZVelocity;
	//	CharacterMovementComp->JumpZVelocity = JumpVel * GravityScale * 1.33f;
	//	InitialJumpVelocity = JumpVel * GravityScale * 1.33f;
	//}
}

// Called every frame
void ASnowboardCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//if (!IsAIControlled())
	//{
	//	return;
	//}

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
	return false;
}

bool ASnowboardCharacterBase::ConsumeItemUtility()
{
	return false;
}

void ASnowboardCharacterBase::SetController(AController* InController)
{
	Controller = InController;	
}
