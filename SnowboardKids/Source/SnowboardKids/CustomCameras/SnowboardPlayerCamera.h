// Lordmatics Games

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraActor.h"
#include "SnowboardPlayerCamera.generated.h"

class ASnowboardCharacterBase;

UCLASS()
class SNOWBOARDKIDS_API ASnowboardPlayerCamera : public ACameraActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASnowboardPlayerCamera();

	void UpdateCamera(const ASnowboardCharacterBase& Player);
	void ConstrainYawToPlayer(bool Value);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void BlendTowards(const FVector& TargetPos, float OverrideRate = -1.0f);
	void BlendTowards(float DeltaTime, const FVector& PlayerPos, float OverrideRate = -1.0f);
	void ClearBlend();
private:
	
private:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	float BehindOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	float VerticalOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	float ForwardBlendRate;

	bool bConstrainYawToPlayer;
};
