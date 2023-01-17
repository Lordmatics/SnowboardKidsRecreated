// Lordmatics Games


#include "SnowboardKids/Utils/GameUtils.h"

float GameUtils::MapValues(float Value, float MinValue, float MaxValue, float TargetMin, float TargetMax)
{
	const float denominator = MaxValue - MinValue;
	if (denominator == 0.0f)
	{
		return Value;
	}
	float ratio = (TargetMax - TargetMin) / denominator;
	return TargetMin + ratio * (Value - MinValue);
}
