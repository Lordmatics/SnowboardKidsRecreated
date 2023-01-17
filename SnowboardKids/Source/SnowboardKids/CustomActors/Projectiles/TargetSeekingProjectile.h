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

private:

	UPROPERTY(Transient, VisibleAnywhere, Category = "Projectile")
	ASnowboardCharacterBase* LockedTarget;

	UPROPERTY(EditAnywhere, Category = "Projectile")
	float SeekRange;
};
