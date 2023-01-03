// Lordmatics Games


#include "SnowboardKids/CustomComponents/CustomPawnMovementComponent.h"
#include "../CustomCharacters/SnowboardCharacterBase.h"
#include <Camera/CameraComponent.h>
#include <Kismet/KismetMathLibrary.h>
#include <DrawDebugHelpers.h>
#include "../Animation/SnowboarderAnimInstance.h"

	//auto map = [](float val, float valmin, float valmax, float desiredmin, float desiredmax) -> float
	//{
	//	const float denominator = valmax - valmin;
	//	if (denominator == 0.0f)
	//	{
	//		return val;
	//	}
	//	float ratio = (desiredmax - desiredmin) / denominator;
	//	return desiredmin + ratio * (val - valmin);
	//};

UCustomPawnMovementComponent::UCustomPawnMovementComponent()
{
	ForwardSpeed = 5.0f;
	HorizontalSpeed = 1.25f;
	Acceleration = 1.0f;
	MaxSpeed = 10.0f;
	MaxSpeedWhenCharged = 20.0f;
	bMovingForward = false;
	bTurning = false;
	bJumping = false;
	bFalling = false;
	bCharged = false;
	bCharging = false;
	bSouthInputIgnored = false;

	JumpTimer = 0.0f;
	JumpApexTime = 2.0f; // Variable depending on skill.

	ChargeTimer = 0.0f;
	ChargeApexTime = 2.5f;

	GravityScale = 1.0f;
	JumpScale = 1.0f;
	JumpForwardScale = 1.0f;
}

void UCustomPawnMovementComponent::ProcessJump(float DeltaTime)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("Jumping: %s"), bJumping ? TEXT("True") : TEXT("False") ));
	}

	if (!bJumping)
	{
		return;
	}

	JumpTimer += DeltaTime;
	if (JumpTimer >= JumpApexTime)
	{
		const float CumulativeHangTime = JumpApexTime + AirTime;
		if (JumpTimer >= CumulativeHangTime)
		{
			CancelJump();
		}		
		return;
	}

	// ChargedJumpFactor
	int ChargedJumpFactor = 1;
	if (bChargedJumping)
	{
		ChargedJumpFactor = 2;
	}

	// Add Vertical movement - and a bit of forward movement.
	FVector JumpVector(FVector::UpVector);
	JumpVector *= JumpScale * ChargedJumpFactor;

	// Only add forward movement, if we're not at max speed.
	if (ForwardSpeed <= MaxSpeed)
	{
		FVector ForwardVector(FVector::ForwardVector);
		ForwardVector *= JumpForwardScale * ChargedJumpFactor;
		JumpVector += ForwardVector;
	}
	
	AddInputVector(JumpVector);
}

void UCustomPawnMovementComponent::ProcessGravity(float DeltaTime)
{
	if (bJumping)
	{
		return;
	}

	const bool bIsGrounded = RaycastDown();
	
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("Falling: %s"), bIsGrounded ? TEXT("False") : TEXT("True")));
	}

	if (!bIsGrounded)
	{
		// Apply downward movement.
		bFalling = true;
		FVector GravityVec(FVector::DownVector);
		GravityVec *= GravityScale;

		AddInputVector(GravityVec);
	}
	else if(bFalling)
	{
		OnLanded();
	}
}

void UCustomPawnMovementComponent::ProcessAcceleration(float DeltaTime)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("Speed: %.1f, Max: %.1f"), ForwardSpeed, MaxSpeed));
	}

	ForwardSpeed += Acceleration * DeltaTime;
	ForwardSpeed = FMath::Clamp(ForwardSpeed, 0.0f, MaxSpeed);
}

void UCustomPawnMovementComponent::ProcessCharging(float DeltaTime)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("Charged: %s"), bCharged ? TEXT("True"):TEXT("False")));
		GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("Charging: %s, Timer: %.1f"), bCharging ? TEXT("True") : TEXT("False"), ChargeTimer));
	}

	if (bCharged)
	{
		return;
	}

	if (bCharging)
	{
		if (AnimInstance)
		{			
			AnimInstance->SetCharging(true);
		}

		ChargeTimer += DeltaTime;
		if (ChargeTimer >= ChargeApexTime)
		{
			bCharged = true;
		}
	}
}

void UCustomPawnMovementComponent::ProcessForwardMovement(float DeltaTime, FQuat Rotation)
{
	const APawn* Owner = GetPawnOwner();
	if (!Owner || !Camera)
	{
		return;
	}

	const FVector& CamFoward = Camera->GetForwardVector();
	const FVector& OwnerForward = Owner->GetActorForwardVector();

	const FVector& InputVector = ConsumeInputVector();
	float YValue = InputVector.Y;

	// Continually move forward.
	// TODO: Handle crashing and reaccelerating etc.
	FVector DeltaVec = OwnerForward * ForwardSpeed;

	if (!bCharging && !bCharged)
	{
		FVector TurnVec = InputVector * HorizontalSpeed;
		if (bJumping || bFalling)
		{
			TurnVec *= 0.5f;
		}
		DeltaVec += TurnVec;
	}
	else
	{
		YValue = 0.0f;
	}
	
	if (AnimInstance)
	{
		AnimInstance->SetSpeed(ForwardSpeed);				
		AnimInstance->SetTilt(YValue);	
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("ForwardSpeed: %.1f"), ForwardSpeed));
			GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("YValue: %.1f"), YValue));
		}
	}

	// Handle Rotating slightly based on input.
	FRotator UpdatedRotation = Rotation.Rotator();
	if (USceneComponent* RootComp = Owner->GetRootComponent())
	{
		if (YValue != 0.0f)
		{
			FRotator NewRotation;
			NewRotation.Add(0.0f, YValue, 0.0f); // Z Will do slashes spin rotation lol.
			RootComp->AddRelativeRotation(NewRotation);			
		}

		// Set Rotation of board to match tilt.
		if (const ASnowboardCharacterBase* SnowboardCharacter = Cast<ASnowboardCharacterBase>(Owner))
		{
			if (UStaticMeshComponent* Snowboard = SnowboardCharacter->GetSnowboard())
			{
				FRotator BoardRotation;
				if (YValue < 0.0f)
				{
					float ZRot = FMath::Lerp(5.0f, 0.0f, YValue);
					BoardRotation = FRotator(0.0f, 0.0f, -ZRot);
				}
				else if (YValue > 0.0f)
				{
					float ZRot = FMath::Lerp(0.0f, 12.5f, YValue);
					BoardRotation = FRotator(0.0f, 0.0f, ZRot);
				}
				else
				{
					BoardRotation = FRotator(0.0f, 0.0f, 0.0f);
				}
				Snowboard->SetRelativeRotation(BoardRotation);
			}
		}
		UpdatedRotation = RootComp->GetComponentRotation();
	}

	FHitResult HitResult;
	ETeleportType TeleportType = ETeleportType::None;
	SafeMoveUpdatedComponent(DeltaVec, UpdatedRotation, true, HitResult, TeleportType);
}

void UCustomPawnMovementComponent::ProcessMovement(float DeltaTime, FQuat Rotation)
{
	if (!ValidateOwnerComponents())
	{
		return;
	}
	
	ProcessJump(DeltaTime);
	ProcessGravity(DeltaTime);
	ProcessAcceleration(DeltaTime);
	ProcessCharging(DeltaTime);
	ProcessForwardMovement(DeltaTime, Rotation);	
}

void UCustomPawnMovementComponent::TriggerJump()
{
	if (bJumping || bFalling)
	{
		return;
	}

	if (bSouthInputIgnored)
	{
		bSouthInputIgnored = false;
		return;
	}

	if (bCharged)
	{
		bChargedJumping = true;
	}

	CancelCharge();

	bJumping = true;
}

void UCustomPawnMovementComponent::CancelJump()
{
	bJumping = false;
	bChargedJumping = false;
	JumpTimer = 0.0f;
}

void UCustomPawnMovementComponent::TriggerCharge()
{
	if (bJumping || bCharged || bFalling)
	{
		bSouthInputIgnored = true;
		return;
	}

	bCharging = true;

	if (AnimInstance)
	{
		AnimInstance->SetCharging(true);
	}
}

void UCustomPawnMovementComponent::CancelCharge()
{
	bCharged = false;
	bCharging = false;
	ChargeTimer = 0.0f;

	if (AnimInstance)
	{
		AnimInstance->SetCharging(false);
	}
}

// False - Falling
// True - Grounded
bool UCustomPawnMovementComponent::RaycastDown()
{
	const APawn* Owner = GetPawnOwner();
	if (!Owner)
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	const float DeltaTime = World->GetDeltaSeconds();
	FHitResult HitResult;
	const FVector& Start = Owner->GetActorLocation() + FVector(0.0f, 2.5f, 0.0f);
	const float RayLength = 50.0f;
	const FVector End = Start + (FVector::DownVector * RayLength);
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Owner);
	const bool bHit = World->LineTraceSingleByChannel(HitResult, Start, End, ECollisionChannel::ECC_WorldStatic, QueryParams);
	bool bPersistent = true;
	float LifeTime = 0.0f;

	if (bHit && HitResult.bBlockingHit)
	{
		if (GEngine && HitResult.GetActor())
		{
			GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("Hit: %s"), *HitResult.GetActor()->GetName()));
		}

		return true;
		// Red up to the blocking hit, green thereafter
		//DrawDebugLine(World, Start, HitResult.ImpactPoint, FLinearColor::Red.ToFColor(true), bPersistent, LifeTime);
		//DrawDebugLine(World, HitResult.ImpactPoint, End, FLinearColor::Green.ToFColor(true), bPersistent, LifeTime);
		//DrawDebugPoint(World, HitResult.ImpactPoint, 16.0f, FLinearColor::Red.ToFColor(true), bPersistent, LifeTime);
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("NoHit")));
		}

		return false;
		// no hit means all red
		//DrawDebugLine(World, Start, End, FLinearColor::Red.ToFColor(true), bPersistent, LifeTime);
	}
	return false;
}

void UCustomPawnMovementComponent::OnLanded()
{
	if (bCharged)
	{
		CancelCharge();
	}
	bFalling = false;
}

bool UCustomPawnMovementComponent::ValidateOwnerComponents()
{
	const APawn* Owner = GetPawnOwner();
	if (!Owner)
	{
		return false;
	}

	if (!Camera)
	{
		if (const ASnowboardCharacterBase* SnowboardBase = Cast<ASnowboardCharacterBase>(Owner))
		{
			Camera = SnowboardBase->GetCamera();
		}
	}

	if (!AnimInstance)
	{
		if (const ASnowboardCharacterBase* SnowboardBase = Cast<ASnowboardCharacterBase>(Owner))
		{
			AnimInstance = SnowboardBase->GetAnimInstance();
		}
	}

	if (!Camera || !AnimInstance)
	{
		return false;
	}
	return true;
}
