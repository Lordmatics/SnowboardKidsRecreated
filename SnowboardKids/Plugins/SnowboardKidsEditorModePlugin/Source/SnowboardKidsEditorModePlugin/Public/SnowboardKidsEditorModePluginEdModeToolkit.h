// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Toolkits/BaseToolkit.h"

//class UTest : public UUnrealEdEngine
//{
//
//};

class FSnowboardKidsEditorModePluginEdModeToolkit : public FModeToolkit
{
public:

	FSnowboardKidsEditorModePluginEdModeToolkit();
	virtual ~FSnowboardKidsEditorModePluginEdModeToolkit();

	/** FModeToolkit interface */
	virtual void Init(const TSharedPtr<IToolkitHost>& InitToolkitHost) override;		

	void CreateContent();

	void OnObjectSelected(UObject* Object);
	void OnBeginPIE(const bool bIsSimulating);
	void OnEndPIE(const bool bIsSimulating);
	void OnPausePIE(const bool bIsSimulating);

	/** IToolkit interface */
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual class FEdMode* GetEditorMode() const override;
	virtual TSharedPtr<class SWidget> GetInlineContent() const override { return ToolkitWidget; }

protected:
	virtual void OnToolStarted(UInteractiveToolManager* Manager, UInteractiveTool* Tool) override;
	virtual void OnToolEnded(UInteractiveToolManager* Manager, UInteractiveTool* Tool) override;

private:

	TSharedPtr<SBorder> ToolkitWidget;
	// This is important to store, as we want to dynamically refresh its content's
	//  This is done via reconstructing the widgets slate on an event.
	TSharedPtr<SActorCollectionToTextWidget> ActorCollection;

	TSharedPtr<SFloatPropertyWidget> FloatProperty;
};
