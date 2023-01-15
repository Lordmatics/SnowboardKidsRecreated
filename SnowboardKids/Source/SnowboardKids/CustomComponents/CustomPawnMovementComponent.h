// Lordmatics Games

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PawnMovementComponent.h"
#include "../Data/BoardData.h"
#include "../Data/TrickData.h"
#include "CustomPawnMovementComponent.generated.h"

class USnowboarderAnimInstance;
class UAnimMontage;

/**
 * 
 */
UCLASS()
class SNOWBOARDKIDS_API UCustomPawnMovementComponent : public UPawnMovementComponent
{
	GENERATED_BODY()

	UCustomPawnMovementComponent();

public:

	FORCEINLINE float GetCurrentSpeed() const { return BoardData.ForwardSpeed; }

	void SetVerticalTrickVector(float Value);
	void SetHorizontalTrickVector(float Value);
	bool CanTurn() const;

	void RequestGrab(ETrickDirection Trick);
	void CancelGrab(ETrickDirection Trick);

protected:

	virtual void BeginPlay() override;

private:
	bool ProcessCrashed(float DeltaTime, FQuat IncomingQuat);
	void ProcessJump(float DeltaTime);
	void ProcessTrick(float DeltaTime);

	void ProcessForwardRoll(APawn& Owner, float DeltaTime);
	void ProcessRightRoll(APawn& Owner, float DeltaTime);
	void ProcessBackwardsRoll(APawn& Owner, float DeltaTime);
	void ProcessLeftRoll(APawn& Owner, float DeltaTime);
	void ProcessDiagonalNE(APawn& Owner, float DeltaTime);
	void ProcessDiagonalSE(APawn& Owner, float DeltaTime);
	void ProcessDiagonalSW(APawn& Owner, float DeltaTime);
	void ProcessDiagonalNW(APawn& Owner, float DeltaTime);

	void ProcessGravity(float DeltaTime);
	void ProcessAcceleration(float DeltaTime);
	void ProcessCharging(float DeltaTime);
	void ProcessForwardMovement(float DeltaTime, FQuat IncomingQuat);
	void ProcessAdjacentObstacles(float DeltaTime);
	void ProcessDetectCollisions(float DeltaTime);
	bool IsGrounded(FHitResult& Result);
	bool GetSurfaceNormal(FHitResult& Result);
	void OnLanded();
	bool ValidateOwnerComponents();

	void ConstrainToPlaneNormal(bool Value);
	
	void SetGravityVector(const FVector& Gravity);
	FVector ConsumeGravityVector();
	void RotateBoard(APawn& Owner, const float YValue);
	void TriggerCrash(const FRotator& UpdatedRotation);
	FRotator OrientRotationToFloor(FQuat IncomingQuat, APawn& Owner, const FVector& DeltaVec, const float YValue, float& PitchResult);

	void SetProcessTrick(bool Value);

public:
	
	void ProcessMovement(float DeltaTime, FQuat IncomingQuat);
	void TriggerJump();
	void CancelJump();
	void TriggerCharge();
	void CancelCharge();

	/*
	* At what rate should we move forward.
	*/
	//UPROPERTY(EditAnywhere, Category = "Movement")
	//float ForwardSpeed;
	
	/*
	* How fast should we recoil
	*/
	UPROPERTY(EditAnywhere, Category = "Movement")
	float CrashSpeed;

	/*
	* Essentially how many units should we move sideways when rotating.
	*/
	//UPROPERTY(EditAnywhere, Category = "Movement")
	//float HorizontalSpeed;

	/*
	* At what rate should we build speed up to our max.
	*/
	//UPROPERTY(EditAnywhere, Category = "Movement")
	//float Acceleration;

	//UPROPERTY(EditAnywhere, Category = "Movement")
	//float MaxSpeed;
	UPROPERTY(EditAnywhere, Category = "Movement")
	float MaxSpeedWhenCharged;

	bool bMovingForward;
	bool bTurning;
	bool bJumping;
	bool bProcessTrick;
	bool bChargedJumping;
	bool bFalling;
	bool bCharging;
	bool bCharged;
	bool bSouthInputIgnored;
	FVector CachedForwardVector;
	FRotator CachedRotationForTrick;

	//UPROPERTY(EditAnywhere, Category = "Movement")
	//float GravityScale;
	//UPROPERTY(EditAnywhere, Category = "Movement")
	//float JumpScale;
	//UPROPERTY(EditAnywhere, Category = "Movement")
	//float JumpForwardScale;

	float JumpTimer;
	float ChargeTimer;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float CheckGroundRayLength;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float SurfaceNormalRayLength;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float CheckCollisionRayLength;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float AdjacentCollisionRayLength;

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

	bool bCrashed;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float CrashDuration;

	float CrashTimer;

	UPROPERTY(EditAnywhere, Category = "Movement")
	UAnimMontage* CrashMontage;

	UPROPERTY(EditAnywhere, Category = "Movement")
	UAnimMontage* TrickFailMontage;

	bool bAdjustedHack = false;

	//UPROPERTY(EditAnywhere, Category = "Movement", meta = (ClampMin = 0.0, ClampMax = 1.0))
	//float TurnLimit;

	UPROPERTY(Transient)
	USnowboarderAnimInstance* AnimInstance;

	bool bNoSurfaceNormalFoundThisFrame;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float MinHeightFromGround;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float MaxHeightFromGround;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float HeightFromGroundFactor;

	UPROPERTY(EditAnywhere, Category = "Movement", meta = (ClampMin = 1.0))
	float HeightAdjustScale;

	/*
	* If we detect an obstacle on our side, how much side velocity should we add.
	*/
	UPROPERTY(EditAnywhere, Category = "Movement", meta = (ClampMin = 1.0))
	float DodgeAdjacentCollisionScale;

	bool bMatchRotToImpactNormal;
	FVector ImpactPoint;
	FVector ImpactNormal;
	FVector GravityVector;
	FVector JumpVector;
	FRotator RotationLastFrame;
	FRotator CrashRot;

	float TimeBeforeGravity;
	float DelayGravityTimer;
	float TimeSpentFalling;
	float CurrentDistanceFromGround;
	float GroundDistRangeForGrabs;
	float CacheGrabDataThreshold;

	UPROPERTY(EditAnywhere, Category = "Movement")
	EBoardType BoardType;

	FBoardData BoardData;
	FTrickData TrickData;
	bool bIsPlayer;
	bool bCrashFromTrick;
	bool bIgnoreNextJumpRelease;
};
