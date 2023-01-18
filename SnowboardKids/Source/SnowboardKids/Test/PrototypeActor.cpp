// Lordmatics Games


#include "SnowboardKids/Test/PrototypeActor.h"
#include <DrawDebugHelpers.h>
#include "../Utils/GameUtils.h"

// Sets default values
APrototypeActor::APrototypeActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void APrototypeActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APrototypeActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (ActorA && ActorB && ActorC && GEngine)
	{
		const FVector& ActorALocation = ActorA->GetActorLocation();
		const FVector& ActorBLocation = ActorB->GetActorLocation();
		const FVector& ActorCLocation = ActorC->GetActorLocation();

		const float DrawLifetime = 0.1f;
		FColor Green = FColor::Green;
		FColor Red = FColor::Red;
		FColor Yellow = FColor::Yellow;
		FColor Blue = FColor::Blue;
		DrawDebugSphere(World, ActorALocation, 25.0f, 4, Green, false, DrawLifetime);
		DrawDebugSphere(World, ActorBLocation, 25.0f, 4, Red, false, DrawLifetime);
		DrawDebugSphere(World, ActorCLocation, 12.5f, 4, Yellow, false, DrawLifetime);

		FVector SeekDir = ActorBLocation - ActorALocation;
		DrawDebugLine(World, ActorALocation, ActorBLocation, Green, false, DrawLifetime);

		// Draw Local Transform
		const float NormalLength = 150.0f;
		const FVector& AForward = ActorA->GetActorForwardVector() * NormalLength;
		const FVector& ARight = ActorA->GetActorRightVector() * NormalLength;
		DrawDebugLine(World, ActorALocation, ActorALocation + AForward, Red, false, DrawLifetime);
		DrawDebugLine(World, ActorALocation, ActorALocation + ARight, Green, false, DrawLifetime);

		// Wahts the angle between forward + dir
		// Dot = 1 if the target is straight in front of the vector, -1 for straight behind
		const float ForwardDot = FVector::DotProduct(AForward.GetSafeNormal(), SeekDir.GetSafeNormal());
		DrawDebugString(World, ActorALocation, FString::Printf(TEXT("ForwardDot: %.1f"), ForwardDot), nullptr, Blue, DrawLifetime);

		const float RightDot = FVector::DotProduct(ARight.GetSafeNormal(), SeekDir.GetSafeNormal());
		DrawDebugString(World, ActorALocation - FVector(0.0f, 0.0f, 150.0f), FString::Printf(TEXT("RightDot: %.1f"), RightDot), nullptr, Blue, DrawLifetime);

		
		const FTransform& ActorATransform = ActorA->GetTransform();		
		float YValue = 0.0f;
		bool bOnRightSide = GameUtils::IsTargetOnMyRight(ActorATransform, ActorBLocation, YValue);
		// If < 0, left, if > 0 right
		DrawDebugString(World, ActorALocation - FVector(0.0f, 0.0f, 250.0f), FString::Printf(TEXT("OnRightSide: %.1f, %s"), YValue, bOnRightSide ? TEXT("True") : TEXT("False")));

		
		FVector SideVector = ActorA->GetActorRightVector() * YValue;
		DrawDebugLine(World, ActorALocation, ActorALocation + SideVector, FColor::Cyan, false, DrawLifetime);
		DrawDebugSphere(World, ActorALocation + SideVector, 25.0f, 4, FColor::Cyan, false, DrawLifetime);

		FVector InversePosToTarget = ActorATransform.InverseTransformPosition(ActorBLocation);
		float XValue = InversePosToTarget.X;

		FVector FrontVector = ActorA->GetActorForwardVector() * XValue;
		DrawDebugLine(World, ActorALocation, ActorALocation + FrontVector, FColor::Cyan, false, DrawLifetime);
		DrawDebugSphere(World, ActorALocation + FrontVector, 25.0f, 4, FColor::Cyan, false, DrawLifetime);

		FVector Start = ActorALocation + SideVector;
		FVector End = Start + FrontVector;
		DrawDebugLine(World, Start, End, FColor::Green, false, DrawLifetime);
		DrawDebugLine(World, ActorALocation + SideVector, ActorBLocation, FColor::Red, false, DrawLifetime);
		//DrawDebugSphere(World, ActorALocation + FrontVector, 25.0f, 4, FColor::Cyan, false, DrawLifetime);



	}
}

