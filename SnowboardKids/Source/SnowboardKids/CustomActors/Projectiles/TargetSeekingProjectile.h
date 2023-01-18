// Lordmatics Games

#pragma once

#include "CoreMinimal.h"
#include "SnowboardKids/CustomActors/Projectiles/ProjectileBase.h"
#include "TargetSeekingProjectile.generated.h"

class ASnowboardCharacterBase;

/**
 * 
 */
UCLASS()
class SNOWBOARDKIDS_API ATargetSeekingProjectile : public AProjectileBase
{
	GENERATED_BODY()

public:
	ATargetSeekingProjectile();

	virtual void ProcessMovement(float DeltaTime) override;
	
protected:
	virtual void BeginPlay() override;

	void ProcessSeek(float DeltaTime);

private:

	UPROPERTY(Transient, VisibleAnywhere, Category = "Projectile")
	ASnowboardCharacterBase* LockedTarget;

	// How close must we be behind the target to acknowledge it.
	UPROPERTY(EditAnywhere, Category = "Projectile")
	float SeekForwardRange;

	// How close must we be adjacent to the target to acknowledge it.
	UPROPERTY(EditAnywhere, Category = "Projectile")
	float SeekSideRange;

	// How fast we should strafe towards are locked target.
	UPROPERTY(EditAnywhere, Category = "Projectile")
	float ProjectileSeekSpeed;

	// If we have a target, and they evade, the projectile should just continue in its last known direction.
	bool bTargetLost;
};
