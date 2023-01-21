// Lordmatics Games


#include "SnowboardKids/CustomActors/Projectiles/RicochetProjectile.h"
#include "SnowboardKids/CustomCharacters/SnowboardCharacterBase.h"

ARicochetProjectile::ARicochetProjectile() :
	Super()
{

}

void ARicochetProjectile::ProcessMovement(float DeltaTime)
{
	// Continuously Move Forward.
	const FVector& ProjectileLocation = GetActorLocation();
	const FVector& ForwardVector = GetActorForwardVector();
	const FVector Delta = ForwardVector * ProjectileSpeed * DeltaTime;
	const FVector AdjustedLocation = ProjectileLocation + Delta;
	FHitResult OutSweepHitResult;
	SetActorLocation(AdjustedLocation, true, &OutSweepHitResult, ETeleportType::None);

	if (OutSweepHitResult.bBlockingHit)
	{
		if (AActor* HitActor = OutSweepHitResult.GetActor())
		{
			ASnowboardCharacterBase* SnowboardCharacter = Cast<ASnowboardCharacterBase>(HitActor);
			if (SnowboardCharacter)
			{
				SnowboardCharacter->OnHitByProjectile(OffensiveItemType);
				Destroy();
			}
			else
			{
				// Make change ForwardVector to just be the rebound normal.
				// If it's a wall - need to reflect.
				ProjectileSpeed *= -1.0f;
				FVector PreAdjustedLocation = ProjectileLocation - Delta;
				SetActorLocation(PreAdjustedLocation, false);
			}			
		}
	}
}

void ARicochetProjectile::BeginPlay()
{
	Super::BeginPlay();
}
