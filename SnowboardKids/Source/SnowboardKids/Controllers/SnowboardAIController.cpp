// Lordmatics Games


#include "SnowboardKids/Controllers/SnowboardAIController.h"
#include "../CustomCharacters/SnowboardCharacterBase.h"

ASnowboardAIController::ASnowboardAIController() :
	Super(),
	PossessedPawn(nullptr)
{

}

void ASnowboardAIController::BeginPlay()
{
	Super::BeginPlay();

	PossessedPawn = Cast<ASnowboardCharacterBase>(GetPawn());
}

void ASnowboardAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!PossessedPawn)
	{
		PossessedPawn = Cast<ASnowboardCharacterBase>(GetPawn());
	}
}
