// Lordmatics Games


#include "SnowboardKids/CustomCameras/SnowboardPlayerCamera.h"
#include "../CustomCharacters/SnowboardCharacterBase.h"

// Sets default values
ASnowboardPlayerCamera::ASnowboardPlayerCamera() :
	BehindOffset(250.0f),
	VerticalOffset(55.0f),
	ForwardBlendRate(20.0f),
	bConstrainYawToPlayer(true)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void ASnowboardPlayerCamera::BeginPlay()
{
	Super::BeginPlay();
	
}

void ASnowboardPlayerCamera::BlendTowards(float DeltaTime, const FVector& PlayerPos, float OverrideRate)
{
	const FVector& CameraPosition = GetActorLocation();
	const float Rate = OverrideRate == -1.0f ? ForwardBlendRate : OverrideRate;
	const FVector& InterpolatedPosition = FMath::VInterpTo(CameraPosition, PlayerPos, DeltaTime, Rate);
	SetActorLocation(InterpolatedPosition);
}

void ASnowboardPlayerCamera::BlendTowards(const FVector& TargetPos, float OverrideRate)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const float DeltaTime = World->GetDeltaSeconds();
	BlendTowards(DeltaTime, TargetPos, OverrideRate);	
}

void ASnowboardPlayerCamera::ClearBlend()
{
}

void ASnowboardPlayerCamera::UpdateCamera(const ASnowboardCharacterBase& Player)
{
	const FVector& PlayerPosition = Player.GetActorLocation();
	if (bConstrainYawToPlayer)
	{
		FVector NewLocation = PlayerPosition;
		FVector ForwardVec = Player.GetActorForwardVector();
		ForwardVec.Z = 0.0f;
		NewLocation -= ForwardVec * BehindOffset;
		NewLocation += FVector::UpVector * VerticalOffset;

		BlendTowards(NewLocation);

		const FRotator& PlayerRot = Player.GetActorRotation();
		FRotator CameraRotation = FRotator(0.0f, PlayerRot.Yaw, 0.0f);
		SetActorRotation(CameraRotation);
	}	
	else
	{
		const FVector& CameraPosition = GetActorLocation();
		FVector DirToPlayer = PlayerPosition - CameraPosition;
		DirToPlayer.Normalize();
		DirToPlayer.Z = 0.0f;
		FVector NewLocation = PlayerPosition;
		NewLocation -= DirToPlayer * BehindOffset;
		NewLocation += FVector::UpVector * VerticalOffset;

		BlendTowards(NewLocation);
	}
}

void ASnowboardPlayerCamera::ConstrainYawToPlayer(bool Value)
{
	bConstrainYawToPlayer = Value;
}
