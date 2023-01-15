// Lordmatics Games

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "SnowboardKids/Data/TrickData.h"
#include "SnowboarderAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class SNOWBOARDKIDS_API USnowboarderAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

	USnowboarderAnimInstance();

public:

	FORCEINLINE bool GetTorsoAnimations() const { return bTorsoAnimation; }
	FORCEINLINE void SetTorsoAnimation(bool Value) { bTorsoAnimation = Value; }

	FORCEINLINE bool IsCharging() const { return bCharging; }
	FORCEINLINE void SetCharging(bool Value) { bCharging = Value; }

	FORCEINLINE float GetTilt() const { return HorizontalTilt; }
	FORCEINLINE void SetTilt(float Value) { HorizontalTilt = Value; }

	FORCEINLINE float GetSpeed() const { return ForwardSpeed; }
	FORCEINLINE void SetSpeed(float Value) { ForwardSpeed = Value; }

	FORCEINLINE bool HasGrabData() const { return bHasGrabData; }
	void ResetTrickVector();
	void SetTrickDirection(ETrickDirection Trick);

private:

private:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation", meta = (AllowPrivateAccess = "true"))
	bool bTorsoAnimation;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation", meta = (AllowPrivateAccess = "true"))
	bool bCharging;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation", meta = (AllowPrivateAccess = "true"))
	float HorizontalTilt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation", meta = (AllowPrivateAccess = "true"))
	float ForwardSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation", meta = (AllowPrivateAccess = "true"))
	bool bHasGrabData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation", meta = (AllowPrivateAccess = "true"))
	bool bHasGrabDataChanged;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation", meta = (AllowPrivateAccess = "true"))
	ETrickDirection CurrentTrickVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation", meta = (AllowPrivateAccess = "true"))
	ETrickDirection CachedTrick;
};
