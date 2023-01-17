// Lordmatics Games


#include "SnowboardKids/CustomActors/Projectiles/TargetSeekingProjectile.h"
#include "SnowboardKids/CustomCharacters/SnowboardCharacterBase.h"
#include <Kismet/KismetMathLibrary.h>
#include "SnowboardKids/Systems/WorldSystems/SnowboardCharacterSubsystem.h"

ATargetSeekingProjectile::ATargetSeekingProjectile() :
	Super(),
	LockedTarget(nullptr),
	SeekRange(2550.0f) // 2750.0f sort of works
{

}

void ATargetSeekingProjectile::ProcessMovement(float DeltaTime)
{
	// Continuously Move Forward.
	const FVector& ProjectileLocation = GetActorLocation();

	// Seek Target
	if (!LockedTarget && Shooter)
	{
		if (USnowboardCharacterSubsystem* SnowboardCharacterSystem = USnowboardCharacterSubsystem::GetCharacterSystem(GetWorld()))
		{
			LockedTarget = SnowboardCharacterSystem->FindClosestCharacterWithinRange(*Shooter, *this, SeekRange);
		}
	}

	FRotator NewRotation = GetActorRotation();
	bool bUpdateRotation = false;
	const FVector& ForwardVector = GetActorForwardVector();
	const FVector& RightVector = GetActorRightVector();
	//FVector DeltaDir = ForwardVector;
	FVector SideVector = FVector::ZeroVector;
	if (LockedTarget)
	{
		const FVector& LockedPosition = LockedTarget->GetActorLocation();
		FVector DirToLockedTarget = LockedPosition - ProjectileLocation;
		const float Distance = DirToLockedTarget.Size();
		const float StopRotatingRange = 150.0f;// FMath::Square<float>(150.0f);
		FVector DirNormalised = (DirToLockedTarget).GetSafeNormal();

		// If the projectile goes in front of the locked target, clear the lock.
		const float DotForward = FVector::DotProduct(ForwardVector, DirNormalised);
		if (DotForward <= 0.0f)
		{
			LockedTarget = nullptr;
		}

		if (LockedTarget)
		{
			// As Projectile gets closer to the target, we need to increase the seek speed by quite alot.
			//if (Distance <= (RangeToAccelerateSeek * 0.5f))
			//{
			//	ProjectileSeekSpeed += SeekIncrementWhilstLocked * 2.25f * DeltaTime;
			//	ProjectileSeekSpeed = FMath::Clamp<float>(ProjectileSeekSpeed, InitialSeekSpeed, MaxSeekSpeed);
			//}

			// TODO: Not 100% happy with the seeking just yet.
			if (Distance <= RangeToAccelerateSeek)
			{
				const float ExtraRate = 1.4f;
				ProjectileSeekSpeed += SeekIncrementWhilstLocked * ExtraRate * DeltaTime;
				ProjectileSeekSpeed = FMath::Clamp<float>(ProjectileSeekSpeed, InitialSeekSpeed, MaxSeekSpeed);
			}
			else
			{
				ProjectileSeekSpeed += SeekIncrementWhilstLocked * DeltaTime;
				ProjectileSeekSpeed = FMath::Clamp<float>(ProjectileSeekSpeed, InitialSeekSpeed, MaxSeekSpeed);
			}

			if (Distance >= StopRotatingRange)
			{
				//NewRotation = UKismetMathLibrary::FindLookAtRotation(ProjectileLocation, LockedPosition);
				bUpdateRotation = true;
			}

			const float DotSide = FVector::DotProduct(RightVector, DirNormalised);
			const float Deadzone = 0.02f;
			if (DotSide >= Deadzone)
			{
				// Right Side
				SideVector = RightVector;
			}
			else if (DotSide <= -Deadzone)
			{
				// Left Side
				SideVector = -RightVector;
			}
		}
	}
	
	// We don't want to seek into the ground when locking onto someone.
	// Clear the vertical component, and let the gravity decide that.
	FVector ForwardDelta = ForwardVector * ProjectileSpeed * DeltaTime;
	ForwardDelta.Z = 0.0f;
	
	FVector AdjacentDelta = SideVector * ProjectileSeekSpeed * DeltaTime;
	AdjacentDelta.Z = 0.0f;
	
	FVector AdjustedLocation = ProjectileLocation + ForwardDelta + AdjacentDelta;
	FHitResult OutSweepHitResult;

	//TODO: Might test interpolating the side delta, see if i can get it to strafe nicer.
	// 
	// 
	// 
	// Are we grounded ?
	// Add Gravity.	

	FHitResult HitResult;
	const AProjectileBase::ScaleGroundResult GroundResult = ShouldApplyGravity(ProjectileLocation, HitResult);

	FVector GroundLocation;
	if (HitResult.bBlockingHit)
	{
		GroundLocation = HitResult.ImpactPoint;
		const float HeightAboveGround = 45.0f;
		const float GroundZ = GroundLocation.Z + HeightAboveGround;
		const bool bDescending = ((GroundZ - ProjectileLocation.Z) < 0.0f);
		const float HeightInterpSpeed = bDescending ? 2.5f : 50.0f;
		ProjectileHeight = FMath::FInterpTo(ProjectileLocation.Z, GroundZ, DeltaTime, HeightInterpSpeed);
		if (AActor* HitActor = HitResult.GetActor())
		{
			if (ASnowboardCharacterBase* SnowboardCharacter = Cast<ASnowboardCharacterBase>(HitActor))
			{
				SnowboardCharacter->OnHitByProjectile(ProjectileType);
				Destroy();
				return;
			}
		}
	}
	else
	{
		// No Ground? Descend..
		const float Descend = ProjectileLocation.Z - 1000.0f;
		ProjectileHeight = FMath::FInterpTo(ProjectileLocation.Z, Descend, DeltaTime, 2.5f);
	}

	AdjustedLocation.Z = ProjectileHeight;
	SetActorLocation(AdjustedLocation, true, &OutSweepHitResult, ETeleportType::None);	
	if (bUpdateRotation && ProjectileMesh)
	{
		//ProjectileMesh->SetWorldRotation(NewRotation);		
	}
}

void ATargetSeekingProjectile::BeginPlay()
{
	Super::BeginPlay();	
}
