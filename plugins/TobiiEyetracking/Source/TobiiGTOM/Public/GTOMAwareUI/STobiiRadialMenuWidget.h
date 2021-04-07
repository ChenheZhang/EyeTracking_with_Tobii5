/******************************************************************************
* Copyright 2017- Tobii Technology AB. All rights reserved.
*
* @author Temaran | Fredrik Lindh | fredrik.lindh@tobii.com | https://github.com/Temaran
******************************************************************************/

#pragma once

#include "CoreMinimal.h"
#include "DisplayDebugHelpers.h"
#include "Slate/SRetainerWidget.h"
#include "Widgets/Layout/SBox.h"

class FArrangedChildren;
class FPaintArgs;
class FSlateWindowElementList;

struct FTobiiRadialMenuPanelRenderData
{
	TArray<FSlateVertex> PanelVertexData;
	TArray<SlateIndex> PanelIndexData;

	TArray<FSlateVertex> BorderVertexData; //This also contains the PanelVertexData in the end.
	TArray<SlateIndex> BorderIndexData;
};

//This is the underlying slate widget. 
class TOBIIGTOM_API STobiiRadialMenuWidget : public SPanel
{
public:
	class FSlot : public TSlotBase<FSlot>
	{
	public:
		FSlot& Offset(const TAttribute<FVector2D>& InOffset)
		{
			OffsetAttr = InOffset;
			return *this;
		}

		FSlot& Scale(const TAttribute<float>& InScale)
		{
			ScaleAttr = InScale;
			return *this;
		}

		FSlot& Alpha(const TAttribute<float>& InAlpha)
		{
			AlphaAttr = InAlpha;
			return *this;
		}

		/** Offset */
		TAttribute<FVector2D> OffsetAttr;

		/** Scale */
		TAttribute<float> ScaleAttr;

		/** Alpha */
		TAttribute<float> AlphaAttr;

		/** Default values for a slot. */
		FSlot()
			: TSlotBase<FSlot>()
			, OffsetAttr(FVector2D(0.0f, 0.0f))
			, ScaleAttr(1.0f)
			, AlphaAttr(1.0f)
		{ }
	};

	SLATE_BEGIN_ARGS(STobiiRadialMenuWidget)
		: _UseHardBorder(true)
		, _BorderColor(FColor(230, 230, 230))
		, _PanelColor(FColor(70, 70, 70))
		, _BorderThicknessPx(5.0f)
		, _SegmentSeparationPx(10.0f)
		, _AngularDisplacementDeg(0.0f)
		, _RadiusPx(500.0f)
		, _VertexCount(360)
	{
		_Visibility = EVisibility::SelfHitTestInvisible;
	}

		SLATE_SUPPORTS_SLOT(STobiiRadialMenuWidget::FSlot)
		SLATE_ARGUMENT(bool, UseHardBorder)
		SLATE_ARGUMENT(FColor, BorderColor)
		SLATE_ARGUMENT(FColor, PanelColor)
		SLATE_ARGUMENT(float, BorderThicknessPx)
		SLATE_ARGUMENT(float, SegmentSeparationPx)
		SLATE_ARGUMENT(float, AngularDisplacementDeg)
		SLATE_ARGUMENT(float, RadiusPx)
		SLATE_ARGUMENT(int32, VertexCount)

	SLATE_END_ARGS()

	STobiiRadialMenuWidget();

	void Construct(const FArguments& InArgs);

	static FSlot& Slot()
	{
		return *(new FSlot());
	}

	/**
	 * Adds a content slot.
	 *
	 * @return The added slot.
	 */
	FSlot& AddSlot()
	{
		Invalidate(EInvalidateWidget::Layout);

		STobiiRadialMenuWidget::FSlot& NewSlot = *(new FSlot());
		this->Children.Add(&NewSlot);
		return NewSlot;
	}

	/**
	 * Removes a particular content slot.
	 *
	 * @param SlotWidget The widget in the slot to remove.
	 */
	int32 RemoveSlot(const TSharedRef<SWidget>& SlotWidget);

	/**
	 * Removes all slots from the panel.
	 */
	void ClearChildren();

public:
	// Begin SWidget overrides
	virtual void OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const override;
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual FChildren* GetChildren() override;
	// End SWidget overrides

protected:
	// Begin SWidget overrides.
	virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override;
	// End SWidget overrides.

protected:
	/** The ConstraintCanvas widget's children. */
	TPanelChildren<FSlot> Children;

public:
	FSlateResourceHandle BrushResourceHandle;
	bool bUseHardBorder;
	FColor BorderColor;
	FColor PanelColor;
	float BorderThicknessPx;
	float SegmentSeparationPx;
	float AngularDisplacementDeg;
	float RadiusPx;
	int32 VertexCount;

private:
	void GeneratePanelRenderData(const FGeometry& AllottedGeometry, int32 NrChildren, TArray<FTobiiRadialMenuPanelRenderData>& OutChildRenderData) const;
};
