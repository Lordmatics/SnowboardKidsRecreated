// Lordmatics Games


#include "SnowboardKids/CustomComponents/CustomPawnMovementComponent.h"
#include "../CustomCharacters/SnowboardCharacterBase.h"
#include <Kismet/KismetMathLibrary.h>
#include <DrawDebugHelpers.h>
#include "../Animation/SnowboarderAnimInstance.h"
#include <Kismet/KismetSystemLibrary.h>
#include <Animation/AnimMontage.h>
#include "../Controllers/SnowboardPlayerController.h"

UCustomPawnMovementComponent::UCustomPawnMovementComponent() :
	CrashMontage(nullptr),
	TrickFailMontage(nullptr),
	BoardData(),
	TrickData()
{	
	CrashSpeed = 200.0f;
	bMovingForward = false;
	bTurning = false;
	bJumping = false;
	bProcessTrick = false;
	bFalling = false;
	bCharged = false;
	bCharging = false;
	bSouthInputIgnored = false;
	bCrashed = false;
	bIgnoreNextJumpRelease = false;

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
	ChargeApexTime = 1.75f;

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
	AdjacentCollisionRayLength = 65.0f;

	DodgeAdjacentCollisionScale = 250.0f;

	JumpVector = FVector::ZeroVector;
	CrashRot = FRotator::ZeroRotator;
	RotationLastFrame = FRotator::ZeroRotator;

	TimeBeforeGravity = 1.2f;
	DelayGravityTimer = 0.0f;
	TimeSpentFalling = 0.0f;
	GroundDistRangeForGrabs = 55.0f;
	CacheGrabDataThreshold = 0.025f;
	CurrentDistanceFromGround = 0.0f;

	bIsPlayer = false;
}

void UCustomPawnMovementComponent::SetVerticalTrickVector(float Value)
{
	bool bStoreTrickVector = bCharging || bCharged || ((bFalling || bJumping) && !bProcessTrick);
	if (bStoreTrickVector)
	{
		TrickData.SetTrickY(Value);
	}
}

void UCustomPawnMovementComponent::SetHorizontalTrickVector(float Value)
{
	bool bStoreTrickVector = bCharging || bCharged || ( (bFalling || bJumping) && !bProcessTrick);
	if (bStoreTrickVector)
	{
		TrickData.SetTrickX(Value);
	}
}

bool UCustomPawnMovementComponent::CanTurn() const
{
	bool bCanTurn = true;
	if (bCharged || bCharging || bProcessTrick)
	{
		bCanTurn = false;
	}
	return bCanTurn;
}

void UCustomPawnMovementComponent::RequestGrab(ETrickDirection Trick)
{
	if (!AnimInstance)
	{
		return;
	}

	const bool bIsGrabbed = AnimInstance->HasGrabData();
	if (bIsGrabbed)
	{
		// Need to release grab before trying a new one.

		// TODO: Maybe buffer 1 grab here - So it can seamlessly blend into the next one if its pressed.
		//return;

		// TODO: OKAY!
		// So Queue up inputs here
		// Clear them all on landed.
		// When cancel grab is hit for an input, pop that trick from teh queue
		// Then we can instantly swap to the next grab.
	}

	if (CurrentDistanceFromGround > GroundDistRangeForGrabs || bJumping)
	{
		if ((bJumping || bFalling) )//&& !bProcessTrick)
		{			
			AnimInstance->SetTrickDirection(Trick);		
		}
	}	
}

void UCustomPawnMovementComponent::CancelGrab(ETrickDirection Trick)
{
	if (!AnimInstance)
	{
		return;
	}

	const bool bIsGrabbed = AnimInstance->HasGrabData();
	if (bIsGrabbed)
	{
		AnimInstance->ResetTrickVector();
	}				
}

void UCustomPawnMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	BoardData = FBoardData(BoardType);
	
	APawn* Owner = GetPawnOwner();
	if (const ASnowboardCharacterBase* SnowboardCharacter = Cast<ASnowboardCharacterBase>(Owner))
	{
		bIsPlayer = !SnowboardCharacter->IsAIControlled();
		if (UStaticMeshComponent* Snowboard = SnowboardCharacter->GetSnowboard())
		{
			UStaticMesh* BoardMesh = SnowboardCharacter->GetBoardFromType(BoardType);
			Snowboard->SetStaticMesh(BoardMesh);
		}
	}
	else
	{
		bIsPlayer = false;
	}
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
		BoardData.ForwardSpeed = BoardData.RecoverySpeed;
		return true;
	}

	APawn* Owner = GetPawnOwner();
	if (!Owner)
	{
		CrashTimer = 0.0f;
		bCrashed = false;
		return false;
	}

	const FVector& OwnerLocation = Owner->GetActorLocation();
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
	if (!bCrashFromTrick)
	{
		DeltaVec *= -1.0f;
	}
	else
	{
		DeltaVec *= 2.5f;
	}
	

	// Need to reset our rotation
	FRotator RecoilRotation = RotationLastFrame;
	//RecoilRotation.Pitch = 0.0f;

	FHitResult OutHit;
	//DeltaVec.Normalize();
	SafeMoveUpdatedComponent(DeltaVec, RecoilRotation, true, OutHit, ETeleportType::None);

	if (!OutHit.bBlockingHit)
	{
		// Apply some psuedo gravity here.
		FVector PseudoGravity = FVector::DownVector * BoardData.GravityScale * DeltaTime;
		SafeMoveUpdatedComponent(PseudoGravity, RecoilRotation, true, OutHit, ETeleportType::None);
	}
	else
	{
		FVector PseudoKnockback = FVector::UpVector * BoardData.GravityScale * 0.5f * DeltaTime;
		SafeMoveUpdatedComponent(PseudoKnockback, RecoilRotation, true, OutHit, ETeleportType::None);
	}
	return true;
}

void UCustomPawnMovementComponent::ProcessJump(float DeltaTime)
{
	if (!bJumping || bFalling)
	{
		return;
	}

	JumpTimer += DeltaTime;
	const float LocalApexTime = bChargedJumping ? JumpApexTime * 1.2f : JumpApexTime;
	if (JumpTimer >= LocalApexTime)
	{
		float CumulativeHangTime = JumpApexTime + AirTime;
		if (bChargedJumping)
		{
			CumulativeHangTime *= 1.2f;
		}

		if (JumpTimer >= CumulativeHangTime)
		{
			CancelJump();
		}		
		return;
	}

	APawn* Owner = GetPawnOwner();
	if (!Owner)
	{
		return;
	}

	const FVector& OwnerLocation = Owner->GetActorLocation();
	const FVector& OwnerForward = Owner->GetActorForwardVector();

	// ChargedJumpFactor
	float ChargedJumpFactorVertical = 1;
	float ChargedJumpFactorForward = 1;
	if (bChargedJumping)
	{
		ChargedJumpFactorVertical = 0.75f;
		ChargedJumpFactorForward = 6.75f;
	}

	// Add Vertical movement - and a bit of forward movement.
	FVector JmpVec(FVector::UpVector);
	JmpVec *= (BoardData.JumpScale * ChargedJumpFactorVertical * DeltaTime);

	// Only add forward movement, if we're not at max speed.
	//if (BoardData.ForwardSpeed <= BoardData.MaxSpeed)
	{
		FVector ForwardVector(OwnerForward);		
		ForwardVector *= BoardData.JumpForwardScale * ChargedJumpFactorForward * DeltaTime;
		ForwardVector.Z = 0.0f;
		JmpVec += ForwardVector;
	}
	
	JumpVector = JmpVec;	

	FHitResult OutResult;
	SafeMoveUpdatedComponent(JumpVector, RotationLastFrame, true, OutResult, ETeleportType::None);
}

void UCustomPawnMovementComponent::ProcessTrick(float DeltaTime)
{
	if (!bProcessTrick)
	{
		return;
	}

	APawn* Owner = GetPawnOwner();
	if (!PawnOwner)
	{
		return;
	}

	TrickData.TrickTimer += DeltaTime;
	ETrickDirection TrickToPerform = TrickData.GetTrickFromCache();
	switch (TrickToPerform)
	{
	case ETrickDirection::North:
		ProcessForwardRoll(*Owner, DeltaTime);
		break;
	case ETrickDirection::East:
		ProcessRightRoll(*Owner, DeltaTime);
		break;
	case ETrickDirection::South:
		ProcessBackwardsRoll(*Owner, DeltaTime);
		break;
	case ETrickDirection::West:
		ProcessLeftRoll(*Owner, DeltaTime);
		break;
	case ETrickDirection::NorthEast:
		ProcessDiagonalNE(*Owner, DeltaTime);
		break;
	case ETrickDirection::SouthEast:
		ProcessDiagonalSE(*Owner, DeltaTime);
		break;
	case ETrickDirection::SouthWest:
		ProcessDiagonalSW(*Owner, DeltaTime);
		break;
	case ETrickDirection::NorthWest:
		ProcessDiagonalNW(*Owner, DeltaTime);
		break;
	case ETrickDirection::Max:
	default:
		checkNoEntry();
		break;
	}
}

void UCustomPawnMovementComponent::ProcessForwardRoll(APawn& Owner, float DeltaTime)
{
	USceneComponent* RootComp = Owner.GetRootComponent();
	if (!RootComp)
	{
		return;
	}

	float LerpValue = TrickData.TrickTimer / TrickData.TimeForTrick;
	LerpValue = FMath::Clamp(LerpValue, 0.0f, 1.0f);
	TrickData.TrickRotation = FMath::Lerp(0.0f, 360.0f, LerpValue);
	const float StartingRot = CachedRotationForTrick.Pitch;
	FRotator DeltaRot = FRotator(StartingRot - TrickData.TrickRotation, CachedRotationForTrick.Yaw, CachedRotationForTrick.Roll);
	RootComp->SetWorldRotation(DeltaRot);

	if (LerpValue >= 1.0f)//TrickData.TrickTimer >= TrickData.TimeForTrick)
	{
		SetProcessTrick(false);		
		TrickData.OnTrickPerformed(ETrickDirection::North);
		TrickData.ResetTrickData();
	}
}

void UCustomPawnMovementComponent::ProcessRightRoll(APawn& Owner, float DeltaTime)
{
	USceneComponent* RootComp = Owner.GetRootComponent();
	if (!RootComp)
	{
		return;
	}
	
	float LerpValue = TrickData.TrickTimer / TrickData.TimeForTrick;
	LerpValue = FMath::Clamp(LerpValue, 0.0f, 1.0f);
	TrickData.TrickRotation = FMath::Lerp(0.0f, 360.0f, LerpValue);
	const float StartingRot = CachedRotationForTrick.Yaw;
	FRotator DeltaRot = FRotator(CachedRotationForTrick.Pitch, StartingRot + TrickData.TrickRotation, CachedRotationForTrick.Roll);
	RootComp->SetWorldRotation(DeltaRot);

	if (LerpValue >= 1.0f)//TrickData.TrickTimer >= TrickData.TimeForTrick)
	{
		SetProcessTrick(false);
		TrickData.OnTrickPerformed(ETrickDirection::East);
		TrickData.ResetTrickData();
	}
}

void UCustomPawnMovementComponent::ProcessBackwardsRoll(APawn& Owner, float DeltaTime)
{
	USceneComponent* RootComp = Owner.GetRootComponent();
	if (!RootComp)
	{
		return;
	}

	float LerpValue = TrickData.TrickTimer / TrickData.TimeForTrick;
	LerpValue = FMath::Clamp(LerpValue, 0.0f, 1.0f);
	TrickData.TrickRotation = FMath::Lerp(0.0f, 360.0f, LerpValue);
	const float StartingRot = CachedRotationForTrick.Pitch;
	FRotator DeltaRot = FRotator(StartingRot + TrickData.TrickRotation, CachedRotationForTrick.Yaw,  CachedRotationForTrick.Roll);
	RootComp->SetWorldRotation(DeltaRot);

	if (LerpValue >= 1.0f)//TrickData.TrickTimer >= TrickData.TimeForTrick)
	{
		SetProcessTrick(false);
		TrickData.OnTrickPerformed(ETrickDirection::South);
		TrickData.ResetTrickData();
	}
}

void UCustomPawnMovementComponent::ProcessLeftRoll(APawn& Owner, float DeltaTime)
{
	USceneComponent* RootComp = Owner.GetRootComponent();
	if (!RootComp)
	{
		return;
	}

	float LerpValue = TrickData.TrickTimer / TrickData.TimeForTrick;
	LerpValue = FMath::Clamp(LerpValue, 0.0f, 1.0f);
	TrickData.TrickRotation = FMath::Lerp(0.0f, 360.0f, LerpValue);
	const float StartingRot = CachedRotationForTrick.Yaw;
	FRotator DeltaRot = FRotator(CachedRotationForTrick.Pitch, StartingRot - TrickData.TrickRotation, CachedRotationForTrick.Roll);
	RootComp->SetWorldRotation(DeltaRot);

	if (LerpValue >= 1.0f)//TrickData.TrickTimer >= TrickData.TimeForTrick)
	{
		SetProcessTrick(false);
		TrickData.OnTrickPerformed(ETrickDirection::West);
		TrickData.ResetTrickData();
	}
}

void UCustomPawnMovementComponent::ProcessDiagonalNE(APawn& Owner, float DeltaTime)
{
	ProcessForwardRoll(Owner, DeltaTime);
}

void UCustomPawnMovementComponent::ProcessDiagonalSE(APawn& Owner, float DeltaTime)
{
	ProcessBackwardsRoll(Owner, DeltaTime);
}

void UCustomPawnMovementComponent::ProcessDiagonalSW(APawn& Owner, float DeltaTime)
{
	ProcessBackwardsRoll(Owner, DeltaTime);
}

void UCustomPawnMovementComponent::ProcessDiagonalNW(APawn& Owner, float DeltaTime)
{
	ProcessForwardRoll(Owner, DeltaTime);
}

void UCustomPawnMovementComponent::ProcessGravity(float DeltaTime)
{
	bNoSurfaceNormalFoundThisFrame = false;
	if (bJumping)
	{
		ConstrainToPlaneNormal(false);
		CurrentDistanceFromGround = FLT_MAX;
		return;
	}

	FHitResult Hitresult;
	const bool bIsGrounded = IsGrounded(Hitresult);
	if (!bIsGrounded)
	{
		// Apply downward movement.
		bFalling = true;
		TimeSpentFalling += DeltaTime;
		DelayGravityTimer += DeltaTime;
		//if (DelayGravityTimer < TimeBeforeGravity)
		//{
		//
		//}
		//else
		{

			float ChargedJumpFactorVertical = 1.0f;
			if (bChargedJumping)
			{
				ChargedJumpFactorVertical = 1.33f;
			}

			FVector GravityVec(FVector::DownVector);
			GravityVec *= BoardData.GravityScale * DeltaTime * ChargedJumpFactorVertical;

			//APawn* Owner = GetPawnOwner();
			//if (bChargedJumping && Owner)
			//{
			//	float ChargedJumpFactorForward = 5.0f;
			//	const FVector& OwnerForward = Owner->GetActorForwardVector();
			//	FVector ForwardVector(OwnerForward);
			//	ForwardVector *= BoardData.JumpForwardScale * ChargedJumpFactorForward * DeltaTime;
			//	GravityVec += ForwardVector;
			//}

			bMatchRotToImpactNormal = false;
			//SetPlaneConstraintEnabled(false);
			//ImpactNormal = FVector::UpVector;
			SetGravityVector(GravityVec);
			ConstrainToPlaneNormal(false); // NOTE: This may or may not be correct

		}

		// NOTE: Be much better to recalibrate our characters Roll (forward/backward tilt) mid air, so we land with the board flat

	}
	else if(bFalling)
	{
		// If we were falling, but now we're grounded.
		// we effectively are 'landing' on this frame.
		ConstrainToPlaneNormal(true);
		ImpactPoint = Hitresult.ImpactPoint;
		ImpactNormal = Hitresult.ImpactNormal;
		TimeSpentFalling = 0.0f;
		//DrawDebugLine(GetWorld(), Hitresult.ImpactPoint, Hitresult.ImpactPoint + (ImpactNormal.Normalize() * 45.0f), FColor::Green, false, 3.0f);
		OnLanded();
	}
	else
	{
		TimeSpentFalling = 0.0f;
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
		GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("Speed: %.1f, Max: %.1f"), BoardData.ForwardSpeed, BoardData.MaxSpeed));
	}
#endif

	BoardData.ForwardSpeed += BoardData.Acceleration * DeltaTime;
	BoardData.ForwardSpeed = FMath::Clamp(BoardData.ForwardSpeed, 0.0f, BoardData.MaxSpeed);
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
	if (!Owner)
	{
		return;
	}
	
	const FVector& OwnerLocation = Owner->GetActorLocation();
	const FVector& RightVec = Owner->GetActorRightVector();
	const FVector& OwnerForward = bProcessTrick ? CachedForwardVector : Owner->GetActorForwardVector();

	FVector InputVector = ConsumeInputVector();
	InputVector.Y = FMath::Clamp(InputVector.Y, -1.0f, 1.0f);
	float YValue = InputVector.Y;
	if (!bIsPlayer)
	{
		GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Yellow, FString::Printf(TEXT("AI Input Y: %.1f"), YValue));
	}

	bool bHasGrabData = false;
	if (AnimInstance)
	{
		bHasGrabData = AnimInstance->HasGrabData();
	}

	// Continually move forward.	
	if (YValue != 0.0f && bIsPlayer && !bHasGrabData)
	{
		// Turning, decelerate a smidge.
		if (BoardData.ForwardSpeed >= BoardData.MinTurnSpeed)
		{
			float DecelerationScale = 0.0f;
			const float Threshold = BoardData.MaxSpeed * 0.85f;
			if (BoardData.ForwardSpeed >= Threshold)
			{
				DecelerationScale = 1.25f;
			}
			BoardData.ForwardSpeed -= BoardData.Acceleration * DecelerationScale * DeltaTime;
			BoardData.ForwardSpeed = FMath::Clamp(BoardData.ForwardSpeed, BoardData.MinTurnSpeed, BoardData.MaxSpeed);
		}		
	}

	FVector DeltaVec = OwnerForward * BoardData.ForwardSpeed * DeltaTime;
	if (bIsPlayer && !bCharging && !bCharged && !bJumping && !bFalling && !bCrashed && !bProcessTrick && !bHasGrabData)
	{
		if (BoardData.ForwardSpeed >= BoardData.MaxSpeed * 0.66f)
		{
			FVector TurnVec = InputVector * BoardData.HorizontalSpeed * DeltaTime;
			DeltaVec += TurnVec;
		}
		else
		{
			FVector TurnVec = InputVector * BoardData.HorizontalSpeed * 0.2f * DeltaTime;
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
		AnimInstance->SetSpeed(BoardData.ForwardSpeed);		
		float AITiltValue = YValue;
		if (!bIsPlayer)
		{
			// This ensures the AI play the idle animation instead of spamming the blend space, their turning is more... rigid so yeah.
			if (YValue < 0.3f)
			{
				AITiltValue = 0.0f;
			}
		}
		AnimInstance->SetTilt(AITiltValue);

//#if defined DEBUG_SNOWBOARD_KIDS
		if (GEngine && !bIsPlayer)
		{
			GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("BoardData.ForwardSpeed: %.1f"), BoardData.ForwardSpeed));
			GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("AITiltValue: %.3f"), AITiltValue));
		}
//#endif
	}

	//TODO: Fix bug whereby driving down a slope, and hitting a flat plane, causing a weird rotation spike, from the MakeRot function on the pitch.
	// It seems to happen when there is no surface normal found and uses an old value for it.
	// Handle Rotating slightly based on input.
	float NewPitch = 0.0f;

	if (bHasGrabData)
	{
		YValue = 0.0f;
	}

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
	//return;
	
	if (bFalling || bJumping || bCrashed || bCharging)
	{
		return;
	}

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
	const FVector& OwnerLocation = Owner->GetActorLocation();
	const FVector& StartLeft = OwnerLocation + FVector(0.0f, 5.0f, 45.5f);
	const FVector& StartRight = OwnerLocation + FVector(0.0f, -5.0f, 45.5f);
	const FVector& ForwardVector = Owner->GetActorForwardVector();
	const FVector& RightVector = Owner->GetActorRightVector();
	const FVector EndLeft = StartLeft - (RightVector * AdjacentCollisionRayLength);
	const FVector EndRight = StartRight + (RightVector * AdjacentCollisionRayLength);
	AActor* OwnerChar = Cast<AActor>(Owner);
	ActorsToIgnore.Add(OwnerChar);
	const bool bHitRight = UKismetSystemLibrary::SphereTraceSingle(GetOuter(), StartRight, EndRight, SphereCastRadius, ETraceTypeQuery::TraceTypeQuery2, true,
		ActorsToIgnore, EDrawDebugTrace::None, RightHitResult, true);

	bool bPersistent = false;
	float LifeTime = 1.0f;

	if (bHitRight && RightHitResult.bBlockingHit)
	{
		bCollisionRight = true;
	}
	FVector HitRightPosition = RightHitResult.ImpactPoint;

	FHitResult LeftHitResult;
	const bool bHitLeft = UKismetSystemLibrary::SphereTraceSingle(GetOuter(), StartLeft, EndLeft, SphereCastRadius, ETraceTypeQuery::TraceTypeQuery2, true,
		ActorsToIgnore, EDrawDebugTrace::None, LeftHitResult, true);

	if (bHitLeft && LeftHitResult.bBlockingHit)
	{
		bCollisionLeft = true;
	}

	FVector HitLeftPosition = LeftHitResult.ImpactPoint;
	if (bCollisionLeft && bCollisionRight)
	{
		// Strafe in the direction that is further away.
		FVector DirToLeft = HitLeftPosition - StartLeft;
		FVector DirToRight = HitRightPosition - StartRight;
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
		FVector RightImpactNormal = RightHitResult.ImpactNormal;
		TranslationZDelta = RightImpactNormal * DodgeAdjacentCollisionScale * DeltaTime;
	}
	else if (bCollisionLeft)
	{
		FVector LeftImpactNormal = LeftHitResult.ImpactNormal;
		TranslationZDelta = LeftImpactNormal * DodgeAdjacentCollisionScale * DeltaTime;
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

	// This is accomplished by taking the magnitude of the vector times the cosine of the vector's angle to find the horizontal component
	// , and the magnitude of the vector times the sine of the vector's angle to find the vertical component
	
	FHitResult HitResult;
	TArray<AActor*> ActorsToIgnore;
	const FVector& OwnerLocation = Owner->GetActorLocation();
	const FVector& Start = OwnerLocation + FVector(0.0f, 0.0f, 10.0f);
	const FVector& StartRight = OwnerLocation + FVector(0.0f, -15.0f, 0.0f);
	const FVector& StartLeft = OwnerLocation + FVector(0.0f, 15.0f, 0.0f);
	const FVector& ForwardVector = Owner->GetActorForwardVector();
	FVector ActualForward = ForwardVector;
	if (ActualForward.Z < 0.0f)
	{
		// Make it 2D if going down a slope, can follow the grain, going up slope.
		// The difference is, we don't want to detect the floor as a collision when changing from a sharp incline.
		ActualForward.Z = 0.0f;
	}
	
	const FVector& RightVector = Owner->GetActorRightVector();
	const FVector End = Start + (ActualForward * CheckCollisionRayLength);
	const FVector EndRight = StartRight + (ActualForward * CheckCollisionRayLength);
	const FVector EndLeft = StartLeft + (ActualForward * CheckCollisionRayLength);
	AActor* OwnerChar = Cast<AActor>(Owner);
	ActorsToIgnore.Add(OwnerChar);
	const bool bHit = UKismetSystemLibrary::SphereTraceSingle(GetOuter(), Start, End, SphereCastRadius, ETraceTypeQuery::TraceTypeQuery2, true,
		ActorsToIgnore, EDrawDebugTrace::ForOneFrame, HitResult, true);

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
		const float AngleBetweenNormalAndUpRads = FVector::DotProduct(CollisionImpactNormal, RightVector);
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

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Yellow, FString::Printf(TEXT("Dist: %.1f"), CurrentDistanceFromGround));
	}

	ProcessJump(DeltaTime);
	ProcessGravity(DeltaTime);

	ProcessAcceleration(DeltaTime);
	ProcessCharging(DeltaTime);
	ProcessForwardMovement(DeltaTime, IncomingQuat);
	ProcessTrick(DeltaTime);

	// Maybe make this AI only + Players with Accessibility option.
	// Makes it harder to crash into walls.
	ProcessAdjacentObstacles(DeltaTime);

	ProcessDetectCollisions(DeltaTime);
}

void UCustomPawnMovementComponent::TriggerJump()
{
	if (bIgnoreNextJumpRelease)
	{
		bIgnoreNextJumpRelease = false;
		return;
	}

	APawn* Owner = GetPawnOwner();
	if (!Owner)
	{
		return;
	}

	if (bJumping || bFalling || bChargedJumping)
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

	if (TrickData.IsTrickBufferred())
	{
		TrickData.UpdateCache();
		SetProcessTrick(true);
		CachedForwardVector = Owner->GetActorForwardVector();
		CachedRotationForTrick = Owner->GetRootComponent()->GetComponentRotation();
		//UE_LOG(LogTemp, Log, TEXT("Cached Roll: %.1f"), CachedRotationForTrick.Roll);
		//UE_LOG(LogTemp, Log, TEXT("Cached Pitch: %.1f"), CachedRotationForTrick.Pitch);
		//UE_LOG(LogTemp, Log, TEXT("Cached Yaw: %.1f"), CachedRotationForTrick.Yaw);


	}
}

void UCustomPawnMovementComponent::CancelJump()
{
	bJumping = false;
	//bChargedJumping = false;
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

		FVector GroundVector = Result.ImpactPoint - (Start - FVector(0.0f, 0.0f, 45.0f));
		const float DistToFloor = GroundVector.Size();
		CurrentDistanceFromGround = DistToFloor;
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

		CurrentDistanceFromGround = FLT_MAX;
		return false;		
	}
	CurrentDistanceFromGround = FLT_MAX;
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
	DelayGravityTimer = 0.0f;
	bChargedJumping = false;

	// Fall over if we hit the floor whilst mid trick.
	bool bHasGrabData = false;
	if (AnimInstance)
	{
		bHasGrabData = AnimInstance->HasGrabData();
		CancelGrab(ETrickDirection::Max);
	}

	if (bProcessTrick || bHasGrabData)
	{		
		TriggerCrash(CachedRotationForTrick);
	}
}

bool UCustomPawnMovementComponent::ValidateOwnerComponents()
{
	const APawn* Owner = GetPawnOwner();
	if (!Owner)
	{
		return false;
	}	

	if (!AnimInstance)
	{
		if (const ASnowboardCharacterBase* SnowboardBase = Cast<ASnowboardCharacterBase>(Owner))
		{
			AnimInstance = SnowboardBase->GetAnimInstance();
		}
	}

	if (!AnimInstance)
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
	APawn* Owner = GetPawnOwner();
	if (!Owner)
	{
		return;
	}

	bCrashed = true;
	CrashRot = UpdatedRotation;
	if (bCharged || bCharging)
	{
		CancelCharge();
		bIgnoreNextJumpRelease = true;
	}	

	USceneComponent* Root = Owner->GetRootComponent();
	if (Root && bProcessTrick)
	{
		// Might need to make the tricks all montages
		// And investigate additive animations.
		// as the blending from failing a trick is awful with this approach currently.
		Root->SetWorldRotation(UpdatedRotation);
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Play Animation	
	if (ASnowboardCharacterBase* SnowboardCharacter = Cast<ASnowboardCharacterBase>(Owner))
	{
		if (bProcessTrick)
		{
			bCrashFromTrick = true;
			SnowboardCharacter->PlayAnimation(TrickFailMontage);
		}
		else
		{
			// Hard crash into a wall.
			bCrashFromTrick = false;
			SnowboardCharacter->PlayAnimation(CrashMontage);

			// TODO: Implement soft crash.
		}
	}

	TrickData.ResetTrickData();
	SetProcessTrick(false);
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

	if (bProcessTrick)
	{
		return UpdatedRotation;
	}

	const float DeltaTime = World->GetDeltaSeconds();
	// If there is input to strafe.
	float RootYaw = FMath::Clamp(YValue, -BoardData.TurnLimit, BoardData.TurnLimit);
	if (YValue != 0.0f)
	{
		static bool RotateOnHorizontalInput = true;
		if (RotateOnHorizontalInput)
		{
			// This way of doing this, fixes the wierd glitch of them insta spinning.
			const FRotator& CurrentRotation = RootComp->GetComponentRotation();
			FRotator TargetRotation = CurrentRotation + FRotator(0.0f, RootYaw, RootYaw);
			FRotator InterpedRot = UKismetMathLibrary::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, BoardData.TurnRateInterpSpeed);
			RootComp->SetWorldRotation(InterpedRot);
		}
	}

	//RotateBoard(Owner, YValue);

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

void UCustomPawnMovementComponent::SetProcessTrick(bool Value)
{
	bProcessTrick = Value;
	// Notify Camera whether it should inherit Yaw or not.
	if (bIsPlayer)
	{
		// Grab Player Camera.
		APawn* Owner = GetPawnOwner();
		ASnowboardCharacterBase* SnowboardPlayer = Cast<ASnowboardCharacterBase>(Owner);
		if (SnowboardPlayer)
		{
			ASnowboardPlayerController* PlayerController = Cast<ASnowboardPlayerController>(SnowboardPlayer->GetController());
			if (PlayerController)
			{
				PlayerController->ConstrainYawToPlayer(!Value);
			}
		}
	}
}
