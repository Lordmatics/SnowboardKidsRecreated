// Lordmatics Games

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemBase.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class ASnowboardCharacterBase;

UCLASS()
class SNOWBOARDKIDS_API AItemBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AItemBase();

	FORCEINLINE void SetShooter(ASnowboardCharacterBase* NewShooter) { Shooter = NewShooter; }

	virtual void OnSpawned();

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

};
