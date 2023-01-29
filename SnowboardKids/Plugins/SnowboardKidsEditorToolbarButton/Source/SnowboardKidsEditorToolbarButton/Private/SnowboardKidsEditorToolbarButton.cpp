// Copyright Epic Games, Inc. All Rights Reserved.

#include "SnowboardKidsEditorToolbarButton.h"
#include "SnowboardKidsEditorToolbarButtonStyle.h"
#include "SnowboardKidsEditorToolbarButtonCommands.h"
#include "Misc/MessageDialog.h"
#include "ToolMenus.h"

static const FName SnowboardKidsEditorToolbarButtonTabName("SnowboardKidsEditorToolbarButton");

#define LOCTEXT_NAMESPACE "FSnowboardKidsEditorToolbarButtonModule"

void FSnowboardKidsEditorToolbarButtonModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FSnowboardKidsEditorToolbarButtonStyle::Initialize();
	FSnowboardKidsEditorToolbarButtonStyle::ReloadTextures();

	FSnowboardKidsEditorToolbarButtonCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FSnowboardKidsEditorToolbarButtonCommands::Get().PluginAction,
		FExecuteAction::CreateRaw(this, &FSnowboardKidsEditorToolbarButtonModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FSnowboardKidsEditorToolbarButtonModule::RegisterMenus));
}

void FSnowboardKidsEditorToolbarButtonModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FSnowboardKidsEditorToolbarButtonStyle::Shutdown();

	FSnowboardKidsEditorToolbarButtonCommands::Unregister();
}

void FSnowboardKidsEditorToolbarButtonModule::PluginButtonClicked()
{
	// Put your "OnButtonClicked" stuff here
	FText DialogText = FText::Format(
							LOCTEXT("PluginButtonDialogText", "Add code to {0} in {1} to override this button's actions"),
							FText::FromString(TEXT("FSnowboardKidsEditorToolbarButtonModule::PluginButtonClicked()")),
							FText::FromString(TEXT("SnowboardKidsEditorToolbarButton.cpp"))
					   );
	FMessageDialog::Open(EAppMsgType::Ok, DialogText);
}

void FSnowboardKidsEditorToolbarButtonModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FSnowboardKidsEditorToolbarButtonCommands::Get().PluginAction, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FSnowboardKidsEditorToolbarButtonCommands::Get().PluginAction));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSnowboardKidsEditorToolbarButtonModule, SnowboardKidsEditorToolbarButton)