// Lordmatics Games


#include "SnowboardKids/CustomComponents/CustomPawnMovementComponent.h"
#include "../CustomCharacters/SnowboardCharacterBase.h"
#include <Camera/CameraComponent.h>
#include <Kismet/KismetMathLibrary.h>
#include <DrawDebugHelpers.h>
#include "../Animation/SnowboarderAnimInstance.h"
#include <Kismet/KismetSystemLibrary.h>
#include <Animation/AnimMontage.h>

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
	ForwardSpeed = 1000.0f;
	CrashSpeed = 200.0f;
	HorizontalSpeed = 200.5f;
	Acceleration = 100.0f;
	MaxSpeed = 1600.0f;
	MaxSpeedWhenCharged = 2000.0f;
	bMovingForward = false;
	bTurning = false;
	bJumping = false;
	bFalling = false;
	bCharged = false;
	bCharging = false;
	bSouthInputIgnored = false;
	bCrashed = false;

	bNoSurfaceNormalFoundThisFrame = false;

	MinHeightFromGround = 54.0f;
	MaxHeightFromGround = 55.5f;
	HeightFromGroundFactor = 0.0f;
	HeightAdjustScale = 100.0f;

	AirTime = 0.0225f;
	SphereCastRadius = 30.0f;

	JumpTimer = 0.0f;
	JumpApexTime = 0.35f; // Variable depending on skill.

	ChargeTimer = 0.0f;
	ChargeApexTime = 2.5f;

	GravityScale = 300.9f;
	JumpScale = 600.0f;
	JumpForwardScale = 200.0f;

	LeftBoardRot = 5.0f;
	RightBoardRot = 12.5f;

	InterpSpeedOrientToFloor = 2.0f;
	BankXRotLimit = 5.0f;
	BankZRotLimit = 15.0f;
	TooSteep = 80.0f;
	GroundedDirScale = 1.0f;
	RollFactor = 1.2f;

	CrashTimer = 0.0f;
	CrashDuration = 1.2f;

	CheckGroundRayLength = 42.5f;
	SurfaceNormalRayLength = 100.0f;
	CheckCollisionRayLength = 70.0f;

	DodgeAdjacentCollisionScale = 125.0f;

	JumpVector = FVector::ZeroVector;
	CrashRot = FRotator::ZeroRotator;
	RotationLastFrame = FRotator::ZeroRotator;
	TurnLimit = 0.5f; // This is literally the Turning Star on the original game, higher = more control.
}

bool UCustomPawnMovementComponent::ProcessCrashed(float DeltaTime, FQuat IncomingQuat)
{
	if (!bCrashed)
	{
		CrashTimer = 0.0f;
		return false;
	}

	CrashTimer += DeltaTime;
	if (CrashTimer >= CrashDuration)
	{
		CrashTimer = 0.0f;
		bCrashed = false;
		// Nuke speed - ready to start reaccelerating
		ForwardSpeed = 0.0f;
		return true;
	}

	APawn* Owner = GetPawnOwner();
	if (!Owner || !Camera)
	{
		CrashTimer = 0.0f;
		bCrashed = false;
		return false;
	}

	const FVector& OwnerLocation = Owner->GetActorLocation();
	const FVector& CamFoward = Camera->GetForwardVector();
	const FVector& OwnerForward = Owner->GetActorForwardVector();

	// Create matrix relative to current state
	//FVector RightVector = FVector::CrossProduct(OwnerForward, FVector::UpVector);
	//FVector UpVector = FVector::CrossProduct(OwnerForward, RightVector);

	// Determine objective forward vector
	//FVector LocalRightVector = FVector::CrossProduct(UpVector, FVector::ForwardVector);
	//FVector LocalForwardVector = FVector::CrossProduct(UpVector, LocalRightVector);

	// Get Angle between our forward and the worlds forward.
	// Then rotate worlds forward so it points in the direction of our forward
	float DotAngle = FVector::DotProduct(OwnerForward, FVector::ForwardVector);
	float AngleInRads = FMath::Acos(DotAngle);
	float AngleInDegs = FMath::RadiansToDegrees(AngleInRads);
	FVector RotatedForward = FVector::ForwardVector.RotateAngleAxis(AngleInDegs, FVector::UpVector);
	// Might be better to construct a more horizontal forward vec
	// If we crash into something at an angle, we are going to bounce
	// Which is not right.
	FVector DeltaVec = OwnerForward * CrashSpeed * DeltaTime;

	// Reverse For Abit.
	DeltaVec *= -1.0f;

	// Need to reset our rotation
	FRotator RecoilRotation = RotationLastFrame;
	//RecoilRotation.Pitch = 0.0f;

	FHitResult OutHit;
	//DeltaVec.Normalize();
	SafeMoveUpdatedComponent(DeltaVec, RecoilRotation, true, OutHit, ETeleportType::None);

	if (!OutHit.bBlockingHit)
	{
		// Apply some psuedo gravity here.
		FVector PseudoGravity = FVector::DownVector * GravityScale * DeltaTime;
		SafeMoveUpdatedComponent(PseudoGravity, RecoilRotation, true, OutHit, ETeleportType::None);
	}
	else
	{
		FVector PseudoKnockback = FVector::UpVector * GravityScale * 0.5f * DeltaTime;
		SafeMoveUpdatedComponent(PseudoKnockback, RecoilRotation, true, OutHit, ETeleportType::None);
	}
	return true;
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
	FVector JmpVec(FVector::UpVector);
	JmpVec *= (JumpScale * ChargedJumpFactor * DeltaTime);

	// Only add forward movement, if we're not at max speed.
	if (ForwardSpeed <= MaxSpeed)
	{
		FVector ForwardVector(FVector::ForwardVector);
		ForwardVector *= JumpForwardScale * ChargedJumpFactor * DeltaTime;
		JmpVec += ForwardVector;
	}
	
	JumpVector = JmpVec;	

	FHitResult OutResult;
	SafeMoveUpdatedComponent(JumpVector, RotationLastFrame, true, OutResult, ETeleportType::None);
}

void UCustomPawnMovementComponent::ProcessGravity(float DeltaTime)
{
	bNoSurfaceNormalFoundThisFrame = false;

	if (bJumping)
	{
		ConstrainToPlaneNormal(false);
		return;
	}

	FHitResult Hitresult;
	const bool bIsGrounded = IsGrounded(Hitresult);
	if (!bIsGrounded)
	{
		// Apply downward movement.
		bFalling = true;
		FVector GravityVec(FVector::DownVector);
		GravityVec *= GravityScale * DeltaTime;

		bMatchRotToImpactNormal = false;
		//SetPlaneConstraintEnabled(false);
		//ImpactNormal = FVector::UpVector;
		SetGravityVector(GravityVec);	
		ConstrainToPlaneNormal(false); // NOTE: This may or may not be correct
		// NOTE: Be much better to recalibrate our characters Roll (forward/backward tilt) mid air, so we land with the board flat

	}
	else if(bFalling)
	{
		// If we were falling, but now we're grounded.
		// we effectively are 'landing' on this frame.
		ConstrainToPlaneNormal(true);
		ImpactPoint = Hitresult.ImpactPoint;
		ImpactNormal = Hitresult.ImpactNormal;

		//DrawDebugLine(GetWorld(), Hitresult.ImpactPoint, Hitresult.ImpactPoint + (ImpactNormal.Normalize() * 45.0f), FColor::Green, false, 3.0f);
		OnLanded();
	}
	else
	{
		// Grounded - make sure our upvector matches the impact normal of the floor.
		const bool SurfaceNormalFound = GetSurfaceNormal(Hitresult);
		if(SurfaceNormalFound)
		{			
			ConstrainToPlaneNormal(true);
			ImpactPoint = Hitresult.ImpactPoint;
			ImpactNormal = Hitresult.ImpactNormal;
			//DrawDebugLine(GetWorld(), Hitresult.ImpactPoint, Hitresult.ImpactPoint + (ImpactNormal.Normalize() * 40.0f), FColor::Blue, false, 3.0f);
			//SetPlaneConstraintNormal(ImpactNormal);
			bNoSurfaceNormalFoundThisFrame = false;
		}	
		else
		{
			bNoSurfaceNormalFoundThisFrame = true;
			volatile int i = 5;
			ImpactNormal = FVector::UpVector;
			ImpactPoint = GetPawnOwner()->GetActorLocation();
			//ConstrainToPlaneNormal(false);
		}
		//else
		//{
		//	ConstrainToPlaneNormal(false);
		//}
	}

#if defined DEBUG_SNOWBOARD_KIDS
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("Falling: %s"), bIsGrounded ? TEXT("False") : TEXT("True")));
		GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("OrientingToGround: %s"), bMatchRotToImpactNormal ? TEXT("True") : TEXT("False")));
	}
#endif
}

void UCustomPawnMovementComponent::ProcessAcceleration(float DeltaTime)
{
#if defined DEBUG_SNOWBOARD_KIDS
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("Speed: %.1f, Max: %.1f"), ForwardSpeed, MaxSpeed));
	}
#endif

	ForwardSpeed += Acceleration * DeltaTime;
	ForwardSpeed = FMath::Clamp(ForwardSpeed, 0.0f, MaxSpeed);
}

void UCustomPawnMovementComponent::ProcessCharging(float DeltaTime)
{
#if defined DEBUG_SNOWBOARD_KIDS
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("Charged: %s"), bCharged ? TEXT("True"):TEXT("False")));
		GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("Charging: %s, Timer: %.1f"), bCharging ? TEXT("True") : TEXT("False"), ChargeTimer));
	}
#endif

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

void UCustomPawnMovementComponent::ProcessForwardMovement(float DeltaTime, FQuat IncomingQuat)
{
	APawn* Owner = GetPawnOwner();
	if (!Owner || !Camera)
	{
		return;
	}
	
	const FVector& OwnerLocation = Owner->GetActorLocation();
	const FVector& CamFoward = Camera->GetForwardVector();
	const FVector& RightVec = Owner->GetActorRightVector();
	const FVector& OwnerForward = Owner->GetActorForwardVector();

	FVector InputVector = ConsumeInputVector();
	InputVector.Y = FMath::Clamp(InputVector.Y, -1.0f, 1.0f);
	float YValue = InputVector.Y;

	// Continually move forward.
	FVector DeltaVec = OwnerForward * ForwardSpeed * DeltaTime;
	if (!bCharging && !bCharged && !bJumping && !bFalling && !bCrashed)
	{
		if (ForwardSpeed >= MaxSpeed * 0.33f)
		{
#if defined DEBUG_SNOWBOARD_KIDS
			GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("Input (X): %.1f"), InputVector.X));
			GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("Input (Y): %.1f"), InputVector.Y));
			GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("Input (Z): %.1f"), InputVector.Z));
#endif
			FVector TurnVec = InputVector * HorizontalSpeed * DeltaTime;
			DeltaVec += TurnVec;
		}	
	}

	if (bFalling)
	{
		// Apply Gravity
		const FVector& Gravity = ConsumeGravityVector();
		DeltaVec.Z = 0.0f; // Need to make sure our velocity isn't larger than the gravity in the Z Direction
		// Else we will fly.
		DeltaVec += Gravity;
	}
	else
	{
		FVector Direction = ImpactPoint - Owner->GetActorLocation();
		float Distance = Direction.Size();
		float LocalMinHeightFromGround = MinHeightFromGround + HeightFromGroundFactor;
		float LocalMaxHeightFromGround = MaxHeightFromGround + HeightFromGroundFactor;
		if (Distance > LocalMaxHeightFromGround)
		{
			DeltaVec += FVector::DownVector * HeightAdjustScale * DeltaTime;
		}
		else if (Distance < LocalMinHeightFromGround)
		{
			DeltaVec += FVector::UpVector * HeightAdjustScale * DeltaTime;
		}

#if defined DEBUG_SNOWBOARD_KIDS
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Red, FString::Printf(TEXT("Dist To Floor: %.1f"), Distance));
		}
#endif
	}
	
	if (AnimInstance)
	{
		AnimInstance->SetSpeed(ForwardSpeed);				
		AnimInstance->SetTilt(YValue);	
#if defined DEBUG_SNOWBOARD_KIDS
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("ForwardSpeed: %.1f"), ForwardSpeed));
			GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("YValue: %.1f"), YValue));
		}
#endif
	}

	//TODO: Fix bug whereby driving down a slope, and hitting a flat plane, causing a weird rotation spike, from the MakeRot function on the pitch.
	// It seems to happen when there is no surface normal found and uses an old value for it.
	// Handle Rotating slightly based on input.
	float NewPitch = 0.0f;
	FRotator UpdatedRotation = OrientRotationToFloor(IncomingQuat, *Owner, DeltaVec, YValue, NewPitch);

	// This bounces from - 180 and 180 alot when rotating
	//float YawLastFrame = RotationLastFrame.Yaw;
	//float YawThisFrame = UpdatedRotation.Yaw;
	//float DiffYaw = FMath::Abs(YawThisFrame - YawLastFrame);
	//if (DiffYaw > 90.0f)
	//{
	//	volatile int i = 5;
	//	//UpdatedRotation.Yaw = IncomingQuat.Rotator().Yaw;
	//	//UE_LOG(LogTemp, Warning, TEXT("Yaw Delta Too Large > 90.0f, Last: %.1f, This: %.1f"), YawLastFrame, YawThisFrame);
	//}

	if (bJumping)
	{
		UpdatedRotation.Roll = IncomingQuat.Rotator().Roll;
		UpdatedRotation.Pitch = IncomingQuat.Rotator().Pitch;
	}
	FHitResult HitResult;
	ETeleportType TeleportType = ETeleportType::None;
	bool bSweep = true;

	float LastPitch = RotationLastFrame.Pitch;
	float ThisPitch = UpdatedRotation.Pitch;
	if (ThisPitch > LastPitch && ThisPitch >= 20.0f)
	{
		volatile int i = 5;
	}
	RotationLastFrame = UpdatedRotation;

	//FQuat RotAsQuat = UpdatedRotation.Quaternion();
	SafeMoveUpdatedComponent(DeltaVec, UpdatedRotation, bSweep, HitResult, TeleportType);

	//if (HitResult.bBlockingHit)
	//{
	//	FVector HitNormal = HitResult.ImpactNormal;
	//	const FVector& Start = Owner->GetActorLocation();
	//	const FVector& ForwardVector = Owner->GetActorForwardVector();
	//	const FVector& RightVector = Owner->GetActorRightVector();
	//	float AdjustedRoll = UKismetMathLibrary::MakeRotFromXZ(ForwardVector, HitNormal).Roll;
	//	float AdjustedPitch = UKismetMathLibrary::MakeRotFromYZ(RightVector, HitNormal).Pitch;
	//	float CurrentYaw = UpdatedRotation.Yaw;
	//	FRotator AdjustedRot = FRotator(AdjustedPitch, CurrentYaw, AdjustedRoll);
	//	
	//	FHitResult OutHit;
	//	//float TranslationX = (ForwardVector.X * 50.0f);
	//	float TranslationZ = UKismetMathLibrary::SafeDivide(FMath::Abs((Start - HitResult.ImpactPoint).Z), 2.5f);
	//	FVector TranslationZDelta = FVector(0.0f, 0.0f, TranslationZ);
	//	//SafeMoveUpdatedComponent(TranslationZDelta, AdjustedRot, true, OutHit, TeleportType);

	//	if (OutHit.bBlockingHit)
	//	{
	//		TriggerCrash(AdjustedRot);
	//	}
	//}
}

void UCustomPawnMovementComponent::ProcessAdjacentObstacles(float DeltaTime)
{
	bool bCollisionRight = false;
	bool bCollisionLeft = false;
	APawn* Owner = GetPawnOwner();
	if (!Owner)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FHitResult RightHitResult;
	TArray<AActor*> ActorsToIgnore;
	const FVector& Start = Owner->GetActorLocation();
	const FVector& ForwardVector = Owner->GetActorForwardVector();
	const FVector& RightVector = Owner->GetActorRightVector();
	const FVector EndLeft = Start - (RightVector * CheckCollisionRayLength);
	const FVector EndRight = Start + (RightVector * CheckCollisionRayLength);
	AActor* OwnerChar = Cast<AActor>(Owner);
	ActorsToIgnore.Add(OwnerChar);
	const bool bHitRight = UKismetSystemLibrary::SphereTraceSingle(GetOuter(), Start, EndRight, SphereCastRadius, ETraceTypeQuery::TraceTypeQuery2, true,
		ActorsToIgnore, EDrawDebugTrace::None, RightHitResult, true);

	bool bPersistent = false;
	float LifeTime = 1.0f;

	if (bHitRight && RightHitResult.bBlockingHit)
	{
		bCollisionRight = true;
	}
	FVector HitRightPosition = RightHitResult.ImpactPoint;

	FHitResult LeftHitResult;
	const bool bHitLeft = UKismetSystemLibrary::SphereTraceSingle(GetOuter(), Start, EndLeft, SphereCastRadius, ETraceTypeQuery::TraceTypeQuery2, true,
		ActorsToIgnore, EDrawDebugTrace::None, LeftHitResult, true);

	if (bHitLeft && LeftHitResult.bBlockingHit)
	{
		bCollisionLeft = true;
	}

	FVector HitLeftPosition = LeftHitResult.ImpactPoint;
	if (bCollisionLeft && bCollisionRight)
	{
		// Strafe in the direction that is further away.
		FVector DirToLeft = HitLeftPosition - Start;
		FVector DirToRight = HitRightPosition - Start;
		const float LeftDistSqr = DirToLeft.SizeSquared();
		const float RightDistSqr = DirToRight.SizeSquared();
		if (LeftDistSqr >= RightDistSqr)
		{
			// Right is closer - strafe left.
			bCollisionRight = false;
		}
		else
		{
			// Left is closer - strafe right.
			bCollisionLeft = false;
		}
	}

	FVector TranslationZDelta = FVector::ZeroVector;
	if (bCollisionRight)
	{		
		TranslationZDelta = FVector::LeftVector * DodgeAdjacentCollisionScale * DeltaTime;
	}
	else if (bCollisionLeft)
	{
		TranslationZDelta = FVector::RightVector * DodgeAdjacentCollisionScale * DeltaTime;
	}

	if (!TranslationZDelta.IsZero())
	{
		FHitResult OutHit;
		SafeMoveUpdatedComponent(TranslationZDelta, RotationLastFrame, true, OutHit, ETeleportType::None);
	}
}

void UCustomPawnMovementComponent::ProcessDetectCollisions(float DeltaTime)
{
	bool bCollisionAhead = false;
	
	APawn* Owner = GetPawnOwner();
	if (!Owner)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FHitResult HitResult;
	TArray<AActor*> ActorsToIgnore;
	const FVector& Start = Owner->GetActorLocation();
	const FVector& ForwardVector = Owner->GetActorForwardVector();
	const FVector& RightVector = Owner->GetActorRightVector();
	const FVector End = Start + (ForwardVector * CheckCollisionRayLength);
	AActor* OwnerChar = Cast<AActor>(Owner);
	ActorsToIgnore.Add(OwnerChar);
	const bool bHit = UKismetSystemLibrary::SphereTraceSingle(GetOuter(), Start, End, SphereCastRadius, ETraceTypeQuery::TraceTypeQuery2, true,
		ActorsToIgnore, EDrawDebugTrace::None, HitResult, true);

	bool bPersistent = false;
	float LifeTime = 1.0f;

	bool bIsObstacleAnotherPlayer = false;
	if (bHit && HitResult.bBlockingHit)
	{
		AActor* HitActor = HitResult.GetActor();
#if defined DEBUG_SNOWBOARD_KIDS
		if (GEngine && HitActor)
		{
			GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("Hit: %s"), *HitActor->GetName()));
		}
#endif
		bCollisionAhead = true;

		ASnowboardCharacterBase* SnowboardCharacter = Cast<ASnowboardCharacterBase>(HitActor);
		bIsObstacleAnotherPlayer = SnowboardCharacter != nullptr;
	}
	else
	{
#if defined DEBUG_SNOWBOARD_KIDS
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("NoHit")));
		}
#endif
	}

	// NOTE: Either use local forward via cross product. // Thinking this might be necessary
	// OR
	// Maybe somehow we can limit the steepness at which we stop adhering to the floors normal.
	// Or
	// Dot Product the impact normal from the ray and check steepness that way?
	// We will want to be able to drive up slopes, but at what point should we crash into a slope ?
	FVector CollisionImpactNormal = HitResult.ImpactNormal;

	if (bCollisionAhead)
	{
		FVector WorldRight = FVector::RightVector;
		FVector WorldForward = FVector::ForwardVector;
		// Projection = V1Dot UnitV2 * UnitV2, how much of V1 is on V2
		const float AngleBetweenNormalAndUpRads = FVector::DotProduct(CollisionImpactNormal, FVector::UpVector);
		const float AngleInDegrees = FMath::RadiansToDegrees(AngleBetweenNormalAndUpRads);
		const float AbsAngle = FMath::Abs(AngleInDegrees);
		if ( (AngleInDegrees < 90.0f && AngleInDegrees > 35.0f) || bIsObstacleAnotherPlayer)
		{
			// Safe
			// If we crash into another player from the front, just dont do anything.
			if (!bIsObstacleAnotherPlayer)
			{
				float Dot = FVector::DotProduct(ForwardVector, CollisionImpactNormal);

				// Recalibrate rotation Based on inpact
				// Create matrix relative to current state
				float NewRoll = UKismetMathLibrary::MakeRotFromXZ(ForwardVector, CollisionImpactNormal).Roll;
				float NewPitch = UKismetMathLibrary::MakeRotFromYZ(RightVector, CollisionImpactNormal).Pitch;
				float CurrentYaw = RotationLastFrame.Yaw;
				FRotator AdjustedRot = FRotator(NewPitch, CurrentYaw, NewRoll);

				FHitResult OutHit;
				float TranslationX = (ForwardVector.X * 50.0f);
				float TranslationZ = UKismetMathLibrary::SafeDivide(FMath::Abs((Start - HitResult.ImpactPoint).Z), 2.5f);
				FVector TranslationZDelta = FVector(TranslationX, 0.0f, TranslationZ);
				SafeMoveUpdatedComponent(TranslationZDelta, AdjustedRot, false, OutHit, ETeleportType::None);
			}
		}
		else
		{
			TriggerCrash(RotationLastFrame);
			return;
		}
	}
}

void UCustomPawnMovementComponent::ProcessMovement(float DeltaTime, FQuat IncomingQuat)
{
	if (!ValidateOwnerComponents())
	{
		return;
	}
	
	if (bAdjustedHack)
	{
		return;
	}

	if (ProcessCrashed(DeltaTime, IncomingQuat))
	{
		return;
	}

	ProcessJump(DeltaTime);
	ProcessGravity(DeltaTime);

	ProcessAcceleration(DeltaTime);
	ProcessCharging(DeltaTime);
	ProcessForwardMovement(DeltaTime, IncomingQuat);

	ProcessAdjacentObstacles(DeltaTime);
	ProcessDetectCollisions(DeltaTime);
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

	ConstrainToPlaneNormal(false);
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

	TArray<AActor*> ActorsToIgnore;
	FVector Start = Owner->GetActorLocation();
	const FVector& ForwardVector = Owner->GetActorForwardVector();
	FVector Offset = ForwardVector * 45.0f;
	FVector StartTwo = Start + Offset;

	const float DeltaTime = World->GetDeltaSeconds();
	const FVector End = Start + (FVector::DownVector * CheckGroundRayLength);
	const FVector EndTwo = StartTwo + (FVector::DownVector * CheckGroundRayLength);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Owner);

	AActor* OwnerChar = Cast<AActor>(Owner);
	ActorsToIgnore.Add(OwnerChar);

	bool bHit = UKismetSystemLibrary::SphereTraceSingle(GetOuter(), StartTwo, EndTwo, SphereCastRadius, ETraceTypeQuery::TraceTypeQuery2, true,
		ActorsToIgnore, EDrawDebugTrace::None, Result, true);

	if (!bHit)
	{
		bHit = UKismetSystemLibrary::SphereTraceSingle(GetOuter(), Start, End, SphereCastRadius, ETraceTypeQuery::TraceTypeQuery2, true,
			ActorsToIgnore, EDrawDebugTrace::None, Result, true);
	}

	bool bPersistent = true;
	float LifeTime = 0.0f;

	if (bHit && Result.bBlockingHit)
	{
#if defined DEBUG_SNOWBOARD_KIDS
		if (GEngine && Result.GetActor())
		{
			GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("Hit: %s"), *Result.GetActor()->GetName()));
		}
#endif
		return true;
	}
	else
	{
#if defined DEBUG_SNOWBOARD_KIDS
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("NoHit")));
		}
#endif

		return false;		
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

	FVector Start = Owner->GetActorLocation();
	//if (const ASnowboardCharacterBase* SnowboardCharacter = Cast<ASnowboardCharacterBase>(Owner))
	//{
	//	if (UStaticMeshComponent* Snowboard = SnowboardCharacter->GetSnowboard())
	//	{
	//		Start = Snowboard->GetComponentLocation();
	//	}
	//}
	const FVector& FowardVector = Owner->GetActorForwardVector();
	//Start = Start + (FowardVector * 35.0f);

	const float DeltaTime = World->GetDeltaSeconds();
	FVector End = Start + (FVector::DownVector * SurfaceNormalRayLength);
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Owner);

	TArray<AActor*> ActorsToIgnore;
	AActor* OwnerChar = Cast<AActor>(Owner);
	ActorsToIgnore.Add(OwnerChar);

	bool bHit = World->LineTraceSingleByChannel(Result, Start, End, ECollisionChannel::ECC_WorldDynamic, QueryParams);
	if (!bHit)
	{
		Start = Start + FowardVector * 30.0f;
		End = Start + (FVector::DownVector * SurfaceNormalRayLength);
		bHit = World->LineTraceSingleByChannel(Result, Start, End, ECollisionChannel::ECC_WorldDynamic, QueryParams);
	}
	//const bool bHit = UKismetSystemLibrary::SphereTraceSingle(GetOuter(), Start, End, SphereCastRadius, ETraceTypeQuery::TraceTypeQuery2, true,
	//	ActorsToIgnore, EDrawDebugTrace::ForOneFrame, Result, true);

	bool bPersistent = true;
	float LifeTime = 0.0f;

	if (bHit && Result.bBlockingHit)
	{
#if defined DEBUG_SNOWBOARD_KIDS
		if (GEngine && Result.GetActor())
		{
			GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("Surface Hit: %s"), *Result.GetActor()->GetName()));
		}
#endif

		// Red up to the blocking hit, green thereafter
		//DrawDebugLine(World, Start, Result.ImpactPoint, FLinearColor::Red.ToFColor(true), bPersistent, LifeTime);
		//DrawDebugLine(World, Result.ImpactPoint, End, FLinearColor::Green.ToFColor(true), bPersistent, LifeTime);
		//DrawDebugPoint(World, Result.ImpactPoint, 16.0f, FLinearColor::Red.ToFColor(true), bPersistent, LifeTime);
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
	ConstrainToPlaneNormal(true);
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

void UCustomPawnMovementComponent::ConstrainToPlaneNormal(bool Value)
{
	SetPlaneConstraintEnabled(Value);
	bMatchRotToImpactNormal = Value;
	if (!Value)
	{
		FVector DefaultNormal = FVector::ZeroVector;
		SetPlaneConstraintNormal(DefaultNormal);
	}
}

void UCustomPawnMovementComponent::SetGravityVector(const FVector& Gravity)
{
	GravityVector = Gravity;
}

FVector UCustomPawnMovementComponent::ConsumeGravityVector()
{
	FVector Gravity = GravityVector;
	GravityVector = FVector::ZeroVector;
	return Gravity;
}

void UCustomPawnMovementComponent::RotateBoard(APawn& Owner, const float YValue)
{
	if (const ASnowboardCharacterBase* SnowboardCharacter = Cast<ASnowboardCharacterBase>(&Owner))
	{
		if (UStaticMeshComponent* Snowboard = SnowboardCharacter->GetSnowboard())
		{
			FRotator BoardRotation;
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
			Snowboard->SetRelativeRotation(BoardRotation);
		}
	}
}

void UCustomPawnMovementComponent::TriggerCrash(const FRotator& UpdatedRotation)
{
	bCrashed = true;
	CrashRot = UpdatedRotation;

	APawn* Owner = GetPawnOwner();
	if (!Owner)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Play Animation	
	if (ASnowboardCharacterBase* SnowboardCharacter = Cast<ASnowboardCharacterBase>(Owner))
	{
		SnowboardCharacter->PlayAnimation(CrashMontage);
	}
}

FRotator UCustomPawnMovementComponent::OrientRotationToFloor(FQuat IncomingQuat, APawn& Owner, const FVector& DeltaVec, const float YValue, float& PitchResult)
{
	FRotator UpdatedRotation = IncomingQuat.Rotator();
	USceneComponent* RootComp = Owner.GetRootComponent();
	if (!RootComp)
	{
		return UpdatedRotation;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return UpdatedRotation;
	}

	const float DeltaTime = World->GetDeltaSeconds();
	// If there is input to strafe.
	float RootYaw = FMath::Clamp(YValue, -TurnLimit, TurnLimit);
	if (YValue != 0.0f)
	{
		static bool RotateOnHorizontalInput = true;
		if (RotateOnHorizontalInput)
		{
			// Rotate The Character
			FRotator NewRotation;
			NewRotation.Add(0.0f, RootYaw, 0.0f); // Z Will do slashes spin rotation lol.
			if (bFalling)
			{
				RootComp->AddRelativeRotation(NewRotation);
			}
			else
			{
				RootComp->AddWorldRotation(NewRotation);
			}			
		}
	}

	RotateBoard(Owner, YValue);

	const FVector& OwnerLocation = Owner.GetActorLocation();
	const FRotator& RootRotation = RootComp->GetComponentRotation();
	const FVector& RightVec = RootComp->GetRightVector();
	const FVector& ForwardVec = RootComp->GetForwardVector();
	FRotator LastFrame = RotationLastFrame;
	// X - Forward
	float NewRoll = UKismetMathLibrary::MakeRotFromXZ(ForwardVec, ImpactNormal).Roll;
	// Y - Right	
	FRotator MakeRotFromYZ = UKismetMathLibrary::MakeRotFromYZ(RightVec, ImpactNormal);
	float NewPitch = MakeRotFromYZ.Pitch;
	if (NewPitch == 90.0f)
	{
		// We've hit a seam. Do not allow this to rotate.
		NewPitch = LastFrame.Pitch;
	}
	PitchResult = NewPitch;
	// Z - Up
	float NewYaw = RootYaw;// RootRotation.Yaw;
#if defined DEBUG_SNOWBOARD_KIDS
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("Roll (X): %.1f"), NewRoll));
		GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("Pitch (Y): %.1f"), NewPitch));
		GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("Yaw (Z): %.1f"), NewYaw));
	}
#endif
	FRotator FinalRot(NewPitch, NewRoll, NewYaw);
	float InterpSpeed = InterpSpeedOrientToFloor;
	FRotator InterpedRot = UKismetMathLibrary::RInterpTo(RootRotation, FinalRot, DeltaTime, InterpSpeedOrientToFloor);
	InterpedRot.Roll = FMath::Clamp(InterpedRot.Roll, -BankXRotLimit, BankXRotLimit);
	//InterpedRot.Yaw = FMath::Clamp(InterpedRot.Yaw, -BankZRotLimit, BankZRotLimit);
	InterpedRot.Pitch = NewPitch;
	
	// DEBUG
	// Compare between this frame and teh last frame for too big of a change..
	float LastPitch = LastFrame.Pitch;
	float LastYaw = LastFrame.Yaw;
	float LastRoll = LastFrame.Roll;
	float Threshold = 30.0f;

	float PitchDiff = FMath::Abs(LastPitch - UpdatedRotation.Pitch);
	float RollDiff = FMath::Abs(LastRoll - UpdatedRotation.Roll);
	if (PitchDiff >= Threshold)
	{
		volatile int i = 5;
		InterpedRot.Pitch = LastFrame.Pitch + (PitchDiff * 0.5f);
	}

	if (RollDiff >= Threshold)
	{
		volatile int i = 5;
		InterpedRot.Roll = LastFrame.Roll + (RollDiff * 0.5f);
	}

	if (LastPitch < 0.0f && InterpedRot.Pitch > 0.0f)
	{
		//InterpedRot.Pitch = -10.0f;
		volatile int i = 5;
	}

	float PitchLimit = 25.0f;
	InterpedRot.Pitch = FMath::Clamp(InterpedRot.Pitch, -PitchLimit, PitchLimit);

#if defined DEBUG_SNOWBOARD_KIDS
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("Crashed: %s"), bCrashed ? TEXT("True") : TEXT("False")));
		GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("InterpedRotPitch (X): %.1f"), InterpedRot.Pitch));
		GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("InterpedRotRoll (Y): %.1f"), InterpedRot.Roll));
		GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("InterpedRotYaw (Z): %.1f"), InterpedRot.Yaw));
	}
#endif

	// We need to isolate the Z Rotation
	UpdatedRotation = InterpedRot;
	UpdatedRotation.Yaw = RootComp->GetComponentRotation().Yaw;
	// Need to enforce this so we can freely turn.

	return UpdatedRotation;
}
