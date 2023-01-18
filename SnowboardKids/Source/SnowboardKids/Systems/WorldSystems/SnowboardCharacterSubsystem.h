// Lordmatics Games

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SnowboardCharacterSubsystem.generated.h"

class ASnowboardCharacterBase;

/**
 * 
 */
UCLASS()
class SNOWBOARDKIDS_API USnowboardCharacterSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

	USnowboardCharacterSubsystem();

public:

	static USnowboardCharacterSubsystem* GetCharacterSystem(const UWorld* World);
	static USnowboardCharacterSubsystem* GetCharacterSystem(const UWorld& World);

	void RegisterCharacter(ASnowboardCharacterBase* Character);
	void UnRegisterCharacter(ASnowboardCharacterBase* Character);

	ASnowboardCharacterBase* FindClosestCharacterWithinRange(const ASnowboardCharacterBase& Shooter, const AActor& Projectile, float ForwardRange, float SideRange) const;

private:

	UPROPERTY(Transient)
	TArray<ASnowboardCharacterBase*> RegisterredCharacters;
};
