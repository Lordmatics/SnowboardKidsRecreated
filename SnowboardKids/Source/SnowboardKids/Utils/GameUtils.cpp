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

FVector GameUtils::SplitDirectionMagnitudes(const AActor& OwnerActor, const FVector& DirNormalised, const float DirMagnitude)
{
	// Ok Given the direction to the target, find the angle, then use the angle to create a right triangle.
	// From the right triangle, we can use trigonometry to determine the magnitudes of the sides forming the triangle.
	// These magnitudes are the x and y values respectively.
	const FVector& OwnerLocation = OwnerActor.GetActorLocation();
	const FVector& ForwardVector = OwnerActor.GetActorForwardVector();
	const FVector& RightVector = OwnerActor.GetActorRightVector();
	FVector LeftVector = -RightVector;

	float InFront = FVector::DotProduct(ForwardVector, DirNormalised);
	if (InFront == 0.0f)
	{
		// Shoot straight
		return FVector::ZeroVector;
	}

	float YValue = 0.0f;
	const FTransform& OwnerTransform = OwnerActor.GetTransform();
	const bool bOnRight = GameUtils::IsTargetOnMyRight(OwnerTransform, OwnerLocation + DirNormalised, YValue);
	FVector SideVector = LeftVector;
	if (bOnRight)
	{
		// Use Right Vector
		SideVector = RightVector;
	}

	float AngleBetweenDirAndSide = FVector::DotProduct(DirNormalised, SideVector);
	AngleBetweenDirAndSide = FMath::Acos(AngleBetweenDirAndSide);
	AngleBetweenDirAndSide = FMath::RadiansToDegrees(AngleBetweenDirAndSide);

	const float FinalAngle = 90.0f - AngleBetweenDirAndSide;
	float XOne = DirMagnitude * FMath::Sin(FinalAngle);
	float YTwo = DirMagnitude * FMath::Cos(FinalAngle);

	// Construct Final Vector
	FVector FinalVec = FVector(XOne, YTwo, 0.0f);
	return FinalVec;
}

bool GameUtils::IsTargetOnMyRight(const FTransform& OwnerTransform, const FVector& Target, float& Value)
{
	float YValue = OwnerTransform.InverseTransformPosition(Target).Y;
	bool bOnRightSide = YValue < 0.0f ? false : true;
	Value = YValue;
	return bOnRightSide;
}

FVector GameUtils::GetMagnitudeOfLocalTransformToTarget(const FTransform& OwnerTransform, const FVector& Target)
{
	FVector InverseVec = OwnerTransform.InverseTransformPosition(Target);
	// Essentially, ActorForwardVector * Inverse.X = How far forward from our position to be adjacent to the target
	// ActorRightVector * Inverse.Y = How far sideways from our position to be in front/behind to the target
	// ActorUpVEctor * Inverse.Z = How far vertically from our position to be in line to the target.
	return InverseVec;
}