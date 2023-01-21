// Lordmatics Games

#pragma once

#include "CoreMinimal.h"
#include "SnowboardKids/CustomActors/Items/Offensive/OffensiveItem.h"
#include "ProjectileBase.generated.h"

UENUM(BlueprintType)
enum class EProjectileType : uint8
{
	Hands,
	Ice,
	Parachute,
	Bombs,
	Snowmen,
	Tornado,
	MAX UMETA(Hidden)
};

// We'll customise how the projectile moves via these classes.
// And the hit response can be determined by an enum.

UCLASS()
class SNOWBOARDKIDS_API AProjectileBase : public AOffensiveItem
{
	GENERATED_BODY()
	
public:	
	AProjectileBase();

	FORCEINLINE EProjectileType GetProjectileType() const { return ProjectileType; }

	virtual void ProcessMovement(float DeltaTime) PURE_VIRTUAL(AProjectileBase::ProcessMovement, return; )

	bool RaycastToFloor(FHitResult& HitResult) const;

	enum class ScaleGroundResult : uint8
	{
		Stay,
		Lift,
		Fall
	};

	AProjectileBase::ScaleGroundResult ShouldApplyGravity(const FVector& Start, FHitResult& GroundResult) const;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

protected:

	UPROPERTY(EditAnywhere, Category = "Projectile")
	EProjectileType ProjectileType;

	FVector ProjectileVelocity;

	UPROPERTY(EditAnywhere, Category = "Projectile")
	float ProjectileSpeed;

	UPROPERTY(EditAnywhere, Category = "Projectile")
	float ProjectileGravity;

	UPROPERTY(EditAnywhere, Category = "Projectile")
	float FloorRayLength;

	UPROPERTY(EditAnywhere, Category = "Projectile")
	float DistanceToConsiderGrounded;

	float ProjectileLifeTimer;

	UPROPERTY(EditAnywhere, Category = "Projectile")
	float ProjectileLife;

	float ProjectileHeight;
};
