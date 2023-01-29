#include "Slate/ActorCollectionToTextWidget.h"
#include <Widgets/Input/SNumericEntryBox.h>

void SActorCollectionToTextWidget::Construct(const FArguments& InArgs)
{
	const TArray<AActor*>& Collection = InArgs._Collection;
	SelectedObjects = Collection;

	UpdateContents();	
};

void SActorCollectionToTextWidget::UpdateSelected(const TArray<AActor*>& InSelectedObjects)
{	
	Container.Reset();	
	SelectedObjects = InSelectedObjects;
	UpdateContents();
}

int32 SActorCollectionToTextWidget::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	int32 Result = SCompoundWidget::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);	
	return Result;
}

FChildren* SActorCollectionToTextWidget::GetChildren()
{
	return SCompoundWidget::GetChildren();
}

void SActorCollectionToTextWidget::OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const
{
	SCompoundWidget::OnArrangeChildren(AllottedGeometry, ArrangedChildren);
}

FSlateColor SActorCollectionToTextWidget::GetForegroundColor() const
{
	return SCompoundWidget::GetForegroundColor();
}

void SActorCollectionToTextWidget::UpdateContents()
{
	Container = SNew(SVerticalBox);
	UE_LOG(LogTemp, Log, TEXT("SActorCollectionToTextWidget::Construct"));

	for (AActor* CurrentSelectedActor : SelectedObjects)
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
}
