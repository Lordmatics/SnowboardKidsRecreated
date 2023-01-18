// Lordmatics Games


#include "SnowboardKids/CustomActors/Projectiles/TargetSeekingProjectile.h"
#include "SnowboardKids/CustomCharacters/SnowboardCharacterBase.h"
#include <Kismet/KismetMathLibrary.h>
#include "SnowboardKids/Systems/WorldSystems/SnowboardCharacterSubsystem.h"
#include "SnowboardKids/Utils/GameUtils.h"

ATargetSeekingProjectile::ATargetSeekingProjectile() :
	Super(),
	LockedTarget(nullptr),
	SeekForwardRange(850.0f),
	SeekSideRange(425.0f),
	ProjectileSeekSpeed(600.0f),
	bTargetLost(false)
{

}

void ATargetSeekingProjectile::ProcessMovement(float DeltaTime)
{
	ProcessSeek(DeltaTime);
}

void ATargetSeekingProjectile::BeginPlay()
{
	Super::BeginPlay();	
}

void ATargetSeekingProjectile::ProcessSeek(float DeltaTime)
{
	// Continuously Move Forward.
	FVector ProjectileLocation = GetActorLocation();

	// Seek Target
	if (!LockedTarget && Shooter)
	{
		if (USnowboardCharacterSubsystem* SnowboardCharacterSystem = USnowboardCharacterSubsystem::GetCharacterSystem(GetWorld()))
		{
			LockedTarget = SnowboardCharacterSystem->FindClosestCharacterWithinRange(*Shooter, *this, SeekForwardRange, SeekSideRange);
			bTargetLost = false;
		}
	}

	const FVector& ForwardVector = GetActorForwardVector();
	const FVector& RightVector = GetActorRightVector();
	float X = 0.0f;
	float Y = 0.0f;
	if (LockedTarget)
	{
		const FVector& LockedTargetPosition = LockedTarget->GetActorLocation();
		const FTransform& OwnerTransform = GetTransform();
		const FVector& Results = GameUtils::GetMagnitudeOfLocalTransformToTarget(OwnerTransform, LockedTargetPosition);

		X = Results.X; // Distance to reach forward position
		Y = Results.Y; // Distance to reach side position

		if (X < 0.0f)
		{
			bTargetLost = true;
			LockedTarget = nullptr;
			UE_LOG(LogTemp, Log, TEXT("Target Evaded Projectile"));
		}
	}

	float ForwardProjectileSpeed = ProjectileSpeed;
	if (LockedTarget)
	{
		// Once we've locked on, slow down just a tad, and start strafing.
		ForwardProjectileSpeed *= 0.9f;
	}

	float ForwardSpeed = ForwardProjectileSpeed * DeltaTime;
	FVector ForwardMomentum = ForwardVector * ForwardSpeed;
	FVector Velocity = bTargetLost ? ProjectileVelocity : ForwardMomentum;
	if (LockedTarget)
	{
		// X Represents the distance between us and the target, in the forward axis.
		// The smaller it is, the larger we want the seek speed.
		float LowSeek = ProjectileSeekSpeed * 0.75f;
		float MappedResult = ProjectileSeekSpeed + LowSeek - GameUtils::MapValues(X, 0.0f, SeekForwardRange, LowSeek, ProjectileSeekSpeed);
		float StrafeSpeed = MappedResult * DeltaTime;

		FVector AdjacentMomentum = RightVector * StrafeSpeed * FMath::Sign(Y);
		Velocity += AdjacentMomentum;		
	}
	Velocity.Z = 0.0f;
	float VelocityZ = 0.0f;
	// Remove Vertical component of Velocity;
	FHitResult HitResult;
	const AProjectileBase::ScaleGroundResult GroundResult = ShouldApplyGravity(ProjectileLocation, HitResult);

	FVector GroundLocation;
	if (HitResult.bBlockingHit)
	{
		GroundLocation = HitResult.ImpactPoint;
		const float HeightAboveGround = 30.0f;
		const float GroundZ = GroundLocation.Z + HeightAboveGround;
		const bool bDescending = ((GroundZ - ProjectileLocation.Z) < 0.0f);
		const float HeightInterpSpeed = bDescending ? 2.5f : 50.0f;
		VelocityZ = FMath::FInterpTo(ProjectileLocation.Z, GroundZ, DeltaTime, HeightInterpSpeed);
		if (AActor* HitActor = HitResult.GetActor())
		{
			if (ASnowboardCharacterBase* SnowboardCharacter = Cast<ASnowboardCharacterBase>(HitActor))
			{
				if (SnowboardCharacter == Shooter)
				{
					UE_LOG(LogTemp, Warning, TEXT("Hitting Self ?"));
				}

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
		VelocityZ = FMath::FInterpTo(ProjectileLocation.Z, Descend, DeltaTime, 2.5f);
	}

	FHitResult OutSweepHitResult;
	FVector NewLocation = ProjectileLocation + Velocity;
	NewLocation.Z = VelocityZ;
	ProjectileVelocity = Velocity;
	SetActorLocation(NewLocation, true, &OutSweepHitResult, ETeleportType::None);	
}
