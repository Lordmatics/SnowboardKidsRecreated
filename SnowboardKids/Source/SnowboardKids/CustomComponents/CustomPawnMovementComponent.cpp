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
	ForwardSpeed = 5.0f;
	CrashSpeed = 2.0f;
	HorizontalSpeed = 1.25f;
	Acceleration = 1.0f;
	MaxSpeed = 8.0f;
	MaxSpeedWhenCharged = 20.0f;
	bMovingForward = false;
	bTurning = false;
	bJumping = false;
	bFalling = false;
	bCharged = false;
	bCharging = false;
	bSouthInputIgnored = false;
	bCrashed = false;

	AirTime = 0.25f;
	SphereCastRadius = 10.0f;

	JumpTimer = 0.0f;
	JumpApexTime = 0.35f; // Variable depending on skill.

	ChargeTimer = 0.0f;
	ChargeApexTime = 2.5f;

	GravityScale = 2.75f;
	JumpScale = 3.0f;
	JumpForwardScale = 2.0f;

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

	CheckGroundRayLength = 5.0f;
	SurfaceNormalRayLength = 5.0f;
	CheckCollisionRayLength = 62.5f;

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
		return false;
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
	FVector RightVector = FVector::CrossProduct(OwnerForward, FVector::UpVector);
	FVector UpVector = FVector::CrossProduct(OwnerForward, RightVector);

	// Determine objective forward vector
	FVector LocalRightVector = FVector::CrossProduct(UpVector, FVector::ForwardVector);
	FVector LocalForwardVector = FVector::CrossProduct(UpVector, LocalRightVector);

	// Get Angle between our forward and the worlds forward.
	// Then rotate worlds forward so it points in the direction of our forward
	float DotAngle = FVector::DotProduct(OwnerForward, FVector::ForwardVector);
	float AngleInRads = FMath::Acos(DotAngle);
	float AngleInDegs = FMath::RadiansToDegrees(AngleInRads);
	FVector RotatedForward = FVector::ForwardVector.RotateAngleAxis(AngleInDegs, FVector::UpVector);
	// Might be better to construct a more horizontal forward vec
	// If we crash into something at an angle, we are going to bounce
	// Which is not right.
	FVector DeltaVec = RotatedForward * CrashSpeed;

	// Reverse For Abit.
	DeltaVec *= -1.0f;

	// Need to reset our rotation
	FRotator RecoilRotation = RotationLastFrame;
	RecoilRotation.Pitch = 0.0f;

	//FRotator UpdatedRotation = IncomingQuat.Rotator();
	//if (USceneComponent* RootComp = Owner->GetRootComponent())
	//{
	//	RootComp->SetRelativeRotation(FRotator::ZeroRotator);

	//	// Set Rotation of board to match tilt.
	//	RotateBoard(*Owner, 0.0f);

	//	FRotator RootRotation = RootComp->GetComponentRotation();

	//	// Fullpower at && YValue == 0.0f
	//	// Much less when it's a value.
	//	FVector RightVec = RootComp->GetRightVector();
	//	FVector ForwardVec = RootComp->GetForwardVector();

	//	// X
	//	float NewPitch = UKismetMathLibrary::MakeRotFromXZ(ForwardVec, ImpactNormal).Roll;
	//	// Y
	//	float NewRoll = UKismetMathLibrary::MakeRotFromYZ(RightVec, ImpactNormal).Pitch;
	//	// Z
	//	float NewYaw = RootRotation.Yaw;

	//	if (GEngine)
	//	{
	//		GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("Pitch (X): %.1f"), NewPitch));
	//		GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("Roll (Y): %.1f"), NewRoll));
	//		GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("Yaw (Z): %.1f"), NewYaw));
	//	}

	//	// If we're going downhill, subtract a number
	//	// If we're going uphill, add a number
	//	FRotator FinalRot(NewRoll, NewPitch, NewYaw);
	//	float InterpSpeed = InterpSpeedOrientToFloor;
	//	FRotator InterpedRot = UKismetMathLibrary::RInterpTo(RootRotation, FinalRot, DeltaTime, InterpSpeedOrientToFloor);
	//	InterpedRot.Roll = FMath::Clamp(InterpedRot.Roll, -BankXRotLimit, BankXRotLimit);
	//	InterpedRot.Yaw = FMath::Clamp(InterpedRot.Yaw, -BankZRotLimit, BankZRotLimit);
	//	float ExactPitch = NewRoll;
	//	InterpedRot.Pitch = ExactPitch;

	//	if (GEngine)
	//	{
	//		GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("Crashed: %s"), bCrashed ? TEXT("True") : TEXT("False")));
	//		GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("InterpedRotPitch (X): %.1f"), InterpedRot.Pitch));
	//		GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("InterpedRotRoll (Y): %.1f"), InterpedRot.Roll));
	//		GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("InterpedRotYaw (Z): %.1f"), InterpedRot.Yaw));
	//	}

	//	// We need to isolate the Z Rotation
	//	UpdatedRotation = InterpedRot;
	//	//float YawAtThisFrame = RootComp->GetComponentRotation().Yaw;
	//	UpdatedRotation.Yaw = RootRotation.Yaw; // Need to enforce this so we can freely turn.
	//}

	FHitResult OutHit;
	//DeltaVec.Normalize();
	SafeMoveUpdatedComponent(DeltaVec, RecoilRotation, true, OutHit, ETeleportType::None);

	//FVector DirToImpact = (ImpactPoint - OwnerLocation);
	//DirToImpact.Normalize();
	//float Dot = FVector::DotProduct(DeltaVec, DirToImpact);

	//if (GEngine)
	//{
	//	GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Red, FString::Printf(TEXT("DotToImpact: %.1f"), Dot));
	//}
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
		GravityVec *= GravityScale;

		bMatchRotToImpactNormal = false;
		//SetPlaneConstraintEnabled(false);
		//ImpactNormal = FVector::UpVector;
		SetGravityVector(GravityVec);	

		// NOTE: Be much better to recalibrate our characters Roll (forward/backward tilt) mid air, so we land with the board flat

	}
	else if(bFalling)
	{
		// If we were falling, but now we're grounded.
		// we effectively are 'landing' on this frame.
		ConstrainToPlaneNormal(true);
		ImpactNormal = Hitresult.ImpactNormal;
		OnLanded();
	}
	else
	{
		// Grounded - make sure our upvector matches the impact normal of the floor.
		const bool SurfaceNormalFound = GetSurfaceNormal(Hitresult);
		if(SurfaceNormalFound)
		{			
			ConstrainToPlaneNormal(true);
			ImpactNormal = Hitresult.ImpactNormal;
			//SetPlaneConstraintNormal(ImpactNormal);
		}	
		//else
		//{
		//	ConstrainToPlaneNormal(false);
		//}
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("Falling: %s"), bIsGrounded ? TEXT("False") : TEXT("True")));
		GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("OrientingToGround: %s"), bMatchRotToImpactNormal ? TEXT("True") : TEXT("False")));

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

	const FVector& InputVector = ConsumeInputVector();
	float YValue = InputVector.Y;

	// Continually move forward.
	// TODO: Handle crashing and reaccelerating etc.
	FVector DeltaVec = OwnerForward * ForwardSpeed;
	FVector DeltaCopy = FVector::ZeroVector;
	if (!bCharging && !bCharged)
	{
		FVector TurnVec = InputVector * HorizontalSpeed;
		//DeltaVec += TurnVec;
	}

	if (bFalling)
	{
		// Apply Gravity
		const FVector& Gravity = ConsumeGravityVector();
		DeltaVec.Z = 0.0f; // Need to make sure our velocity isn't larger than the gravity in the Z Direction
		// Else we will fly.
		DeltaVec += Gravity;
		DeltaCopy += Gravity;
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
	float NewPitch = 0.0f;
	FRotator UpdatedRotation = OrientRotationToFloor(IncomingQuat, *Owner, DeltaVec, YValue, NewPitch);
	FHitResult HitResult;
	ETeleportType TeleportType = ETeleportType::None;// : ETeleportType::ResetPhysics;
	bool bSweep = true;
	RotationLastFrame = UpdatedRotation;

	// Recalculate forward movement, based on new rotation.
	//FVector RotatedForward = OwnerForward;
	//if (NewPitch != 0.0f)
	//{
	//	RotatedForward = RotatedForward.RotateAngleAxis(NewPitch, RightVec);
	//	RotatedForward.Normalize();
	//}
	//DeltaCopy += RotatedForward *ForwardSpeed;	
	SafeMoveUpdatedComponent(DeltaVec, UpdatedRotation, bSweep, HitResult, TeleportType);

	// NOTE: We need to recalibrate our height, after each rotation
	// In case we go down a slope, then go back to flat surface
	// The sharp rotation will clip us under the floor.

	float OwnerHeight = Owner->GetActorLocation().Z;
	float ImpactHeight = ImpactPoint.Z;
	if (ImpactHeight >= OwnerHeight)
	{
		volatile int i = 5;
	}

	if (HitResult.bBlockingHit)
	{
		FVector HitNormal = HitResult.ImpactNormal;
		const FVector& Start = Owner->GetActorLocation();
		const FVector& ForwardVector = Owner->GetActorForwardVector();
		const FVector& RightVector = Owner->GetActorRightVector();
		float AdjustedRoll = UKismetMathLibrary::MakeRotFromXZ(ForwardVector, HitNormal).Roll;
		float AdjustedPitch = UKismetMathLibrary::MakeRotFromYZ(RightVector, HitNormal).Pitch;
		float CurrentYaw = UpdatedRotation.Yaw;
		FRotator AdjustedRot = FRotator(AdjustedPitch, CurrentYaw, AdjustedRoll);
		
		FHitResult OutHit;
		float TranslationX = (ForwardVector.X * 50.0f);
		float TranslationZ = UKismetMathLibrary::SafeDivide(FMath::Abs((Start - HitResult.ImpactPoint).Z), 2.5f);
		FVector TranslationZDelta = FVector(TranslationX, 0.0f, TranslationZ);
		SafeMoveUpdatedComponent(TranslationZDelta, AdjustedRot, true, OutHit, TeleportType);

		if (OutHit.bBlockingHit)
		{
			TriggerCrash(AdjustedRot);
		}

	}
	if(GetWorld())
		DrawDebugLine(GetWorld(), OwnerLocation, OwnerLocation + DeltaCopy, FLinearColor::Red.ToFColor(true), false, 2.0f);

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
	FVector Start = Owner->GetActorLocation();
	FVector ForwardVector = Owner->GetActorForwardVector();
	FVector RightVector = Owner->GetActorRightVector();
	//Start += FVector(0.0f, 0.0f, 5.0f);	
	const FVector End = Start + (ForwardVector * CheckCollisionRayLength);
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Owner);

	AActor* OwnerChar = Cast<AActor>(Owner);
	ActorsToIgnore.Add(OwnerChar);

	const bool bHit = UKismetSystemLibrary::SphereTraceSingle(GetOuter(), Start, End, SphereCastRadius, ETraceTypeQuery::TraceTypeQuery2, true,
		ActorsToIgnore, EDrawDebugTrace::ForOneFrame, HitResult, true);

	//const bool bHit = World->LineTraceSingleByChannel(Result, Start, End, ECollisionChannel::ECC_WorldStatic, QueryParams);
	bool bPersistent = false;
	float LifeTime = 1.0f;

	if (bHit && HitResult.bBlockingHit)
	{
		if (GEngine && HitResult.GetActor())
		{
			GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("Hit: %s"), *HitResult.GetActor()->GetName()));
		}

		// Red up to the blocking hit, green thereafter
		DrawDebugLine(World, Start, HitResult.ImpactPoint, FLinearColor::Red.ToFColor(true), bPersistent, LifeTime);
		DrawDebugLine(World, HitResult.ImpactPoint, End, FLinearColor::Green.ToFColor(true), bPersistent, LifeTime);
		DrawDebugPoint(World, HitResult.ImpactPoint, 16.0f, FLinearColor::Red.ToFColor(true), bPersistent, LifeTime);

		bCollisionAhead = true;
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("NoHit")));
		}

		DrawDebugLine(World, Start, End, FLinearColor::Red.ToFColor(true), bPersistent, LifeTime);
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

		// Okay okay okay
		// So...
		// Driving into a wall == bad = force collision
		// This will be very close to the normal being horizontal, so 1 in the x or y

		// Driving into the floor, should just have the character recalibrate to the new surface
		// This normal will be much more like 0 0 1

		// Need to distinguish between these 2 cases, to do the approrpriate handling.

		FVector WorldRight = FVector::RightVector;
		FVector WorldForward = FVector::ForwardVector;

		// If the angle between the normal and the world up vector is idk, < 90, so 3.14 / 4 radians
		// WE can assume change of surface, else trigger crash.

		// Projection = V1Dot UnitV2 * UnitV2, how much of V1 is on V2
		const float AngleBetweenNormalAndUpRads = FVector::DotProduct(CollisionImpactNormal, FVector::UpVector);
		const float AngleInDegrees = FMath::RadiansToDegrees(AngleBetweenNormalAndUpRads);
		//const float Threshold = 3.14f * 0.25f;
		const float AbsAngle = FMath::Abs(AngleInDegrees);
		if (AngleInDegrees < 90.0f && AngleInDegrees > 0.0f)
		{
			// Safe
		}
		else
		{
			TriggerCrash(RotationLastFrame);
			return;
		}
		


		float Dot = FVector::DotProduct(ForwardVector, CollisionImpactNormal);
		// -0.99 is a 90 degree wall
		// -0.92 for 100 degree wall / 10 degree slope
		// -0.98 for 80 degree wall
		// -0.93 for 70 degree wall
		//if (Dot <= -0.55f)

		// -0.9f crash
		// -0.85f driving into the floor - maybe we should just update the rotation in this scenario?
		//if (Dot <= -0.93f)
		{
			//TriggerCrash(RotationLastFrame);
		//}
		//else
		//{
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
			//bAdjustedHack = true;
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
	if (const ASnowboardCharacterBase* SnowboardCharacter = Cast<ASnowboardCharacterBase>(Owner))
	{
		if (UStaticMeshComponent* Snowboard = SnowboardCharacter->GetSnowboard())
		{
			Start = Snowboard->GetComponentLocation();
		}
	}

	const float DeltaTime = World->GetDeltaSeconds();
	//Start += FVector(0.0f, 0.0f, 5.0f);	
	const FVector End = Start + (FVector::DownVector * CheckGroundRayLength);
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Owner);

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

	FVector Start = Owner->GetActorLocation();
	if (const ASnowboardCharacterBase* SnowboardCharacter = Cast<ASnowboardCharacterBase>(Owner))
	{
		if (UStaticMeshComponent* Snowboard = SnowboardCharacter->GetSnowboard())
		{
			Start = Snowboard->GetComponentLocation();
		}
	}
	const float DeltaTime = World->GetDeltaSeconds();
	//const FVector& Start = Owner->GetActorLocation() + FVector(0.0f, 0.0f, 0.0f);
	//const FVector& Start = Snowboard->GetActorLocation() + FVector(0.0f, 0.0f, 0.0f);
	
	const FVector End = Start + (FVector::DownVector * SurfaceNormalRayLength);
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Owner);

	TArray<AActor*> ActorsToIgnore;
	AActor* OwnerChar = Cast<AActor>(Owner);
	ActorsToIgnore.Add(OwnerChar);

	//SphereCastRadius
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

void UCustomPawnMovementComponent::ConstrainToPlaneNormal(bool Value)
{
	//if (!Value)
	//{
	//	return;
	//}
	SetPlaneConstraintEnabled(Value);
	bMatchRotToImpactNormal = Value;
	if (!Value)
	{
		FVector DefaultNormal = FVector::ZeroVector;
		SetPlaneConstraintNormal(DefaultNormal);
		//ImpactNormal = DefaultNormal;
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
	
	//ConstrainToPlaneNormal(!bFalling);	

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
			RootComp->AddRelativeRotation(NewRotation);

			// Set Rotation of board to match tilt.
			//RotateBoard(Owner, YValue);
		}
	}
	else
	{
		// Set Rotation of board to match tilt.
		//RotateBoard(Owner, YValue);
	}

	RotateBoard(Owner, YValue);

	const FVector& OwnerLocation = Owner.GetActorLocation();
	FRotator RootRotation = RootComp->GetComponentRotation();
	FVector RightVec = RootComp->GetRightVector();
	FVector ForwardVec = RootComp->GetForwardVector();
	// X - Forward
	float NewRoll = UKismetMathLibrary::MakeRotFromXZ(ForwardVec, ImpactNormal).Roll;
	// Y - Right
	float NewPitch = UKismetMathLibrary::MakeRotFromYZ(RightVec, ImpactNormal).Pitch;
	PitchResult = NewPitch;
	// Z - Up
	float NewYaw = RootYaw;// RootRotation.Yaw;
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("Roll (X): %.1f"), NewRoll));
		GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("Pitch (Y): %.1f"), NewPitch));
		GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("Yaw (Z): %.1f"), NewYaw));
	}
	FRotator FinalRot(NewPitch, NewRoll, NewYaw);
	float InterpSpeed = InterpSpeedOrientToFloor;
	FRotator InterpedRot = UKismetMathLibrary::RInterpTo(RootRotation, FinalRot, DeltaTime, InterpSpeedOrientToFloor);
	InterpedRot.Roll = FMath::Clamp(InterpedRot.Roll, -BankXRotLimit, BankXRotLimit);
	InterpedRot.Yaw = FMath::Clamp(InterpedRot.Yaw, -BankZRotLimit, BankZRotLimit);	
	InterpedRot.Pitch = NewPitch;

	FVector DirToImpact = (ImpactPoint - OwnerLocation);
	DirToImpact.Normalize();
	float Dot = FVector::DotProduct(DeltaVec, DirToImpact);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Red, FString::Printf(TEXT("DotToImpact: %.1f"), Dot));
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("Crashed: %s"), bCrashed ? TEXT("True") : TEXT("False")));
		GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("InterpedRotPitch (X): %.1f"), InterpedRot.Pitch));
		GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("InterpedRotRoll (Y): %.1f"), InterpedRot.Roll));
		GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("InterpedRotYaw (Z): %.1f"), InterpedRot.Yaw));
	}

	// We need to isolate the Z Rotation
	UpdatedRotation = InterpedRot;
	if (YValue != 0.0f)
	{
		int i = 5;
	}
	UpdatedRotation.Yaw = RootComp->GetComponentRotation().Yaw;
	//UpdatedRotation.Yaw = RootRotation.Yaw; // Need to enforce this so we can freely turn.
	return UpdatedRotation;
}
