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

private:
};