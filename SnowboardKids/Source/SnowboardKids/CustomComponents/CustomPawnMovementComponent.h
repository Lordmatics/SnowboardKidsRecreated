// Lordmatics Games

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PawnMovementComponent.h"
#include "CustomPawnMovementComponent.generated.h"

class UCameraComponent;
class USnowboarderAnimInstance;

/**
 * 
 */
UCLASS()
class SNOWBOARDKIDS_API UCustomPawnMovementComponent : public UPawnMovementComponent
{
	GENERATED_BODY()

	UCustomPawnMovementComponent();

private:
	void ProcessJump(float DeltaTime);
	void ProcessGravity(float DeltaTime);
	void ProcessAcceleration(float DeltaTime);
	void ProcessCharging(float DeltaTime);
	void ProcessForwardMovement(float DeltaTime, FQuat Rotation);
	bool IsGrounded(FHitResult& Result);
	bool GetSurfaceNormal(FHitResult& Result);
	void OnLanded();
	bool ValidateOwnerComponents();

public:
	
	void ProcessMovement(float DeltaTime, FQuat Rotation);
	void TriggerJump();
	void CancelJump();
	void TriggerCharge();
	void CancelCharge();

	/*
	* At what rate should we move forward.
	*/
	UPROPERTY(EditAnywhere, Category = "Movement")
	float ForwardSpeed;
	
	/*
	* Essentially how many units should we move sideways when rotating.
	*/
	UPROPERTY(EditAnywhere, Category = "Movement")
	float HorizontalSpeed;

	/*
	* At what rate should we build speed up to our max.
	*/
	UPROPERTY(EditAnywhere, Category = "Movement")
	float Acceleration;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float MaxSpeed;
	UPROPERTY(EditAnywhere, Category = "Movement")
	float MaxSpeedWhenCharged;

	bool bMovingForward;
	bool bTurning;
	bool bJumping;
	bool bChargedJumping;
	bool bFalling;
	bool bCharging;
	bool bCharged;
	bool bSouthInputIgnored;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float GravityScale;
	UPROPERTY(EditAnywhere, Category = "Movement")
	float JumpScale;
	UPROPERTY(EditAnywhere, Category = "Movement")
	float JumpForwardScale;

	float JumpTimer;
	float ChargeTimer;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float JumpApexTime;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float ChargeApexTime;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float AirTime;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float SphereCastRadius;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float LeftBoardRot;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float RightBoardRot;

	UPROPERTY(EditAnywhere, Category = "Movement")
		float InterpSpeedOrientToFloor;

	UPROPERTY(EditAnywhere, Category = "Movement")	
	float BankXRotLimit;
	
	UPROPERTY(EditAnywhere, Category = "Movement")
	float BankZRotLimit;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float TooSteep;

	/*
	* How much extra roll should we apply when orienting to the ground.
	*/
	UPROPERTY(EditAnywhere, Category = "Movement")
	float RollFactor;
	/*
	* How much we should adjust our forward movement, to not drive into the floor.
	*/
	UPROPERTY(EditAnywhere, Category = "Movement")
	float GroundedDirScale;

	UPROPERTY(Transient)
	UCameraComponent* Camera;

	UPROPERTY(Transient)
	USnowboarderAnimInstance* AnimInstance;

	bool bMatchRotToImpactNormal;
	FVector ImpactPoint;
	FVector ImpactNormal;
};
