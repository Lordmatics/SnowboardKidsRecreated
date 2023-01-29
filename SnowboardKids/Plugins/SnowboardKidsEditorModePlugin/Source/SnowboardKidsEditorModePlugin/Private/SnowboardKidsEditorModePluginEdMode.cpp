// Copyright Epic Games, Inc. All Rights Reserved.

#include "SnowboardKidsEditorModePluginEdMode.h"
#include "SnowboardKidsEditorModePluginEdModeToolkit.h"
#include "Toolkits/ToolkitManager.h"
#include "EditorModeManager.h"

const FEditorModeID FSnowboardKidsEditorModePluginEdMode::EM_SnowboardKidsEditorModePluginEdModeId = TEXT("EM_SnowboardKidsEditorModePluginEdMode");

FSnowboardKidsEditorModePluginEdMode::FSnowboardKidsEditorModePluginEdMode()
{

}

FSnowboardKidsEditorModePluginEdMode::~FSnowboardKidsEditorModePluginEdMode()
{

}

void FSnowboardKidsEditorModePluginEdMode::Enter()
{
	FEdMode::Enter();

	if (!Toolkit.IsValid() && UsesToolkits())
	{
		Toolkit = MakeShareable(new FSnowboardKidsEditorModePluginEdModeToolkit);
		Toolkit->Init(Owner->GetToolkitHost());
	}
}

void FSnowboardKidsEditorModePluginEdMode::Exit()
{
	if (Toolkit.IsValid())
	{
		FToolkitManager::Get().CloseToolkit(Toolkit.ToSharedRef());
		Toolkit.Reset();
	}

	// Call base Exit method to ensure proper cleanup
	FEdMode::Exit();
}

bool FSnowboardKidsEditorModePluginEdMode::UsesToolkits() const
{
	return true;
}




