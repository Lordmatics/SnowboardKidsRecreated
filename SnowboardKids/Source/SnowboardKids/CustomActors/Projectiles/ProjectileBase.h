// Lordmatics Games

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProjectileBase.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class ASnowboardCharacterBase;


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
class SNOWBOARDKIDS_API AProjectileBase : public AActor
{
	GENERATED_BODY()
	
public:	
	AProjectileBase();

	FORCEINLINE EProjectileType GetProjectileType() const { return ProjectileType; }
	FORCEINLINE void SetShooter(ASnowboardCharacterBase* NewShooter) { Shooter = NewShooter; }

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile", meta = (AllowPrivateAccess = "true"))
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* ProjectileMesh;

	UPROPERTY(Transient, VisibleAnywhere, Category = "Projectile")
	ASnowboardCharacterBase* Shooter;

	UPROPERTY(EditAnywhere, Category = "Projectile")
	EProjectileType ProjectileType;

	UPROPERTY(EditAnywhere, Category = "Projectile")
	float ProjectileSpeed;

	UPROPERTY(EditAnywhere, Category = "Projectile")
	float ProjectileSeekSpeed;

	UPROPERTY(EditAnywhere, Category = "Projectile")
	float MaxSeekSpeed;

	UPROPERTY(EditAnywhere, Category = "Projectile")
	float SeekIncrementWhilstLocked;

	UPROPERTY(EditAnywhere, Category = "Projectile")
	float RangeToAccelerateSeek;

	float InitialSeekSpeed;

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