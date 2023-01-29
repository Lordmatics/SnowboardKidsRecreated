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

	//virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	//virtual FVector2D ComputeDesiredSize(float) const override;
	//virtual void OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const override;

	///**
	// * Called to tell a widget to paint itself (and it's children).
	// *
	// * The widget should respond by populating the OutDrawElements array with FDrawElements
	// * that represent it and any of its children.
	// *
	// * @param Args              All the arguments necessary to paint this widget (@todo umg: move all params into this struct)
	// * @param AllottedGeometry  The FGeometry that describes an area in which the widget should appear.
	// * @param MyCullingRect    The clipping rectangle allocated for this widget and its children.
	// * @param OutDrawElements   A list of FDrawElements to populate with the output.
	// * @param LayerId           The Layer onto which this widget should be rendered.
	// * @param InColorAndOpacity Color and Opacity to be applied to all the descendants of the widget being painted
	// * @param bParentEnabled	True if the parent of this widget is enabled.
	// * @return The maximum layer ID attained by this widget or any of its children.
	// */
	//int32 Paint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const;

	///**
	// * Ticks this widget with Geometry.  Override in derived classes, but always call the parent implementation.
	// *
	// * @param  AllottedGeometry The space allotted for this widget
	// * @param  InCurrentTime  Current absolute real time
	// * @param  InDeltaTime  Real time passed since last tick
	// */
	//virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime);


	void CreateContent();

	void OnObjectSelected(UObject* Object);
	void OnBeginPIE(const bool bIsSimulating);
	void OnEndPIE(const bool bIsSimulating);
	void OnPausePIE(const bool bIsSimulating);

	UFUNCTION()
	void OnEditorSelectionChanged(UObject* SelectionThatChanged);

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
	TSharedPtr<SActorCollectionToTextWidget> ActorCollection;
//	TSharedRef<IToolkitHost> SharedRef;
};
