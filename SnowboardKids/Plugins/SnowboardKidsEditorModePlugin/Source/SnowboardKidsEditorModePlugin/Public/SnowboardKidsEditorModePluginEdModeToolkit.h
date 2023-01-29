// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Toolkits/BaseToolkit.h"

class FSnowboardKidsEditorModePluginEdModeToolkit : public FModeToolkit
{
public:

	FSnowboardKidsEditorModePluginEdModeToolkit();
	
	/** FModeToolkit interface */
	virtual void Init(const TSharedPtr<IToolkitHost>& InitToolkitHost) override;

	/** IToolkit interface */
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual class FEdMode* GetEditorMode() const override;
	virtual TSharedPtr<class SWidget> GetInlineContent() const override { return ToolkitWidget; }

	// Numeric Slider Functions
	TOptional<float> GetScalarParameterSliderMin() const;
	TOptional<float> GetScalarParameterSliderMax() const;
	void OnScalarParameterSlideBegin();
	void OnScalarParameterSlideEnd(const float NewValue);

	// Numeric Field Functions
	FORCEINLINE TOptional<float> GetOffsetValue() const { return OffsetValue; }
	FORCEINLINE void SetOffsetValue(float NewValue) { OffsetValue = NewValue; }
	void SetOffsetValueCommitted(const float NewValue, ETextCommit::Type CommitType);

private:

	TSharedPtr<SWidget> ToolkitWidget;
	float OffsetValue;
};
