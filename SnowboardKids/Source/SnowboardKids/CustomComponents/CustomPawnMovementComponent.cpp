// Lordmatics Games


#include "SnowboardKids/CustomComponents/CustomPawnMovementComponent.h"
#include "../CustomCharacters/SnowboardCharacterBase.h"
#include <Camera/CameraComponent.h>
#include <Kismet/KismetMathLibrary.h>
#include <DrawDebugHelpers.h>

UCustomPawnMovementComponent::UCustomPawnMovementComponent()
{
	ForwardSpeed = 5.0f;
	TurnSpeed = 1.25f;
	Acceleration = 1.0f;
	MaxSpeed = 10.0f;
	MaxSpeedWhenCharged = 20.0f;
	bMovingForward = false;
	bTurning = false;
	bJumping = false;
	bFalling = false;

	JumpTimer = 0.0f;
	JumpApexTime = 2.0f; // Variable depending on skill.

	GravityScale = 1.0f;
	JumpScale = 1.0f;
}

void UCustomPawnMovementComponent::ProcessJump(float DeltaTime)
{
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

	// Add Vertical movement.
	FVector JumpVector(FVector::UpVector);
	JumpVector *= JumpScale;

	AddInputVector(JumpVector);
}

void UCustomPawnMovementComponent::ProcessGravity(float DeltaTime)
{
	if (bJumping)
	{
		return;
	}

	const bool bIsGrounded = RaycastDown();
	if (!bIsGrounded)
	{
		// Apply downward movement.
		bFalling = true;
		FVector GravityVec(FVector::DownVector);
		GravityVec *= GravityScale;

		AddInputVector(GravityVec);
	}
	else
	{
		bFalling = false;
	}
}

void UCustomPawnMovementComponent::ProcessMovement(float DeltaTime, FQuat Rotation)
{
	const APawn* Owner = GetPawnOwner();
	if (!Owner)
	{
		return;
	}

	if (!Camera)
	{
		if (const ASnowboardCharacterBase* SnowboardBase = Cast<ASnowboardCharacterBase>(Owner))
		{
			Camera = SnowboardBase->GetCamera();
		}
	}

	if (!Camera)
	{
		return;
	}

	ProcessJump(DeltaTime);
	ProcessGravity(DeltaTime);

	// Dot with camera ?
	
	const FVector& CamFoward = Camera->GetForwardVector();
	const FVector& OwnerForward = Owner->GetActorForwardVector();

	const FVector& InputVector = ConsumeInputVector();
	const float YValue = InputVector.Y;

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("Input: X: %.1f, Y: %.1f, Z: %.1f"), InputVector.X, InputVector.Y, InputVector.Z));
		GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("Right: %.1f"), YValue));
	}
	FVector DeltaVec = OwnerForward  * ForwardSpeed;
	DeltaVec += (InputVector * TurnSpeed);
	// TODO: When turning - need to rotate slightly in that direction.

	FHitResult HitResult;
	ETeleportType TeleportType = ETeleportType::None;

	// Rotation
	if (YValue != 0.0f)
	{
		//FVector Axis;
		//float AngleRadians;
		//Rotation.ToAxisAndAngle(Axis, AngleRadians);
		//// TODO: Map 0 - 1 y value, to 0 degrees -> 45 degrees.
		//// So map 0 - 1, to 1 -> 4 respectively.
		//float MappedDivisor = 1.0f * FMath::Sign(YValue);
		//const float AngleRotation = UKismetMathLibrary::SafeDivide(3.14f, MappedDivisor);
		//AngleRadians += AngleRotation; //add 45 degrees
		//Rotation = FQuat(Axis, AngleRadians);

	}

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


	SafeMoveUpdatedComponent(DeltaVec, Rotation, true, HitResult, TeleportType);	
}

void UCustomPawnMovementComponent::TriggerJump()
{
	if (bJumping)
	{
		return;
	}

	bJumping = true;
}

void UCustomPawnMovementComponent::CancelJump()
{
	bJumping = false;
	JumpTimer = 0.0f;
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
