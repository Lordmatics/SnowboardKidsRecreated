// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include <Widgets/SCompoundWidget.h>

class SActorCollectionToTextWidget : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SActorCollectionToTextWidget) {}
		SLATE_ARGUMENT(TArray<AActor*>, Collection)		
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual bool SupportsKeyboardFocus() const override { return true; }
	void UpdateSelected(const TArray<AActor*>& InSelectedObjects);
	
	// SWidgetOverrides
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual FChildren* GetChildren() override;
	virtual void OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const override;
	virtual FSlateColor GetForegroundColor() const override;

private:
	TSharedPtr<SVerticalBox> Container;
	TArray<AActor*> SelectedObjects;
};