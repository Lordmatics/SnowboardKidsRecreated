// Lordmatics Games

#pragma once

#include "CoreMinimal.h"


namespace GameUtils
{
	template<typename T>
	void EnumString(const T& EnumValue, FName& OutName)
	{
		FName UnformattedName;
		UEnum::GetValueAsName(EnumValue, UnformattedName);

		FString NameAsString = UnformattedName.ToString();
		int Index = 0;
		NameAsString.FindChar(':', Index);
		if (Index == INDEX_NONE)
		{
			OutName = UnformattedName;
			return;
		}
		// Got 2 colons, so increment twice to get infront of both.
		Index++;
		Index++;

		NameAsString.RemoveAt(0, Index);
		OutName = FName(*NameAsString);		
	}

	// Translate a ratio to a different ratio.
	float MapValues(float Value, float MinValue, float MaxValue, float TargetMin, float TargetMax);	

	// This works by formulating a right angled triangle between us and a target.
	// Can use the inverse transform to achieve very similiar results.
	FVector SplitDirectionMagnitudes(const AActor& OwnerActor, const FVector& DirNormalised, const float DirMagnitude);
	bool IsTargetOnMyRight(const FTransform& OwnerTransform, const FVector& Target, float& Value);
	// Unrotates our vectors to determine the local differences in length between us and a target.
	FVector GetMagnitudeOfLocalTransformToTarget(const FTransform& OwnerTransform, const FVector& Target);
}