// Lordmatics Games


#include "SnowboardKids/Animation/SnowboarderAnimInstance.h"

USnowboarderAnimInstance::USnowboarderAnimInstance() :
	bTorsoAnimation(false),
	bCharging(false),
	HorizontalTilt(0.0f),
	ForwardSpeed(0.0f),
	bHasGrabData(false),
	bHasGrabDataChanged(false),
	CurrentTrickVector(ETrickDirection::Max)
{

}

void USnowboarderAnimInstance::ResetTrickVector()
{
	CurrentTrickVector = ETrickDirection::Max;
	bHasGrabData = false;
	bHasGrabDataChanged = false;
	//if (bHasGrabData || bHasGrabDataChanged)
	//{
	//	UE_LOG(LogTemp, Log, TEXT("ResetTrickVector"));
	//}	
}

void USnowboarderAnimInstance::SetTrickVector(const FTrickVector& InTrickVector)
{
	const float X = InTrickVector.X;
	const float Y = InTrickVector.Y;

	const bool bHorizontalOverVertical = X > 0.0f ? X > Y : X < Y;
	if (bHorizontalOverVertical)
	{
		if (X > 0.0f)
		{
			// Right
			SetTrickVectorInternal(ETrickDirection::East);
			//UE_LOG(LogTemp, Log, TEXT("Setting Grab Data East"));
			bHasGrabData = true;
			return;
		}
		else if (X < 0.0f)
		{
			// Left
			SetTrickVectorInternal(ETrickDirection::West);
			UE_LOG(LogTemp, Log, TEXT("Setting Grab Data West"));
			bHasGrabData = true;
			return;
		}
	}
	else
	{
		if (Y > 0.0f)
		{
			// Forward
			SetTrickVectorInternal(ETrickDirection::North);
			//UE_LOG(LogTemp, Log, TEXT("Setting Grab Data North"));
			bHasGrabData = true;
			return;
		}
		else if(Y < 0.0f)
		{
			// Backward
			SetTrickVectorInternal(ETrickDirection::South);
			//UE_LOG(LogTemp, Log, TEXT("Setting Grab Data South"));
			bHasGrabData = true;
			return;
		}
	}	

	bHasGrabData = false;
}

void USnowboarderAnimInstance::SetTrickVectorInternal(ETrickDirection InTrickDirection)
{
	if(InTrickDirection != CurrentTrickVector)	
	{
		bHasGrabDataChanged = true;
		CurrentTrickVector = InTrickDirection;
	}
	else
	{
		bHasGrabDataChanged = false;
	}
}
