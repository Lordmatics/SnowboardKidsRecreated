// Lordmatics Games

#pragma once

#include "CoreMinimal.h"
#include "SnowboardKids/CustomActors/Projectiles/ProjectileBase.h"
#include "RicochetProjectile.generated.h"

/**
 * 
 */
UCLASS()
class SNOWBOARDKIDS_API ARicochetProjectile : public AProjectileBase
{
	GENERATED_BODY()
	
public:
	ARicochetProjectile();

	virtual void ProcessMovement(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

private:
};
