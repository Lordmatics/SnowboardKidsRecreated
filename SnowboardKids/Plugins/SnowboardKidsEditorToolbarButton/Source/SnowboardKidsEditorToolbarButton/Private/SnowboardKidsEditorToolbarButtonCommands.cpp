// Copyright Epic Games, Inc. All Rights Reserved.

#include "SnowboardKidsEditorToolbarButtonCommands.h"

#define LOCTEXT_NAMESPACE "FSnowboardKidsEditorToolbarButtonModule"

void FSnowboardKidsEditorToolbarButtonCommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "SnowboardKidsEditorToolbarButton", "Execute SnowboardKidsEditorToolbarButton action", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
