// Lordmatics Games

#pragma once

#include "CoreMinimal.h"
#include <UObject/Interface.h>
#include "CharacterInterface.generated.h"

/**
 * 
 */
UINTERFACE()
class SNOWBOARDKIDS_API UCharacterInterface : public UInterface
{
	GENERATED_UINTERFACE_BODY()
};

class SNOWBOARDKIDS_API ICharacterInterface
{
	GENERATED_IINTERFACE_BODY()

	virtual bool ConsumeItemOffensive() { return false; }
	virtual bool ConsumeItemUtility() { return false; }
};