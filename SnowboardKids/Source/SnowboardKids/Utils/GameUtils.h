// Lordmatics Games

#pragma once

#include "CoreMinimal.h"


namespace GameUtils
{
	template<typename T>
	void EnumString(const T& EnumValue, FName& OutName);

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

	float MapValues(float Value, float MinValue, float MaxValue, float TargetMin, float TargetMax);	
}