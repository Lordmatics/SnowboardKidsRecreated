#include "Slate/ActorCollectionToTextWidget.h"
#include <Widgets/Input/SNumericEntryBox.h>

void SActorCollectionToTextWidget::Construct(const FArguments& InArgs)
{
	const TArray<AActor*>& Collection = InArgs._Collection;

	TSharedPtr<SVerticalBox> Container = SNew(SVerticalBox);
	UE_LOG(LogTemp, Log, TEXT("SActorCollectionToTextWidget::Construct"));

	for (AActor* CurrentSelectedActor : Collection)
	{
		if (!CurrentSelectedActor)
		{
			continue;
		}

		const FString& SelectedName = CurrentSelectedActor->GetName();
		UE_LOG(LogTemp, Log, TEXT("Actor: %s"), *SelectedName);
		Container->AddSlot()
		.AutoHeight()
		.Padding(10.0f, 1.0f, 10.0f, 1.0f)
		[	
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(5)
			[
				SNew(SBox)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("Selected: ")))
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(5)
			[
				SNew(SBox)
				[
					SNew(STextBlock)
					.Text(FText::FromString(SelectedName))
				]
			]
		];
	}
	
	ChildSlot
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				Container.ToSharedRef()
			]			
		];
};