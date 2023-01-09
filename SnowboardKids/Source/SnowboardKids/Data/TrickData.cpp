// Lordmatics Games


#include "SnowboardKids/Data/TrickData.h"

FTrickData::FTrickData() :
	TrickCounter(0),
	LastKnownTrickDirection(ETrickDirection::Max),
	TrickVector(FTrickVector()),
	RotationSpeed(100.0f),
	TrickTimer(0.0f),
	TimeForTrick(1.33f),
	TrickRotation(0.0f)
{

}

int FTrickData::OnTrickPerformed(ETrickDirection Trick)
{
	const int TrickID = (int)Trick;
	UniqueTricksPerformed[TrickID] = true;
	TrickCounter++;
	LastKnownTrickDirection = Trick;
	return GenerateTrickScore(Trick);
}

int FTrickData::GenerateTrickScore(ETrickDirection Trick)
{
	return 100;
}

void FTrickData::ResetTrickData()
{
	TrickCounter = 0;
	for (int I = 0; I < (int)ETrickDirection::Max ; I++)
	{
		UniqueTricksPerformed[I] = false;
	}
	LastKnownTrickDirection = ETrickDirection::Max;
	TrickVector.MakeZero();
	TrickTimer = 0.0f;
	TrickRotation = 0.0f;
}

void FTrickData::SetTrickX(float Value)
{
	TrickVector[0] = Value;
}

void FTrickData::SetTrickY(float Value)
{
	TrickVector[1] = Value;
}

void FTrickData::UpdateCache()
{
	CachedTrickVector = TrickVector;
	TrickVector.MakeZero();
}

ETrickDirection FTrickData::GetTrickFromCache() const
{
	const float X = CachedTrickVector[0];
	const float Y = CachedTrickVector[1];
	ETrickDirection Trick;
	if (X > 0.75f)
	{
		// Forward Roll
		Trick = ETrickDirection::East;
	}
	else if (X < -0.75f)
	{
		// Backwards Roll
		Trick = ETrickDirection::West;
	}

	if (Y > 0.75f)
	{
		if (Trick == ETrickDirection::East)
		{
			Trick = ETrickDirection::NorthEast;
		}
		else if (Trick == ETrickDirection::West)
		{
			Trick = ETrickDirection::NorthWest;
		}
		else
		{
			Trick = ETrickDirection::North;
		}
	}
	else if(Y < -0.75f)
	{
		if (Trick == ETrickDirection::East)
		{
			Trick = ETrickDirection::SouthEast;
		}
		else if (Trick == ETrickDirection::West)
		{
			Trick = ETrickDirection::SouthWest;
		}
		else
		{
			Trick = ETrickDirection::South;
		}
	}
	return Trick;
}

FTrickVector::FTrickVector() :
	X(0.0f),
	Y(0.0f)
{

}

FTrickVector::FTrickVector(float x, float y) :
	X(x),
	Y(y)
{

}

FTrickVector::FTrickVector(const FTrickVector& Other) :
	X(Other.X),
	Y(Other.Y)
{

}

void FTrickVector::MakeZero()
{
	X = 0.0f;
	Y = 0.0f;
}
