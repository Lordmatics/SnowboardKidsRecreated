// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "SnowboardKidsEditorToolbarButtonStyle.h"

class FSnowboardKidsEditorToolbarButtonCommands : public TCommands<FSnowboardKidsEditorToolbarButtonCommands>
{
public:

	FSnowboardKidsEditorToolbarButtonCommands()
		: TCommands<FSnowboardKidsEditorToolbarButtonCommands>(TEXT("SnowboardKidsEditorToolbarButton"), NSLOCTEXT("Contexts", "SnowboardKidsEditorToolbarButton", "SnowboardKidsEditorToolbarButton Plugin"), NAME_None, FSnowboardKidsEditorToolbarButtonStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > PluginAction;
};
