// Lordmatics Games

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "SnowboardKids/Interfaces/CharacterInterface.h"
#include "SnowboardKids/Data/BoardData.h"
#include "SnowboardKids/CustomActors/Projectiles/ProjectileBase.h"
#include "SnowboardCharacterBase.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UStaticMeshComponent;
class UMaterialInstanceDynamic;
class UCapsuleComponent;
class USnowboarderAnimInstance;
class UAnimMontage;
class AProjectileBase;
class UDataTable;
class UWidgetComponent;
//class UCustomCharacterMovementComponent;
class USkeletalMeshComponent;
class UPawnMovementComponent;
class UAnimInstance;
class UCustomPawnMovementComponent;

namespace MaterialParameterNames
{
	const FName Alpha = FName(TEXT("Alpha"));
}

namespace SocketNames
{
	const FName SnowboardSocket = FName(TEXT("SnowboardSocket"));
	const FName FootSocket = FName(TEXT("FootSocket"));
}

//#define DEBUG_SNOWBOARD_KIDS

UCLASS()
class SNOWBOARDKIDS_API ASnowboardCharacterBase : public APawn, public ICharacterInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ASnowboardCharacterBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	FORCEINLINE UCapsuleComponent* GetCapsuleComponent() const { return CapsuleComponent; }
	//FORCEINLINE UCameraComponent* GetCamera() const { return ThirdPersonCamera; }
	//FORCEINLINE USpringArmComponent* GetSpringArm() const { return ThirdPersonSpringArm; }
	FORCEINLINE UStaticMeshComponent* GetSnowboard() const { return SnowboardMesh; }
	FORCEINLINE const TArray<UMaterialInstanceDynamic*>& GetDynamicMaterials() const { return DynamicMaterials; }	
	FORCEINLINE USkeletalMeshComponent* GetSkeletalMesh() const { return SkeletalMesh; }
	FORCEINLINE UCustomPawnMovementComponent* GetCharacterMovement() const { return CharacterMovement; }
	USnowboarderAnimInstance* GetAnimInstance() const;

	UStaticMesh* GetBoardFromType(EBoardType BoardType) const;

	// Called from Character Movement.
	virtual void OnLanded(const FHitResult& Hit);

	void PlayAnimation(UAnimMontage* Montage);

	void OnHitByProjectile(EProjectileType ProjectileType);

	// Inputs
	void OnRightTriggerPressed();
	void OnRightTriggerReleased();
	void OnNorthPressed();
	void OnNorthReleased();
	void OnEastPressed();
	void OnEastReleased();
	void OnSouthPressed();
	void OnSouthReleased();
	void OnWestPressed();
	void OnWestReleased();
	
	void OnDPadNorthPressed();
	void OnDPadNorthReleased();
	void OnDPadEastPressed();
	void OnDPadEastReleased();
	void OnDPadSouthPressed();
	void OnDPadSouthReleased();
	void OnDPadWestPressed();
	void OnDPadWestReleased();

	void MoveInDirection(EAxis::Type Axis, const float Value);
	void MoveForward(float Value);
	void MoveRight(float Value);
	void TurnAtRate(float Rate);
	void LookUpAtRate(float Rate);
	//virtual void AddControllerYawInput(float Value) override;
	//virtual void AddControllerPitchInput(float Value) override;
	// End Inputs

	// Interface Functions
	virtual bool ConsumeItemOffensive() override;
	virtual bool ConsumeItemUtility() override;
	// End Interface

	FORCEINLINE bool IsAIControlled() const { return bIsAIControlled; }
	FORCEINLINE void SetIsAI(bool IsAI) { bIsAIControlled = IsAI; }

	void SetController(AController* InController);

	bool IsTargetable() const;

	void OnFinishLineCrossed();

private:
	virtual void BeginPlay() override;	
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual UPawnMovementComponent* GetMovementComponent() const override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;

	UFUNCTION()
	void ResetFinishLineOverlap();

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
		UCapsuleComponent* CapsuleComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* SnowboardMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
		TArray<UMaterialInstanceDynamic*> DynamicMaterials;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
		USkeletalMeshComponent* SkeletalMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
		UCustomPawnMovementComponent* CharacterMovement;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
		UWidgetComponent* PlayerWidget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
		float BaseTurnRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
		float BaseLookUpRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
		bool bMovementDisabled;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
		bool bRotationDisabled;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
		bool bIsAIControlled;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character", meta = (AllowPrivateAccess = "true"))
		FBoardMeshes BoardMeshes;

	// TODO: Change this to a data table, where we can map the projectile type - bp asset that corresponds to it.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character", meta = (AllowPrivateAccess = "true"))
		UDataTable* ProjectileTable;

	FTimerHandle ResetFinishLineOverlapHandle;
	bool bOverlappedFinishLine;
};
