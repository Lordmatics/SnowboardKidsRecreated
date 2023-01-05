// Lordmatics Games


#include "SnowboardKids/CustomComponents/CustomPawnMovementComponent.h"
#include "../CustomCharacters/SnowboardCharacterBase.h"
#include <Camera/CameraComponent.h>
#include <Kismet/KismetMathLibrary.h>
#include <DrawDebugHelpers.h>
#include "../Animation/SnowboarderAnimInstance.h"
#include <Kismet/KismetSystemLibrary.h>

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

	SphereCastRadius = 25.0f;

	JumpTimer = 0.0f;
	JumpApexTime = 2.0f; // Variable depending on skill.

	ChargeTimer = 0.0f;
	ChargeApexTime = 2.5f;

	GravityScale = 1.0f;
	JumpScale = 1.0f;
	JumpForwardScale = 1.0f;

	LeftBoardRot = 5.0f;
	RightBoardRot = 12.5f;

	InterpSpeedOrientToFloor = 2.0f;
	BankXRotLimit = 10.0f;
	BankZRotLimit = 20.0f;
	TooSteep = 15.0f;
	GroundedDirScale = 1.0f;
	RollFactor = 1.2f;
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

	FHitResult Hitresult;
	if (GetSurfaceNormal(Hitresult))
	{
		bMatchRotToImpactNormal = true;
		ImpactNormal = Hitresult.ImpactNormal;

		// THIS LINE FIXES SO MUCH OMG
		SetPlaneConstraintNormal(ImpactNormal);
	}

	if (bJumping)
	{
		bMatchRotToImpactNormal = false;
		return;
	}

	const bool bIsGrounded = IsGrounded(Hitresult);
	
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("Falling: %s"), bIsGrounded ? TEXT("False") : TEXT("True")));
		GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("OrientingToGround: %s"), bMatchRotToImpactNormal ? TEXT("True") : TEXT("False")));
		
	}

	if (!bIsGrounded)
	{
		// Apply downward movement.
		bFalling = true;
		FVector GravityVec(FVector::DownVector);
		GravityVec *= GravityScale;

		bMatchRotToImpactNormal = false;
		AddInputVector(GravityVec);
	}
	else if(bFalling)
	{
		bMatchRotToImpactNormal = false;
		OnLanded();
	}
	else
	{
		// Grounded - make sure our upvector matches the impact normal of the floor.
		if (GetSurfaceNormal(Hitresult))
		{
			bMatchRotToImpactNormal = true;
			ImpactNormal = Hitresult.ImpactNormal;
		}	
		else
		{
			bMatchRotToImpactNormal = false;
		}
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
	APawn* Owner = GetPawnOwner();
	if (!Owner || !Camera)
	{
		return;
	}
	
	const FVector& OwnerLocation = Owner->GetActorLocation();
	const FVector& CamFoward = Camera->GetForwardVector();
	const FVector& OwnerForward = Owner->GetActorForwardVector();

	const FVector& InputVector = ConsumeInputVector();
	float YValue = InputVector.Y;

	// Continually move forward.
	// TODO: Handle crashing and reaccelerating etc.
	FVector DeltaVec = OwnerForward * ForwardSpeed;

	if (bMatchRotToImpactNormal)
	{
		// This should ensure we are always driving to not be steered into the floor.
		//FVector ImpactDir = ImpactPoint - Owner->GetActorLocation();
		//DeltaVec += ImpactPoint.GetSafeNormal() * GroundedDirScale;
	}	

	if (!bCharging && !bCharged)
	{
		FVector TurnVec = InputVector * HorizontalSpeed;
		if (bJumping || bFalling)
		{
			//TurnVec *= 0.5f;
		}
		DeltaVec += TurnVec;
	}
	else
	{
		FVector GravityVec = InputVector * GravityScale;
		DeltaVec += GravityVec;

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

			if (USkeletalMeshComponent* SkeletalMesh = SnowboardCharacter->GetSkeletalMesh())
			{
				//FRotator MeshRotation;
				//if (bMatchRotToImpactNormal)
				//{
				//	FRotator RootRotation = SkeletalMesh->GetComponentRotation();
				//	FVector RightVec = SkeletalMesh->GetRightVector();
				//	FVector ForwardVec = SkeletalMesh->GetForwardVector();
				//	float NewRoll = UKismetMathLibrary::MakeRotFromYZ(RightVec, ImpactNormal).Pitch;
				//	float NewPitch = UKismetMathLibrary::MakeRotFromXZ(ForwardVec, ImpactNormal).Roll;
				//	float NewYaw = RootRotation.Yaw;
				//	FRotator FinalRot(NewRoll, NewPitch, NewYaw);
				//	const float InterpSpeed = 2.0f;
				//	FRotator InterpedRot = UKismetMathLibrary::RInterpTo(RootRotation, FinalRot, DeltaTime, InterpSpeed);
				//	MeshRotation = InterpedRot;
				//}
				//else
				//{
				//	MeshRotation = FRotator::ZeroRotator;
				//}
				//SkeletalMesh->SetRelativeRotation(FRotator::ZeroRotator);

			}

			if (UStaticMeshComponent* Snowboard = SnowboardCharacter->GetSnowboard())
			{
				FRotator BoardRotation;

				
				{
					if (YValue < 0.0f)
					{
						float ZRot = FMath::Lerp(LeftBoardRot, 0.0f, YValue);
						BoardRotation = FRotator(0.0f, 0.0f, -ZRot);
					}
					else if (YValue > 0.0f)
					{
						float ZRot = FMath::Lerp(0.0f, RightBoardRot, YValue);
						BoardRotation = FRotator(0.0f, 0.0f, ZRot);
					}
					else
					{
						BoardRotation = FRotator(0.0f, 0.0f, 0.0f);
					}
				}


				Snowboard->SetRelativeRotation(BoardRotation);
			}
		}
		
		FRotator RootRotation = RootComp->GetComponentRotation();
		//if (bMatchRotToImpactNormal)
		{
			// Fullpower at && YValue == 0.0f
			// Much less when it's a value.
			FVector RightVec = RootComp->GetRightVector();
			FVector ForwardVec = RootComp->GetForwardVector();
			
			// X
			float NewPitch = UKismetMathLibrary::MakeRotFromXZ(ForwardVec, ImpactNormal).Roll;
			// Y
			float NewRoll = UKismetMathLibrary::MakeRotFromYZ(RightVec, ImpactNormal).Pitch;
			// Z
			float NewYaw = RootRotation.Yaw;

			//NewPitch = FMath::Clamp(NewPitch, -BankXRotLimit, BankXRotLimit);
			//NewYaw = FMath::Clamp(NewYaw, -BankZRotLimit, BankZRotLimit);

			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("Pitch (X): %.1f"), NewPitch));
				GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("Roll (Y): %.1f"), NewRoll));
				GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("Yaw (Z): %.1f"), NewYaw));
			}

			// If we're going downhill, subtract a number
			// If we're going uphill, add a number
			FRotator FinalRot(NewRoll, NewPitch, NewYaw);	
			float InterpSpeed = InterpSpeedOrientToFloor;
			if (YValue != 0.0f)
			{
				//InterpSpeed *= FMath::Abs(YValue);
			}

			FRotator InterpedRot = UKismetMathLibrary::RInterpTo(RootRotation, FinalRot, DeltaTime, InterpSpeedOrientToFloor);
			InterpedRot.Roll = FMath::Clamp(InterpedRot.Roll, -BankXRotLimit, BankXRotLimit);
			InterpedRot.Yaw = FMath::Clamp(InterpedRot.Yaw, -BankZRotLimit, BankZRotLimit);
			float ExactPitch = NewRoll;
			//if (FMath::Sign(ExactPitch) >= 0)
			//{
			//	ExactPitch *= RollFactor;
			//}
			InterpedRot.Pitch = ExactPitch;
			if (NewRoll > TooSteep)
			{
				// Trigger Collision / knock down
			}

			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("InterpedRotPitch (X): %.1f"), InterpedRot.Pitch));
				GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("InterpedRotRoll (Y): %.1f"), InterpedRot.Roll));
				GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("InterpedRotYaw (Z): %.1f"), InterpedRot.Yaw));
			}
						
			// We need to isolate the Z Rotation
			UpdatedRotation = InterpedRot;
			UpdatedRotation.Yaw = RootRotation.Yaw; // Need to enforce this so we can freely turn.
		}
		//else
		{
			//const float InterpSpeed = 2.0f;
			//FRotator InterpedRot = UKismetMathLibrary::RInterpTo(RootRotation, FRotator::ZeroRotator, DeltaTime, InterpSpeed);
			//UpdatedRotation = InterpedRot;
		}			
	}

	FHitResult HitResult;
	ETeleportType TeleportType = ETeleportType::None;
	
	// TODO: Try to get our vertical position to snap to the impact points height.
	FVector DirToImpact = (ImpactPoint - OwnerLocation);
	DirToImpact.Normalize();
	float Dot = FVector::DotProduct(DeltaVec, DirToImpact);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Red, FString::Printf(TEXT("DotToImpact: %.1f"), Dot));
	}
	SafeMoveUpdatedComponent(DeltaVec, UpdatedRotation, false, HitResult, TeleportType);

	//if (bMatchRotToImpactNormal)
	//{
	//	const FVector& NewLocation = Owner->GetActorLocation();
	//	Owner->SetActorLocation(FVector(NewLocation.X, NewLocation.Y, ImpactPoint.Z));
	//	//DeltaVec.Z = ImpactPoint.Z * DeltaTime;
	//}
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
bool UCustomPawnMovementComponent::IsGrounded(FHitResult& Result)
{
	APawn* Owner = GetPawnOwner();
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
	const FVector& Start = Owner->GetActorLocation();// +FVector(-25.0f, 0.0f, 25.0f);
	const float RayLength = 50.0f;
	const FVector End = Start + (FVector::DownVector * RayLength);
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Owner);

	TArray<AActor*> ActorsToIgnore;
	AActor* OwnerChar = Cast<AActor>(Owner);
	ActorsToIgnore.Add(OwnerChar);

	const bool bHit = UKismetSystemLibrary::SphereTraceSingle(GetOuter(), Start, End, SphereCastRadius, ETraceTypeQuery::TraceTypeQuery2, true,
		ActorsToIgnore, EDrawDebugTrace::ForOneFrame, Result, true);


	//const bool bHit = World->LineTraceSingleByChannel(Result, Start, End, ECollisionChannel::ECC_WorldStatic, QueryParams);
	bool bPersistent = true;
	float LifeTime = 0.0f;

	if (bHit && Result.bBlockingHit)
	{
		if (GEngine && Result.GetActor())
		{
			GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("Hit: %s"), *Result.GetActor()->GetName()));
		}
		ImpactPoint = Result.ImpactPoint;
		// Probably need to update the rotation via the impact normal
		
		// Red up to the blocking hit, green thereafter
		DrawDebugLine(World, Start, Result.ImpactPoint, FLinearColor::Red.ToFColor(true), bPersistent, LifeTime);
		DrawDebugLine(World, Result.ImpactPoint, End, FLinearColor::Green.ToFColor(true), bPersistent, LifeTime);
		DrawDebugPoint(World, Result.ImpactPoint, 16.0f, FLinearColor::Red.ToFColor(true), bPersistent, LifeTime);
		return true;
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("NoHit")));
		}

		DrawDebugLine(World, Start, End, FLinearColor::Red.ToFColor(true), bPersistent, LifeTime);
		return false;
		// no hit means all red
		
	}
	return false;
}

bool UCustomPawnMovementComponent::GetSurfaceNormal(FHitResult& Result)
{
	APawn* Owner = GetPawnOwner();
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
	const FVector& Start = Owner->GetActorLocation() + FVector(10.0f, 0.0f, 0.0f);
	const float RayLength = 75.0f;
	const FVector End = Start + (FVector::DownVector * RayLength);
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Owner);

	TArray<AActor*> ActorsToIgnore;
	AActor* OwnerChar = Cast<AActor>(Owner);
	ActorsToIgnore.Add(OwnerChar);

	const bool bHit = UKismetSystemLibrary::SphereTraceSingle(GetOuter(), Start, End, SphereCastRadius, ETraceTypeQuery::TraceTypeQuery2, true,
		ActorsToIgnore, EDrawDebugTrace::ForOneFrame, Result, true);


	//const bool bHit = World->LineTraceSingleByChannel(Result, Start, End, ECollisionChannel::ECC_WorldStatic, QueryParams);
	bool bPersistent = true;
	float LifeTime = 0.0f;

	if (bHit && Result.bBlockingHit)
	{
		if (GEngine && Result.GetActor())
		{
			GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("Surface Hit: %s"), *Result.GetActor()->GetName()));
		}

		// Probably need to update the rotation via the impact normal

		// Red up to the blocking hit, green thereafter
		DrawDebugLine(World, Start, Result.ImpactPoint, FLinearColor::Red.ToFColor(true), bPersistent, LifeTime);
		DrawDebugLine(World, Result.ImpactPoint, End, FLinearColor::Green.ToFColor(true), bPersistent, LifeTime);
		DrawDebugPoint(World, Result.ImpactPoint, 16.0f, FLinearColor::Red.ToFColor(true), bPersistent, LifeTime);
		return true;
	}
	else
	{
		return false;	
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
