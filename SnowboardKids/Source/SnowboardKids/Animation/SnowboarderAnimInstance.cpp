// Lordmatics Games


#include "SnowboardKids/Animation/SnowboarderAnimInstance.h"

USnowboarderAnimInstance::USnowboarderAnimInstance() :
	bTorsoAnimation(false),
	bCharging(false),
	HorizontalTilt(0.0f),
	ForwardSpeed(0.0f),
	bHasGrabData(false),
	bHasGrabDataChanged(false),
	CurrentTrickVector(ETrickDirection::Max),
	CachedTrick(ETrickDirection::Max)
{

}

void USnowboarderAnimInstance::ResetTrickVector()
{
	CurrentTrickVector = ETrickDirection::Max;
	bHasGrabData = false;
	bHasGrabDataChanged = false;
}

void USnowboarderAnimInstance::SetTrickDirection(ETrickDirection Trick)
{
	if (Trick != CurrentTrickVector)
	{
		if (!bHasGrabData)
		{
			bHasGrabData = true;
			bHasGrabDataChanged = false;
		}
		else
		{
			bHasGrabDataChanged = true;
		}
		//bHasGrabDataChanged = true;
		CurrentTrickVector = Trick;
		CachedTrick = Trick;
	}
	else
	{
		bHasGrabDataChanged = false;
	}
}
