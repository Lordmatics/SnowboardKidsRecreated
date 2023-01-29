// Copyright Epic Games, Inc. All Rights Reserved.

#include "SnowboardKidsEditorModePlugin.h"
#include "SnowboardKidsEditorModePluginEdMode.h"

#define LOCTEXT_NAMESPACE "FSnowboardKidsEditorModePluginModule"

void FSnowboardKidsEditorModePluginModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	FEditorModeRegistry::Get().RegisterMode<FSnowboardKidsEditorModePluginEdMode>(FSnowboardKidsEditorModePluginEdMode::EM_SnowboardKidsEditorModePluginEdModeId, LOCTEXT("SnowboardKidsEditorModePluginEdModeName", "SnowboardKidsEditorModePluginEdMode"), FSlateIcon(), true);
}

void FSnowboardKidsEditorModePluginModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	FEditorModeRegistry::Get().UnregisterMode(FSnowboardKidsEditorModePluginEdMode::EM_SnowboardKidsEditorModePluginEdModeId);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSnowboardKidsEditorModePluginModule, SnowboardKidsEditorModePlugin)