// Lordmatics Games


#include "SnowboardKids/CustomActors/Projectiles/ProjectileBase.h"

// Sets default values
AProjectileBase::AProjectileBase() :
	Super(),
	ProjectileType(EProjectileType::Hands),
	ProjectileSpeed(2450.0f),
	ProjectileGravity(1050.0f),
	FloorRayLength(5000.0f),
	DistanceToConsiderGrounded(66.0f),
	ProjectileLifeTimer(0.0f),
	ProjectileLife(7.0f),
	ProjectileHeight(0.0f),
	ProjectileVelocity(FVector::ZeroVector)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

bool AProjectileBase::RaycastToFloor(FHitResult& HitResult) const
{	
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	const FVector& Start = GetActorLocation();	
	FVector End = Start + (FVector::DownVector * FloorRayLength);
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
		
	bool bHit = World->LineTraceSingleByChannel(HitResult, Start, End, ECollisionChannel::ECC_WorldDynamic, QueryParams);
	return bHit;
}

AProjectileBase::ScaleGroundResult AProjectileBase::ShouldApplyGravity(const FVector& Start, FHitResult& GroundResult) const
{
	const bool bGroundDetected = RaycastToFloor(GroundResult);
	AProjectileBase::ScaleGroundResult Result = ScaleGroundResult::Stay;
	if (bGroundDetected)
	{
		if (GroundResult.bBlockingHit)
		{
			const FVector& ImpactPoint = GroundResult.ImpactPoint;
			const float DistanceToFloor = (Start - ImpactPoint).SizeSquared();
			const float Threshold = FMath::Square<float>(DistanceToConsiderGrounded);
			if (DistanceToFloor > Threshold)
			{
				Result = AProjectileBase::ScaleGroundResult::Fall;
			}
			else if(DistanceToFloor <= (Threshold * 0.5f) )
			{
				Result = AProjectileBase::ScaleGroundResult::Lift;
			}
		}
	}
	return Result;
}

// Called when the game starts or when spawned
void AProjectileBase::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AProjectileBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ProjectileLifeTimer += DeltaTime;
	if (ProjectileLifeTimer >= ProjectileLife)
	{
		Destroy();
		return;
	}

	ProcessMovement(DeltaTime);
}

