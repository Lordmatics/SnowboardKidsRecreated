// Lordmatics Games

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../CustomActors/Items/Utility/UtilityItem.h"
#include "../CustomActors/Items/Offensive/OffensiveItem.h"
#include "PlayerWidget.generated.h"

/**
 * 
 */
//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLapChanged, int, NewLap);

UCLASS(BlueprintType)
class SNOWBOARDKIDS_API UPlayerWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPlayerWidget(const FObjectInitializer& ObjectInitializer);
	
	// Return true if crossed the final lap
	bool UpdateLap(int InTotalLaps);
	void UpdatePosition(int InNewPosition);
	void AddCoins(int InCoins);
	void RemoveCoins(int InCoins);
	void UpdateUtility(EUtilityType Utility);
	void UpdateOffensive(EOffensiveType Offensive);

	bool CanAfford(int CoinCost) const { return Coins >= CoinCost; }
	int GetCoins() const { return Coins; }
	int GetPosition() const { return PositionInRace; }
	int GetLap() const { return CurrentLap; }
	EUtilityType GetUtility() const { return CurrentUtility; }
	EOffensiveType GetOffensive() const { return CurrentOffensive; }

protected:
	// C++ Function calls this one
	// BP Can handle changing the image, switching on the int.
	UFUNCTION(BlueprintImplementableEvent, Category = "Player UI")
		void BPLapChanged(int InLap, int InTotalLaps);

	UFUNCTION(BlueprintImplementableEvent, Category = "Player UI")
		void BPPositionChanged(int InPosition);

	UFUNCTION(BlueprintImplementableEvent, Category = "Player UI")
		void BPCoinsChanged(int InCoins);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Player UI")
		void BPUtilityChanged(EUtilityType Type);

	UFUNCTION(BlueprintImplementableEvent, Category = "Player UI")
		void BPOffensiveChanged(EOffensiveType Type);

private:

	/** What lap are we currently on. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player UI", meta = (AllowPrivateAccess = "true"))
	int CurrentLap;

	/** How many laps in total until we are done. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player UI", meta = (AllowPrivateAccess = "true"))
	int TotalLaps;

	/** What place are we at in the course */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player UI", meta = (AllowPrivateAccess = "true"))
	int PositionInRace;

	/** How many coins do we currently have.*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player UI", meta = (AllowPrivateAccess = "true"))
	int Coins;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player UI", meta = (AllowPrivateAccess = "true"))
	EUtilityType CurrentUtility;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player UI", meta = (AllowPrivateAccess = "true"))
	EOffensiveType CurrentOffensive;

	// Percentage along course via checkpoints - implement slider.
};
