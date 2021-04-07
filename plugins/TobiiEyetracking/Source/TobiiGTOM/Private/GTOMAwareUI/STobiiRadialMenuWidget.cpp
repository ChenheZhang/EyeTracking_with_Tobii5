/******************************************************************************
* Copyright 2017- Tobii Technology AB. All rights reserved.
*
* @author Temaran | Fredrik Lindh | fredrik.lindh@tobii.com | https://github.com/Temaran
******************************************************************************/

#include "STobiiRadialMenuWidget.h"
#include "TobiiGTOMBlueprintLibrary.h"

#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Components/WidgetComponent.h"
#include "Framework/Application/SlateApplication.h"
#include "Slate/SceneViewport.h"

#define TWO_PI (6.28318530718)
#define PI_OVER_TWO (1.57079632679)

STobiiRadialMenuWidget::STobiiRadialMenuWidget()
	: Children(this)
{
	SetCanTick(false);
	bCanSupportFocus = false;
}

void STobiiRadialMenuWidget::Construct(const STobiiRadialMenuWidget::FArguments& InArgs)
{
	const int32 NumSlots = InArgs.Slots.Num();
	for (int32 SlotIndex = 0; SlotIndex < NumSlots; ++SlotIndex)
	{
		Children.Add(InArgs.Slots[SlotIndex]);
	}
	   
	bUseHardBorder = InArgs._UseHardBorder;
	BorderColor = InArgs._BorderColor;
	PanelColor = InArgs._PanelColor;
	BorderThicknessPx = InArgs._BorderThicknessPx;
	SegmentSeparationPx = InArgs._SegmentSeparationPx;
	AngularDisplacementDeg = InArgs._AngularDisplacementDeg;
	RadiusPx = InArgs._RadiusPx;
	VertexCount = InArgs._VertexCount;

	BrushResourceHandle = FSlateApplication::Get().GetRenderer()->GetResourceHandle(*FCoreStyle::Get().GetDefaultBrush());
}

void STobiiRadialMenuWidget::ClearChildren()
{
	if (Children.Num())
	{
		Invalidate(EInvalidateWidget::Layout);
		Children.Empty();
	}
}

int32 STobiiRadialMenuWidget::RemoveSlot(const TSharedRef<SWidget>& SlotWidget)
{
	Invalidate(EInvalidateWidget::Layout);

	for (int32 SlotIdx = 0; SlotIdx < Children.Num(); ++SlotIdx)
	{
		if (SlotWidget == Children[SlotIdx].GetWidget())
		{
			Children.RemoveAt(SlotIdx);
			return SlotIdx;
		}
	}

	return -1;
}

void STobiiRadialMenuWidget::GeneratePanelRenderData(const FGeometry& AllottedGeometry, int32 NrChildren, TArray<FTobiiRadialMenuPanelRenderData>& OutChildRenderData) const
{
	OutChildRenderData.Empty(NrChildren);

	const FVector2D WidgetCenter = AllottedGeometry.GetLocalSize() / 2.0f;
	const int32 VerticesPerSegment = FMath::Max(VertexCount / Children.Num(), 3);
	const float ActualRadius = FMath::Min(WidgetCenter.X, WidgetCenter.Y);
	const float AngleStep = TWO_PI / Children.Num();
	const float AngleHalfStep = AngleStep / 2.0f;
	const float AngleMinorStep = AngleStep / VerticesPerSegment;
	const float HalfSeparation = SegmentSeparationPx / 2.0f;
	const float BorderCenterOffsetDist = HalfSeparation / FMath::Sin(AngleHalfStep);
	const float BorderSeparationAngle = FMath::Asin(HalfSeparation / ActualRadius);

	const float NoBorderDistOffset = HalfSeparation + BorderThicknessPx;
	const float NoBorderRadius = ActualRadius - BorderThicknessPx;
	const float NoBorderCenterOffsetDist = NoBorderDistOffset / FMath::Sin(AngleHalfStep);
	const float NoBorderSeparationAngle = FMath::Asin(NoBorderDistOffset / NoBorderRadius);

	float CurrentChildAngle = PI_OVER_TWO + FMath::DegreesToRadians(AngularDisplacementDeg); //Start at the top
	
	for (int32 ChildIndex = 0; ChildIndex < NrChildren; ++ChildIndex)
	{
		FTobiiRadialMenuPanelRenderData NewRenderData;

		const STobiiRadialMenuWidget::FSlot& CurChild = Children[ChildIndex];
		const float ChildAlpha = CurChild.AlphaAttr.Get();

		FColor AdjustedBorderColor = BorderColor;
		AdjustedBorderColor.A *= ChildAlpha;
		FColor AdjustedPanelColor = PanelColor;
		AdjustedPanelColor.A *= ChildAlpha;

		//Panel polygon
		{
			float ChildSine, ChildCosine;
			FMath::SinCos(&ChildSine, &ChildCosine, -CurrentChildAngle);
			FVector2D WidgetCenterToChildCenterDir(ChildCosine, ChildSine);
			
			//First generate the nexus point
			const FVector2D CenterVertexPosition = WidgetCenter + WidgetCenterToChildCenterDir * NoBorderCenterOffsetDist;
			FSlateVertex CenterVertex = FSlateVertex::Make<ESlateVertexRounding::Enabled>(AllottedGeometry.GetAccumulatedRenderTransform(), CenterVertexPosition, FVector2D::ZeroVector, AdjustedPanelColor);
			NewRenderData.PanelVertexData.Add(CenterVertex);

			//Next we need to find our segment extreme points
			const float SegmentStartVertexAngle = CurrentChildAngle - AngleHalfStep + NoBorderSeparationAngle;
			const float SegmentEndVertexAngle = CurrentChildAngle + AngleHalfStep - NoBorderSeparationAngle;

			//Generate all edge vertices
			for (int32 VertexIdx = 0; VertexIdx < VerticesPerSegment; VertexIdx++)
			{
				const float AngleAlpha = (float)VertexIdx / (VerticesPerSegment - 1);
				const float CurrentAngle = FMath::Lerp(SegmentStartVertexAngle, SegmentEndVertexAngle, AngleAlpha);
				float VertexSine, VertexCosine;
				FMath::SinCos(&VertexSine, &VertexCosine, -CurrentAngle);

				const FVector2D SegmentVertexPosition = WidgetCenter + FVector2D(NoBorderRadius * VertexCosine, NoBorderRadius * VertexSine);
				FSlateVertex SegmentVertex = FSlateVertex::Make<ESlateVertexRounding::Enabled>(AllottedGeometry.GetAccumulatedRenderTransform(), SegmentVertexPosition, FVector2D::ZeroVector, AdjustedPanelColor);
				NewRenderData.PanelVertexData.Add(SegmentVertex);
			}

			//Build indices
			for (int32 VertexIdx = 1; VertexIdx < VerticesPerSegment; VertexIdx++)
			{
				NewRenderData.PanelIndexData.Add(0);
				NewRenderData.PanelIndexData.Add(VertexIdx);
				NewRenderData.PanelIndexData.Add(VertexIdx + 1);
			}
		}

		//Border polygon
		{
			float ChildSine, ChildCosine;
			FMath::SinCos(&ChildSine, &ChildCosine, -CurrentChildAngle);
			FVector2D WidgetCenterToChildCenterDir(ChildCosine, ChildSine);

			//First generate the nexus point
			const FVector2D CenterVertexPosition = WidgetCenter + WidgetCenterToChildCenterDir * BorderCenterOffsetDist;
			FSlateVertex CenterVertex = FSlateVertex::Make<ESlateVertexRounding::Enabled>(AllottedGeometry.GetAccumulatedRenderTransform(), CenterVertexPosition, FVector2D::ZeroVector, AdjustedBorderColor);
			NewRenderData.BorderVertexData.Add(CenterVertex);

			//Next we need to find our segment extreme points
			const float SegmentStartVertexAngle = CurrentChildAngle - AngleHalfStep + BorderSeparationAngle;
			const float SegmentEndVertexAngle = CurrentChildAngle + AngleHalfStep - BorderSeparationAngle;

			//Generate all edge vertices
			for (int32 VertexIdx = 0; VertexIdx < VerticesPerSegment; VertexIdx++)
			{
				const float AngleAlpha = (float)VertexIdx / (VerticesPerSegment - 1);
				const float CurrentAngle = FMath::Lerp(SegmentStartVertexAngle, SegmentEndVertexAngle, AngleAlpha);
				float VertexSine, VertexCosine;
				FMath::SinCos(&VertexSine, &VertexCosine, -CurrentAngle);

				const FVector2D SegmentVertexPosition = WidgetCenter + FVector2D(ActualRadius * VertexCosine, ActualRadius * VertexSine);

				FSlateVertex SegmentVertex = FSlateVertex::Make<ESlateVertexRounding::Enabled>(AllottedGeometry.GetAccumulatedRenderTransform(), SegmentVertexPosition, FVector2D::ZeroVector, AdjustedBorderColor);
				NewRenderData.BorderVertexData.Add(SegmentVertex);
			}

			//Add the panel vertices at the end of the array
			const int32 FirstPanelVertexIdx = NewRenderData.BorderVertexData.Num();
			for (FSlateVertex SlateVertex : NewRenderData.PanelVertexData)
			{
				if (bUseHardBorder)
				{
					SlateVertex.Color = AdjustedBorderColor;
				}

				NewRenderData.BorderVertexData.Add(SlateVertex);
			}

			//Build indices, building quads from the inner panel outwards
			for (int32 VertexIdx = 0; VertexIdx < VerticesPerSegment; VertexIdx++)
			{
				//First tri
				NewRenderData.BorderIndexData.Add(FirstPanelVertexIdx + VertexIdx);
				NewRenderData.BorderIndexData.Add(VertexIdx);
				NewRenderData.BorderIndexData.Add(VertexIdx + 1);

				//Second tri
				NewRenderData.BorderIndexData.Add(FirstPanelVertexIdx + VertexIdx);
				NewRenderData.BorderIndexData.Add(VertexIdx + 1);
				NewRenderData.BorderIndexData.Add(FirstPanelVertexIdx + VertexIdx + 1);
			}
			
			//Final two tris
			NewRenderData.BorderIndexData.Add(NewRenderData.BorderVertexData.Num() - 1);
			NewRenderData.BorderIndexData.Add(FirstPanelVertexIdx - 1);
			NewRenderData.BorderIndexData.Add(0);
			NewRenderData.BorderIndexData.Add(NewRenderData.BorderVertexData.Num() - 1);
			NewRenderData.BorderIndexData.Add(0);
			NewRenderData.BorderIndexData.Add(FirstPanelVertexIdx);
		}

		OutChildRenderData.Add(MoveTemp(NewRenderData));
		CurrentChildAngle += AngleStep;
	}
}

void STobiiRadialMenuWidget::OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const
{
	if (Children.Num() > 0)
	{
		const FVector2D WidgetCenter = AllottedGeometry.GetLocalSize() / 2.0f;
		const float ActualRadius = FMath::Min(WidgetCenter.X, WidgetCenter.Y);
		const float AngleStep = TWO_PI / Children.Num();		

		FVector2D LargestInscribedRect;
		float DistanceToRectCenter;
		UTobiiGTOMBlueprintLibrary::FindLargestInscribedAlignedRect(AngleStep, ActualRadius, LargestInscribedRect, DistanceToRectCenter);
		const float ChildElementSideLength = FMath::Min(LargestInscribedRect.X, LargestInscribedRect.Y);
		const float SlightlyScaledDownSize = ChildElementSideLength * 0.8f;

		float CurrentAngle = PI_OVER_TWO + FMath::DegreesToRadians(AngularDisplacementDeg); //Start at the top
		for (int32 ChildIndex = 0; ChildIndex < Children.Num(); ++ChildIndex)
		{
			const STobiiRadialMenuWidget::FSlot& CurChild = Children[ChildIndex];
			const TSharedRef<SWidget>& CurWidget = CurChild.GetWidget();

			const EVisibility ChildVisibility = CurWidget->GetVisibility();
			if (ArrangedChildren.Accepts(ChildVisibility))
			{
				const FVector2D Offset = CurChild.OffsetAttr.Get();
				float ChildSine, ChildCosine;
				FMath::SinCos(&ChildSine, &ChildCosine, -CurrentAngle);
				const FVector2D ChildCenter(DistanceToRectCenter * ChildCosine, DistanceToRectCenter * ChildSine);

				const float ChildScale = CurChild.ScaleAttr.Get();
				const FVector2D ChildSize(SlightlyScaledDownSize * ChildScale, SlightlyScaledDownSize * ChildScale);
				const FVector2D BaseChildOffset = -ChildSize / 2.0f;
				const FVector2D LocalPosition = WidgetCenter + ChildCenter + BaseChildOffset + Offset;
				CurrentAngle += AngleStep;

				ArrangedChildren.AddWidget(ChildVisibility, AllottedGeometry.MakeChild(CurWidget, LocalPosition, ChildSize));
			}
		}
	}
}

int32 STobiiRadialMenuWidget::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	SCOPED_NAMED_EVENT_TEXT("SConstraintCanvas", FColor::Orange);

	if (Children.Num() < 2)
	{
		return LayerId;
	}

	FArrangedChildren ArrangedChildren(EVisibility::Visible);
	OnArrangeChildren(AllottedGeometry, ArrangedChildren);

	TArray<FTobiiRadialMenuPanelRenderData> ChildRenderData;
	GeneratePanelRenderData(AllottedGeometry, Children.Num(), ChildRenderData);

	const bool bForwardedEnabled = ShouldBeEnabled(bParentEnabled);
	const float AngleStep = TWO_PI / Children.Num();

	float CurrentChildAngle = PI_OVER_TWO + FMath::DegreesToRadians(AngularDisplacementDeg); //Start at the top

	// Because we paint multiple children, we must track the maximum layer id that they produced in case one of our parents
	// wants to an overlay for all of its contents.
	int32 MaxLayerId = LayerId;
	const FPaintArgs NewArgs = Args.WithNewParent(this);
 	for (int32 ChildIndex = 0; ChildIndex < ArrangedChildren.Num(); ++ChildIndex)
 	{
 		FArrangedWidget& CurWidget = ArrangedChildren[ChildIndex]; 
 		if (!IsChildWidgetCulled(MyCullingRect, CurWidget))
 		{
			MaxLayerId++;

			//Draw underlying panel if our render data is valid
			if (ChildIndex < ChildRenderData.Num())
			{
				const FTobiiRadialMenuPanelRenderData& CurrentRenderData = ChildRenderData[ChildIndex];

				FSlateDrawElement::MakeCustomVerts(OutDrawElements, LayerId
					, BrushResourceHandle
					, CurrentRenderData.PanelVertexData
					, CurrentRenderData.PanelIndexData
					, nullptr, 0, 0, ESlateDrawEffect::NoPixelSnapping);

 				FSlateDrawElement::MakeCustomVerts(OutDrawElements, LayerId
 					, BrushResourceHandle
 					, CurrentRenderData.BorderVertexData
 					, CurrentRenderData.BorderIndexData
 					, nullptr, 0, 0, ESlateDrawEffect::NoPixelSnapping);
			}
			
			const STobiiRadialMenuWidget::FSlot& CurChild = Children[ChildIndex];
			FWidgetStyle CompoundedWidgetStyle = FWidgetStyle(InWidgetStyle).BlendColorAndOpacityTint(FLinearColor(1.0f, 1.0f, 1.0f, CurChild.AlphaAttr.Get()));
 			CurWidget.Widget->Paint(NewArgs, CurWidget.Geometry, MyCullingRect, OutDrawElements, MaxLayerId, CompoundedWidgetStyle, bForwardedEnabled);
			CurrentChildAngle += AngleStep;
 		}
 	}

	return MaxLayerId;
}

FVector2D STobiiRadialMenuWidget::ComputeDesiredSize(float LayoutScaleMultiplier) const
{
	const float DiameterPx = RadiusPx * 2.0f;
	return FVector2D(DiameterPx, DiameterPx);
}

FChildren* STobiiRadialMenuWidget::GetChildren()
{
	return &Children;
}
