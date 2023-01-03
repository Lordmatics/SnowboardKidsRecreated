// Lordmatics Games

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PawnMovementComponent.h"
#include "CustomPawnMovementComponent.generated.h"

class UCameraComponent;

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
	bool RaycastDown();

public:
	
	void ProcessMovement(float DeltaTime, FQuat Rotation);
	void TriggerJump();
	void CancelJump();

	UPROPERTY(EditAnywhere, Category = "Movement")
	float ForwardSpeed;
	UPROPERTY(EditAnywhere, Category = "Movement")
	float TurnSpeed;
	UPROPERTY(EditAnywhere, Category = "Movement")
	float Acceleration;
	UPROPERTY(EditAnywhere, Category = "Movement")
	float MaxSpeed;
	UPROPERTY(EditAnywhere, Category = "Movement")
	float MaxSpeedWhenCharged;

	bool bMovingForward;
	bool bTurning;
	bool bJumping;
	bool bFalling;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float GravityScale;
	UPROPERTY(EditAnywhere, Category = "Movement")
	float JumpScale;

	float JumpTimer;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float JumpApexTime;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float AirTime;

	UPROPERTY(Transient)
	UCameraComponent* Camera;
};
