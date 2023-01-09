// Lordmatics Games

#pragma once

#include "CoreMinimal.h"
#include "TrickData.generated.h"

UENUM(BlueprintType)
enum class ETrickDirection : uint8
{	
	North UMETA(ToolTip = "North"),
	East UMETA(ToolTip = "East"),
	South UMETA(ToolTip = "South"),
	West UMETA(ToolTip = "West"),
	NorthEast UMETA(ToolTip = "NorthEast"),
	SouthEast UMETA(ToolTip = "SouthEast"),
	SouthWest UMETA(ToolTip = "SouthWest"),
	NorthWest UMETA(ToolTip = "NorthWest"),
	Max	UMETA(Hidden)
};

struct SNOWBOARDKIDS_API FTrickVector
{
	FTrickVector();
	FTrickVector(float x, float y);
	FTrickVector(const FTrickVector& Other);

	FORCEINLINE float& operator[](int Index)
	{
		if (Index == 0)
		{
			return X;
		}
		return Y;
	};

	FORCEINLINE const float& operator[](int Index) const
	{
		if (Index == 0)
		{
			return X;
		}
		return Y;
	};

	void MakeZero();

	float X;
	float Y;
};
/**
 *
 */
USTRUCT(BlueprintType)
struct SNOWBOARDKIDS_API FTrickData
{
	GENERATED_BODY()

	FTrickData();

	int OnTrickPerformed(ETrickDirection Trick);
	int GenerateTrickScore(ETrickDirection Trick);
	void ResetTrickData();
	void SetTrickX(float Value);
	void SetTrickY(float Value);
	float GetTrickX() const { return TrickVector[0]; }
	float GetTrickY() const { return TrickVector[1]; }
	float GetCachedTrickX() const { return CachedTrickVector[0]; }
	float GetCachedTrickY() const { return CachedTrickVector[1]; }
	bool IsTrickBufferred() const { return TrickVector[0] != 0.0f || TrickVector[1] != 0.0f; }
	void UpdateCache();
	ETrickDirection GetTrickFromCache() const;

	int TrickCounter;
	bool UniqueTricksPerformed[8];
	ETrickDirection LastKnownTrickDirection;
	FTrickVector TrickVector;
	FTrickVector CachedTrickVector;	
	float RotationSpeed;
	float TrickTimer;
	float TimeForTrick;
	float TrickRotation;
};