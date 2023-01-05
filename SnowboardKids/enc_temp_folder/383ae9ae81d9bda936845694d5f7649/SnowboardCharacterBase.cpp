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

// Sets default values
ASnowboardCharacterBase::ASnowboardCharacterBase() :
	BaseTurnRate(45.0f),
	BaseLookUpRate(45.0f),
	bMovementDisabled(false),
	bRotationDisabled(false)
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
	SnowboardMesh->SetupAttachment(SkeletalMesh, SocketNames::FootSocket);
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

void ASnowboardCharacterBase::OnLanded(const FHitResult& Hit)
{
	// TODO: Implement.
}

// Called when the game starts or when spawned
void ASnowboardCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
	if (DynamicMaterials.Num() > 0)
	{
		DynamicMaterials.Empty();
	}

	if (!SkeletalMesh)
	{
		return;
	}

	if(SnowboardMesh)
		SnowboardMesh->SetupAttachment(SkeletalMesh, SocketNames::FootSocket);

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

	if (UCustomPawnMovementComponent* CharacterMovementComponent = GetCharacterMovement())
	{
		const bool bMovementIgnored = CharacterMovementComponent->IsMoveInputIgnored();

		const FVector& Vel = CharacterMovementComponent->Velocity;

		const FQuat& Rotation = GetCapsuleComponent()->GetComponentQuat();
		CharacterMovementComponent->ProcessMovement(DeltaTime, Rotation);

		//TODO:
		// Always moving Forward, unless crashed.
		// Input to control strafing.

		//const FVector& InputVec = CharacterMovement->ConsumeInputVector();
		//if (!InputVec.IsZero())
		//{



		//	Speed += Acceleration * DeltaTime;



		//	const FQuat& CapsuleQuat = GetCapsuleComponent()->GetComponentQuat();
		//	FHitResult OutHit;
		//	CharacterMovementComponent->SafeMoveUpdatedComponent(InputVec, CapsuleQuat, true, OutHit, ETeleportType::None);
		//}
		////else if(Speed > 0.0f)
		////{
		////	//bMovingForward = false;
		////	//bTurning = false;
		////	//static float DecelFactor = 5.0f;
		////	//Speed -= Acceleration * DecelFactor * DeltaTime;

		////}
		//Speed = FMath::Clamp(Speed, 0.0f, MaxSpeed);

		if (GEngine)
		{
			//GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("Input: X: %.1f, Y: %.1f, Z: %.1f"), InputVec.X, InputVec.Y, InputVec.Z));

			GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("Ignored: %s"), bMovementIgnored ? TEXT("True") : TEXT("False")));
			GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("Vel: X: %.1f, Y: %.1f, Z: %.1f"), Vel.X, Vel.Y, Vel.Z));
			//GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("Speed %.1f"), Speed));
		}
	}
}

// Called to bind functionality to input
void ASnowboardCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("North", IE_Pressed, this, &ASnowboardCharacterBase::OnNorthPressed);
	PlayerInputComponent->BindAction("North", IE_Released, this, &ASnowboardCharacterBase::OnNorthReleased);

	PlayerInputComponent->BindAction("East", IE_Pressed, this, &ASnowboardCharacterBase::OnEastPressed);
	PlayerInputComponent->BindAction("East", IE_Released, this, &ASnowboardCharacterBase::OnEastReleased);

	PlayerInputComponent->BindAction("South", IE_Pressed, this, &ASnowboardCharacterBase::OnSouthPressed);
	PlayerInputComponent->BindAction("South", IE_Released, this, &ASnowboardCharacterBase::OnSouthReleased);

	PlayerInputComponent->BindAction("West", IE_Pressed, this, &ASnowboardCharacterBase::OnWestPressed);
	PlayerInputComponent->BindAction("West", IE_Released, this, &ASnowboardCharacterBase::OnWestReleased);

	PlayerInputComponent->BindAxis("MoveForward", this, &ASnowboardCharacterBase::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASnowboardCharacterBase::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &ASnowboardCharacterBase::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ASnowboardCharacterBase::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &ASnowboardCharacterBase::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ASnowboardCharacterBase::LookUpAtRate);
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

void ASnowboardCharacterBase::MoveInDirection(EAxis::Type Axis, const float Value)
{
	if (!Controller)
	{
		//bMovingForward = false;
		//bTurning = false;
		return;
	}

	if (Value == 0.0f)
	{
		//bMovingForward = false;
		//bTurning = false;
		return;
	}

	if (bMovementDisabled)
	{
		//bMovingForward = false;
		//bTurning = false;
		return;
	}

	if (!CharacterMovement)
	{
		//bMovingForward = false;
		//bTurning = false;
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

	//GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("Ignored: %s"), bMovementIgnored ? TEXT("True") : TEXT("False")));

	CharacterMovement->AddInputVector(Direction * Value, true);
	/*FHitResult OutHit;
	CharacterMovement->SafeMoveUpdatedComponent(CharacterMovement->ConsumeInputVector(), GetCapsuleComponent()->GetComponentQuat(), true, OutHit, ETeleportType::None);*/
	//SafeMoveUpdatedComponent(ConsumeInputVector(), UpdatedComponent->GetComponentQuat(), true, OutHit);
}

void ASnowboardCharacterBase::MoveForward(float Value)
{
	//MoveInDirection(EAxis::X, Value);
	//bMovingForward = true;
}

void ASnowboardCharacterBase::MoveRight(float Value)
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

	FVector InputVec = FVector(0.0f, 1.0f, 0.0f);
	CharacterMovement->AddInputVector(InputVec * Value, true);

	//MoveInDirection(EAxis::Y, Value);
	//bTurning = true;
}

void ASnowboardCharacterBase::TurnAtRate(float Rate)
{
	if (bRotationDisabled)
	{
		return;
	}

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
	if (bRotationDisabled)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const float DeltaSeconds = World->GetDeltaSeconds();
	const float PitchInput = Rate * BaseLookUpRate * DeltaSeconds;

	AddControllerPitchInput(PitchInput);
}

void ASnowboardCharacterBase::AddControllerYawInput(float Value)
{
	if (bRotationDisabled)
	{
		return;
	}

	APawn::AddControllerYawInput(Value);
}

void ASnowboardCharacterBase::AddControllerPitchInput(float Value)
{
	if (bRotationDisabled)
	{
		return;
	}

	Value *= -1.0f;
	APawn::AddControllerPitchInput(Value);
}