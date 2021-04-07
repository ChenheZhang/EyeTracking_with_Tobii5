/******************************************************************************
* Copyright 2017- Tobii Technology AB. All rights reserved.
*
* @author Temaran | Fredrik Lindh | fredrik.lindh@tobii.com | https://github.com/Temaran
******************************************************************************/

#pragma once

#include "Slate/SRetainerWidget.h"
#include "TobiiGTOMTypes.h"
#include "STobiiRadialMenuWidget.h"

#include "CoreMinimal.h"
#include "DisplayDebugHelpers.h"
#include "Widgets/Layout/SBox.h"

class AHUD;
class UCanvas;
class UTobiiGazeFocusableWidget;

//This is the underlying slate widget. 
//WARNING: Please note that we do not support using these directly yet.
class TOBIIGTOM_API STobiiGazeFocusableWidget : public SBox
{
public:
	SLATE_BEGIN_ARGS(STobiiGazeFocusableWidget)
	{
		_Visibility = EVisibility::SelfHitTestInvisible;
	}
		SLATE_EVENT(FSimpleDelegate, OnHovered)
		SLATE_EVENT(FSimpleDelegate, OnUnhovered)

		/************************************************************************/
		/* From SBox widget                                                     */
		/************************************************************************/
		//Horizontal alignment of content in the area allotted to the SBox by its parent
		SLATE_ARGUMENT(EHorizontalAlignment, HAlign)

		//Vertical alignment of content in the area allotted to the SBox by its parent
		SLATE_ARGUMENT(EVerticalAlignment, VAlign)

		//Padding between the SBox and the content that it presents. Padding affects desired size.
		SLATE_ATTRIBUTE(FMargin, Padding)

		//The widget content presented by the SBox
		SLATE_DEFAULT_SLOT(FArguments, Content)
	SLATE_END_ARGS()

	/************************************************************************/
	/* Tobii                                                                */
	/************************************************************************/
public:
	TWeakObjectPtr<UTobiiGazeFocusableWidget> UMGWidget;

	STobiiGazeFocusableWidget();
	void Construct(const FArguments& InArgs);

	float GetCleanUIAlpha() { return CleanUIAlpha; }
	bool IsHoveredByPointer() { return bIsHovered; }
	bool HitByGaze();

	void AddExtraCleanUIContainerToPollHitsFrom(STobiiGazeFocusableWidget* CleanUIContainerToPoll, bool PollGazeHits = true, bool PollMouseHits = true);
	void RemoveExtraCleanUIContainerToPollHitsFrom(STobiiGazeFocusableWidget* CleanUIContainerToStopPolling);

	static STobiiGazeFocusableWidget* TryCastToTobiiGazeFocusable(SWidget* Widget);

	/************************************************************************/
	/* SBox                                                                 */
	/************************************************************************/
public:
	FSimpleDelegate OnHovered;
	FSimpleDelegate OnUnhovered;

	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual void OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override;

private:
	struct CleanUIPollingInfo
	{
		TWeakPtr<SWidget> CleanUIToPollFrom;
		bool PollGaze;
		bool PollPointer;
	};

	float CleanUIAlpha;

	TArray<CleanUIPollingInfo> CleanUIContainersToPollHitsFrom;
};
