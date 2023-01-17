// Lordmatics Games

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SnowboardPlayerController.generated.h"

class ASnowboardCharacterBase;
class ASnowboardPlayerCamera;

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

	void ConstrainYawToPlayer(bool Value);

protected:

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void PlayerTick(float DeltaTime) override;

private:

	void ProcessCamera(float DeltaTime);

private:
	UFUNCTION()
		void OnRightTriggerPressed();
	
	UFUNCTION()
		void OnRightTriggerReleased();

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

	UPROPERTY(EditAnywhere, Category = "Camera")
	TSubclassOf<ASnowboardPlayerCamera> CameraClass;

	UPROPERTY(Transient)
	ASnowboardCharacterBase* PossessedPawn;

	UPROPERTY(Transient)
	ASnowboardPlayerCamera* PlayerCamera;

	bool bRotationDisabled;
};
