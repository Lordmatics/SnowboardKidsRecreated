#include "Slate/FloatWidget.h"
#include <Widgets/Input/SNumericEntryBox.h>

void SFloatPropertyWidget::Construct(const FArguments& InArgs)
{
	const float ArgValue = InArgs._Value;
	SetOffsetValue(ArgValue);
	const FText& LocalText = InArgs._Text;

	const float ArgMin = InArgs._Min;
	const float ArgMax = InArgs._Max;
	MinValue = ArgMin;
	MaxValue = ArgMax;

	ChildSlot
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(10.0f, 1.0f, 10.0f, 1.0f)
			[
				SNew(SBox)
				[
					SNew(STextBlock)
					.Text(LocalText)
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(10.0f, 1.0f, 10.0f, 1.0f)
			[
				SNew(SNumericEntryBox<float>)
				.AllowSpin(true)
				.MinValue(TOptional<float>())
				.MaxValue(TOptional<float>())
				.MinSliderValue(this, &SFloatPropertyWidget::GetScalarParameterSliderMin)
				.MaxSliderValue(this, &SFloatPropertyWidget::GetScalarParameterSliderMax)
				.Delta(0.1f)
				.Value(this, &SFloatPropertyWidget::GetOffsetValue)
				.OnBeginSliderMovement(this, &SFloatPropertyWidget::OnScalarParameterSlideBegin)
				.OnEndSliderMovement(this, &SFloatPropertyWidget::OnScalarParameterSlideEnd)
				.OnValueChanged(this, &SFloatPropertyWidget::SetOffsetValue)
				.OnValueCommitted(this, &SFloatPropertyWidget::SetOffsetValueCommitted)
			]
		];
}