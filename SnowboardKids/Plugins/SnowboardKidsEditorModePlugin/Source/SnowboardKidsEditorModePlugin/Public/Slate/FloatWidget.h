// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include <Widgets/SCompoundWidget.h>

// Wrapper for a float property.
class SFloatPropertyWidget : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SFloatPropertyWidget) {}
		SLATE_ARGUMENT(FText, Text)
		SLATE_ARGUMENT(float, Value)
		SLATE_ARGUMENT(float, Min)
		SLATE_ARGUMENT(float, Max)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual bool SupportsKeyboardFocus() const override { return true; }

	// Numeric Slider Functions
	FORCEINLINE TOptional<float> GetScalarParameterSliderMin() const { return MinValue; }
	FORCEINLINE TOptional<float> GetScalarParameterSliderMax() const { return MaxValue; }
	FORCEINLINE void OnScalarParameterSlideBegin() {}
	FORCEINLINE void OnScalarParameterSlideEnd(const float NewValue) {}

	// Numeric Field Functions
	FORCEINLINE TOptional<float> GetOffsetValue() const { return FloatValue; }
	FORCEINLINE void SetOffsetValue(float NewValue) { FloatValue = NewValue; }
	FORCEINLINE void SetOffsetValueCommitted(const float NewValue, ETextCommit::Type CommitType) { SetOffsetValue(NewValue); }

private:
	float FloatValue;
	float MinValue;
	float MaxValue;
};