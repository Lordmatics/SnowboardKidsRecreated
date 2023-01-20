// Lordmatics Games


#include "SnowboardKids/CustomWidgets/PlayerWidget.h"

UPlayerWidget::UPlayerWidget(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer),
	CurrentLap(0),
	TotalLaps(3),
	PositionInRace(1),
	Coins(0)
{

}

bool UPlayerWidget::UpdateLap(int InTotalLaps)
{	
	if (CurrentLap >= InTotalLaps)
	{
		return true;
	}

	BPLapChanged(++CurrentLap, InTotalLaps);
	return false;
}

void UPlayerWidget::UpdatePosition(int InNewPosition)
{
	PositionInRace = InNewPosition;
	BPPositionChanged(InNewPosition);
}

void UPlayerWidget::AddCoins(int InCoins)
{
	Coins += InCoins;
	BPCoinsChanged(Coins);
}

void UPlayerWidget::RemoveCoins(int InCoins)
{
	Coins -= InCoins;
	BPCoinsChanged(Coins);
}
