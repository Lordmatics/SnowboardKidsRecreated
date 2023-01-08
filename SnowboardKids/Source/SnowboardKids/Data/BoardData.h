// Lordmatics Games

#pragma once

#include "CoreMinimal.h"
#include "BoardData.generated.h"

class UStaticMesh;

UENUM(BlueprintType)
enum class EBoardType : uint8
{
	FreeStyle UMETA(ToolTip = "FreeStyle"),
	AllAround UMETA(ToolTip = "AllAround"),
	Alpine UMETA(ToolTip = "Alpine"),
	Special UMETA(ToolTip = "Special")
};

/**
 * 
 */
USTRUCT(BlueprintType)
struct SNOWBOARDKIDS_API FBoardData
{
	GENERATED_BODY()

	FBoardData();
	FBoardData(EBoardType BoardType);

	/*
	* Forward Velocity.
	*/
	UPROPERTY(EditAnywhere, Category = "BoardData")
	float ForwardSpeed;

	/*
	* Direct Sideways Velocity
	*/
	UPROPERTY(EditAnywhere, Category = "BoardData")
	float HorizontalSpeed;

	/*
	* How quickly do we gain speed
	*/
	UPROPERTY(EditAnywhere, Category = "BoardData")
	float Acceleration;

	UPROPERTY(EditAnywhere, Category = "BoardData")
	float RecoverySpeed;

	UPROPERTY(EditAnywhere, Category = "BoardData")
	float MinTurnSpeed;

	/*
	* How fast can ForwardSpeed climb to.
	*/
	UPROPERTY(EditAnywhere, Category = "BoardData")
	float MaxSpeed;

	UPROPERTY(EditAnywhere, Category = "BoardData")
	float MaxSpeedWhenCharged;

	/*
	* How much vertical velocity should we apply when jumping
	*/
	UPROPERTY(EditAnywhere, Category = "BoardData")
	float JumpScale;

	UPROPERTY(EditAnywhere, Category = "BoardData")
	float JumpForwardScale;

	/*
	* How much downward velocity should we apply when falling
	*/
	UPROPERTY(EditAnywhere, Category = "BoardData")
	float GravityScale;

	/*
	* How much rotation should be applied to the character when turning.
	*/
	UPROPERTY(EditAnywhere, Category = "BoardData", meta = (ClampMin = -1.0f, ClampMax = 1.0f))
	float TurnLimit;

	UPROPERTY(EditAnywhere, Category = "BoardData", meta = (ClampMin = -1.0f, ClampMax = 1.0f))
	float TurnRateInterpSpeed;
};

USTRUCT(BlueprintType)
struct SNOWBOARDKIDS_API FBoardMeshes
{
	GENERATED_BODY()

	FBoardMeshes();

	UStaticMesh* GetMeshFromType(EBoardType BoardType) const;

	UPROPERTY(EditAnywhere, Category = "BoardData")
	UStaticMesh* FreeStyleMesh;

	UPROPERTY(EditAnywhere, Category = "BoardData")
	UStaticMesh* AllAroundMesh;

	UPROPERTY(EditAnywhere, Category = "BoardData")
	UStaticMesh* AlpineMesh;

	UPROPERTY(EditAnywhere, Category = "BoardData")
	UStaticMesh* SpecialMesh;
};