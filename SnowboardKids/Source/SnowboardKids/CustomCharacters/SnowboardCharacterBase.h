// Lordmatics Games

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "SnowboardCharacterBase.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UStaticMeshComponent;
class UMaterialInstanceDynamic;
class UCapsuleComponent;
class USnowboarderAnimInstance;
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

UCLASS()
class SNOWBOARDKIDS_API ASnowboardCharacterBase : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ASnowboardCharacterBase();

	FORCEINLINE UCapsuleComponent* GetCapsuleComponent() const { return CapsuleComponent; }
	FORCEINLINE UCameraComponent* GetCamera() const { return ThirdPersonCamera; }
	FORCEINLINE USpringArmComponent* GetSpringArm() const { return ThirdPersonSpringArm; }
	FORCEINLINE UStaticMeshComponent* GetSnowboard() const { return SnowboardMesh; }
	FORCEINLINE const TArray<UMaterialInstanceDynamic*>& GetDynamicMaterials() const { return DynamicMaterials; }	
	FORCEINLINE USkeletalMeshComponent* GetSkeletalMesh() const { return SkeletalMesh; }
	FORCEINLINE UCustomPawnMovementComponent* GetCharacterMovement() const { return CharacterMovement; }
	USnowboarderAnimInstance* GetAnimInstance() const;

	// Called from Character Movement.
	virtual void OnLanded(const FHitResult& Hit);

private:
	virtual void BeginPlay() override;	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual UPawnMovementComponent* GetMovementComponent() const override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;
	
	UFUNCTION()
	void OnNorthPressed();

	UFUNCTION()
	void OnNorthReleased();

	UFUNCTION()
	void OnEastPressed();

	UFUNCTION()
	void OnEastReleased();

	UFUNCTION()
	void OnSouthPressed();

	UFUNCTION()
	void OnSouthReleased();

	UFUNCTION()
	void OnWestPressed();

	UFUNCTION()
	void OnWestReleased();

	void MoveInDirection(EAxis::Type Axis, const float Value);
	void MoveForward(float Value);
	void MoveRight(float Value);
	void TurnAtRate(float Rate);
	void LookUpAtRate(float Rate);
	virtual void AddControllerYawInput(float Value) override;
	virtual void AddControllerPitchInput(float Value) override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
		UCapsuleComponent* CapsuleComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
		UCameraComponent* ThirdPersonCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
		USpringArmComponent* ThirdPersonSpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* SnowboardMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
		TArray<UMaterialInstanceDynamic*> DynamicMaterials;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
		USkeletalMeshComponent* SkeletalMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
		UCustomPawnMovementComponent* CharacterMovement;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
		float BaseTurnRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
		float BaseLookUpRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
		bool bMovementDisabled;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
		bool bRotationDisabled;


};
