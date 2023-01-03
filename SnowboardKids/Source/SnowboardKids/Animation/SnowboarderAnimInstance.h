// Lordmatics Games

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
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

private:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation", meta = (AllowPrivateAccess = "true"))
	bool bTorsoAnimation;
};
