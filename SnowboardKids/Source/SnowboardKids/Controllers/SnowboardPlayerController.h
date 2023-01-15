// Lordmatics Games

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SnowboardPlayerController.generated.h"

class ASnowboardCharacterBase;

/**
 * 
 */
UCLASS()
class SNOWBOARDKIDS_API ASnowboardPlayerController : public APlayerController
{
	GENERATED_BODY()

	ASnowboardPlayerController();

public:

	FORCEINLINE void SetRotationDisabled(bool Value) { bRotationDisabled = Value; }
	FORCEINLINE bool IsRotationDisabled() const { return bRotationDisabled; }
	
	virtual void SetupInputComponent();

protected:

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	UFUNCTION()
		void OnNorthPressed();

	UFUNCTION()
		void OnNorthReleased();

	UFUNCTION()
		void OnEastPressed();

	UFUNCTION()
		void OnEastReleased();

	UFUNCTION()
		void OnSouthPressed();

	UFUNCTION()
		void OnSouthReleased();

	UFUNCTION()
		void OnWestPressed();

	UFUNCTION()
		void OnWestReleased();

	UFUNCTION()
		void OnDPadNorthPressed();

	UFUNCTION()
		void OnDPadNorthReleased();

	UFUNCTION()
		void OnDPadEastPressed();

	UFUNCTION()
		void OnDPadEastReleased();

	UFUNCTION()
		void OnDPadSouthPressed();

	UFUNCTION()
		void OnDPadSouthReleased();

	UFUNCTION()
		void OnDPadWestPressed();

	UFUNCTION()
		void OnDPadWestReleased();

	UFUNCTION()
	void MoveForward(float Value);

	UFUNCTION()
	void MoveRight(float Value);

	UFUNCTION()
	void TurnAtRate(float Rate);

	UFUNCTION()
	void LookUpAtRate(float Rate);

	UFUNCTION()
	void AddControllerYawInput(float Value);

	UFUNCTION()
	void AddControllerPitchInput(float Value);

private:

	UPROPERTY(Transient)
	ASnowboardCharacterBase* PossessedPawn;

	bool bRotationDisabled;
};
