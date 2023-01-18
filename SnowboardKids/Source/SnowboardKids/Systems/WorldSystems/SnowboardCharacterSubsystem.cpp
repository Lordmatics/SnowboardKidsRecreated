// Lordmatics Games


#include "SnowboardKids/Systems/WorldSystems/SnowboardCharacterSubsystem.h"
#include "SnowboardKids/CustomCharacters/SnowboardCharacterBase.h"
#include "SnowboardKids/Utils/GameUtils.h"

USnowboardCharacterSubsystem::USnowboardCharacterSubsystem() :
	Super(),
	RegisterredCharacters()
{

}

USnowboardCharacterSubsystem* USnowboardCharacterSubsystem::GetCharacterSystem(const UWorld* World)
{
	return UWorld::GetSubsystem<USnowboardCharacterSubsystem>(World);
}

USnowboardCharacterSubsystem* USnowboardCharacterSubsystem::GetCharacterSystem(const UWorld& World)
{
	return World.GetSubsystem<USnowboardCharacterSubsystem>();
}

void USnowboardCharacterSubsystem::RegisterCharacter(ASnowboardCharacterBase* Character)
{
	RegisterredCharacters.Push(Character);
}

void USnowboardCharacterSubsystem::UnRegisterCharacter(ASnowboardCharacterBase* Character)
{
	RegisterredCharacters.RemoveSingle(Character);
}

ASnowboardCharacterBase* USnowboardCharacterSubsystem::FindClosestCharacterWithinRange(const ASnowboardCharacterBase& Shooter, const AActor& Projectile, float ForwardRange, float SideRange) const
{
	if (RegisterredCharacters.Num() <= 1)
	{
		return nullptr;
	}

	// Little data structure to weight up the 2 targeting factors.
	// The narrower the target the better, as it won't have to seek pass a potential target to reach them
	// But if the distance between the narrower one is further significantly, then it should go for the wider target.
	//struct CharacterTargetWeight
	//{
	//	CharacterTargetWeight() : Ptr(nullptr), Dot(0.0f), Dist(0.0f) {}

	//	ASnowboardCharacterBase* Ptr;
	//	float Dot;
	//	float Dist;
	//};

	ASnowboardCharacterBase* BestCharacter = nullptr;
	float ClosestDistanceSqr = FLT_MAX;
	//float LastKnownDot = 1.0f;
	//TArray<CharacterTargetWeight> Weights;
	for (ASnowboardCharacterBase* CurrentCharacter : RegisterredCharacters)
	{
		if (!CurrentCharacter)
		{
			continue;
		}

		if (CurrentCharacter == &Shooter)
		{
			continue;
		}


		if (!CurrentCharacter->IsTargetable())
		{
			continue;
		}

		const FVector& CurrentTargetLocation = CurrentCharacter->GetActorLocation();
		//const FVector& ProjectileLocation = Projectile.GetActorLocation();
		const FTransform ProjectileTransform = Projectile.GetTransform();
		//const FVector DirToChar = CurrentTargetLocation - ProjectileLocation;
		//const float DistanceSquared = DirToChar.Size();
		//const float RangeSquared = 100.0f;// Range;// FMath::Square<float>(Range);

		const FVector& InverseTransformVec = GameUtils::GetMagnitudeOfLocalTransformToTarget(ProjectileTransform, CurrentTargetLocation);		
		const float ForwardDistance = InverseTransformVec.X;
		// Take the absolute, coz we can seek from either side
		const float SideDistance = FMath::Abs(InverseTransformVec.Y);
		const bool bWithinForwardRange = ForwardDistance > 0.0f && ForwardDistance <= ForwardRange;
		const bool bWithinSideRange = SideDistance <= SideRange;
			
		// Ok this needs to weight them.
		// Narrow is better, if distance is close.
		//UE_LOG(LogTemp, Log, TEXT("Dist: %.1f vs Range: %.1f"), DistanceSquared, RangeSquared);
		if (bWithinForwardRange && bWithinSideRange)
		{
			//const FVector DirNormalised = DirToChar.GetSafeNormal();
			//const FVector& ProjectileForward = Projectile.GetActorForwardVector();
			//float Dot = FVector::DotProduct(DirNormalised, ProjectileForward);
			//// Are they in front Enough ?
			//if(Dot > 0.5f)
			//{		
			//	CharacterTargetWeight NewWeight;
			//	NewWeight.Ptr = CurrentCharacter;
			//	NewWeight.Dot = Dot;
			//	NewWeight.Dist = DistanceSquared;
			//	Weights.Push(NewWeight);
			//}

			// Don't think i need to score this now, just take the closest adjacent
			if (SideDistance < ClosestDistanceSqr)
			{
				ClosestDistanceSqr = SideDistance;
				BestCharacter = CurrentCharacter;
			}
		}
	}

	//float Score = FLT_MIN;
	//float DotScore = 100.0f;
	//float DistanceScore = 150.0f;
	//float MaxDist = 100.0f;// Range;
	//for (CharacterTargetWeight& Weight : Weights)
	//{
	//	float LocalScore = 0.0f;

	//	const float Dot = Weight.Dot;
	//	const float Dist = Weight.Dist;

	//	// The narrower they are, the more score they get here.
	//	// basically, 50 ~ 100 points.
	//	LocalScore += Dot * DotScore;

	//	// Max 10000, Current = 2500, Result should be 0.25f;
	//	// So 1 - 0.25f = 0.75f, aka, closer distance, scores higher.
	//	const float MappedDist = 1.0f - GameUtils::MapValues(Dist, 0.0f, MaxDist, 0.0f, 1.0f);

	//	// The closer they are the more score they get here.
	//	// Closer to 0, gets 150.0f, closer to MaxRange, gets 0.0f
	//	LocalScore += DistanceScore * MappedDist;

	//	if (LocalScore >= Score)
	//	{
	//		Score = LocalScore;
	//		BestCharacter = Weight.Ptr;
	//	}
	//}
	return BestCharacter;
}
