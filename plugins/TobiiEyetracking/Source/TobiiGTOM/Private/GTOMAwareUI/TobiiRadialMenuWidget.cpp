/******************************************************************************
* Copyright 2017- Tobii Technology AB. All rights reserved.
*
* @author Temaran | Fredrik Lindh | fredrik.lindh@tobii.com | https://github.com/Temaran
******************************************************************************/

#include "TobiiRadialMenuWidget.h"
#include "TobiiGazeFocusableWidget.h"
#include "STobiiGazeFocusableWidget.h"

#define LOCTEXT_NAMESPACE "UMG"

UTobiiRadialMenuSlot::UTobiiRadialMenuSlot(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, Offset(0.0f, 0.0f)
	, Scale(1.0f)
	, Alpha(1.0f)

	, Slot(nullptr)
{
}

void UTobiiRadialMenuSlot::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	Slot = nullptr;
}

void UTobiiRadialMenuSlot::BuildSlot(TSharedRef<STobiiRadialMenuWidget> RadialMenu)
{
	Slot = &RadialMenu->AddSlot()
		[
			Content == nullptr ? SNullWidget::NullWidget : Content->TakeWidget()
		];

	SynchronizeProperties();
}

#if WITH_EDITOR

bool UTobiiRadialMenuSlot::NudgeByDesigner(const FVector2D& NudgeDirection, const TOptional<int32>& GridSnapSize)
{
	return false;
}

bool UTobiiRadialMenuSlot::DragDropPreviewByDesigner(const FVector2D& LocalCursorPosition, const TOptional<int32>& XGridSnapSize, const TOptional<int32>& YGridSnapSize)
{
	return false;
}

void UTobiiRadialMenuSlot::SynchronizeFromTemplate(const UPanelSlot* const TemplateSlot)
{
}

#endif //WITH_EDITOR

void UTobiiRadialMenuSlot::SetOffset(FVector2D InOffset)
{
	Offset = InOffset;
	if (Slot)
	{
		Slot->Offset(InOffset);
	}
}
FVector2D UTobiiRadialMenuSlot::GetOffset() const
{
	if (Slot)
	{
		return Slot->OffsetAttr.Get();
	}

	return FVector2D::ZeroVector;
}

void UTobiiRadialMenuSlot::SetScale(float InScale)
{
	Scale = InScale;
	if (Slot)
	{
		Slot->Scale(InScale);
	}
}
float UTobiiRadialMenuSlot::GetScale() const
{
	if (Slot)
	{
		return Slot->ScaleAttr.Get();
	}

	return 1.0f;
}

void UTobiiRadialMenuSlot::SetAlpha(float InAlpha)
{
	Alpha = InAlpha;
	if (Slot)
	{
		Slot->Alpha(InAlpha);
	}
}
float UTobiiRadialMenuSlot::GetAlpha() const
{
	if (Slot)
	{
		return Slot->AlphaAttr.Get();
	}

	return 1.0f;
}

void UTobiiRadialMenuSlot::SynchronizeProperties()
{
	SetOffset(Offset);
	SetScale(Scale);
	SetAlpha(Alpha);
}

#if WITH_EDITOR

void UTobiiRadialMenuSlot::PreEditChange(class FEditPropertyChain& PropertyAboutToChange)
{
	Super::PreEditChange(PropertyAboutToChange);
}
void UTobiiRadialMenuSlot::PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent)
{
	SynchronizeProperties();
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////

UTobiiRadialMenuWidget::UTobiiRadialMenuWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bUseHardBorder(true)
	, BorderColor(FColor(230, 230, 230))
	, PanelColor(FColor(70, 70, 70))
	, BorderThicknessPx(5.0f)
	, SegmentSeparationPx(10.0f)
	, AngularDisplacementDeg(0.0f)
	, RadiusPx(500.0f)
	, VertexCount(360)
{
	bIsVariable = false;

	STobiiRadialMenuWidget::FArguments Defaults;
	Visibility = UWidget::ConvertRuntimeToSerializedVisibility(Defaults._Visibility.Get());
}

void UTobiiRadialMenuWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	MyRadialMenu.Reset();
}

TSharedRef<SWidget> UTobiiRadialMenuWidget::RebuildWidget()
{
	MyRadialMenu = SNew(STobiiRadialMenuWidget)
		.UseHardBorder(bUseHardBorder)
		.BorderColor(BorderColor)
		.PanelColor(PanelColor)
		.BorderThicknessPx(BorderThicknessPx)
		.SegmentSeparationPx(SegmentSeparationPx)
		.AngularDisplacementDeg(AngularDisplacementDeg)
		.RadiusPx(RadiusPx)
		.VertexCount(VertexCount);

	for (UPanelSlot* PanelSlot : Slots)
	{
		if (UTobiiRadialMenuSlot* TypedSlot = Cast<UTobiiRadialMenuSlot>(PanelSlot))
		{
			TypedSlot->Parent = this;
			TypedSlot->BuildSlot(MyRadialMenu.ToSharedRef());
			
			UTobiiGazeFocusableWidget* GazeFocusableWidget = Cast<UTobiiGazeFocusableWidget>(TypedSlot->Content);
			if (GazeFocusableWidget != nullptr)
			{
				STobiiGazeFocusableWidget* SlateWidget = GazeFocusableWidget->GetSlateGazeFocusableWidget();
				//SlateWidget->ParentRadialMenuSlot = TypedSlot->GetSlateSlot();
			}
		}
	}

	return MyRadialMenu.ToSharedRef();
}

UClass* UTobiiRadialMenuWidget::GetSlotClass() const
{
	return UTobiiRadialMenuSlot::StaticClass();
}

void UTobiiRadialMenuWidget::SetUseHardBorder(bool bNewUseHardBorder)
{
	bUseHardBorder = bNewUseHardBorder;
	if (MyRadialMenu.IsValid())
	{
		MyRadialMenu->bUseHardBorder = bUseHardBorder;
	}
}

void UTobiiRadialMenuWidget::SetBorderColor(FColor NewBorderColor)
{
	BorderColor = NewBorderColor;
	if (MyRadialMenu.IsValid())
	{
		MyRadialMenu->BorderColor = BorderColor;
	}
}

void UTobiiRadialMenuWidget::SetPanelColor(FColor NewPanelColor)
{
	PanelColor = NewPanelColor;
	if (MyRadialMenu.IsValid())
	{
		MyRadialMenu->PanelColor = PanelColor;
	}
}

void UTobiiRadialMenuWidget::SetBorderThicknessPx(float NewBorderThicknessPx)
{
	BorderThicknessPx = NewBorderThicknessPx;
	if (MyRadialMenu.IsValid())
	{
		MyRadialMenu->BorderThicknessPx = BorderThicknessPx;
	}
}

void UTobiiRadialMenuWidget::SetSegmentSeparationPx(float NewSegmentSeparationPx)
{
	SegmentSeparationPx = NewSegmentSeparationPx;
	if (MyRadialMenu.IsValid())
	{
		MyRadialMenu->SegmentSeparationPx = SegmentSeparationPx;
	}
}

void UTobiiRadialMenuWidget::SetAngularDisplacementDeg(float NewAngularDisplacementDeg)
{
	AngularDisplacementDeg = NewAngularDisplacementDeg;
	if (MyRadialMenu.IsValid())
	{
		MyRadialMenu->AngularDisplacementDeg = AngularDisplacementDeg;
	}
}

void UTobiiRadialMenuWidget::SetRadiusPx(float NewRadiusPx)
{
	RadiusPx = NewRadiusPx;
	if (MyRadialMenu.IsValid())
	{
		MyRadialMenu->RadiusPx = RadiusPx;
	}
}

void UTobiiRadialMenuWidget::SetVertexCount(int32 NewVertexCount)
{
	VertexCount = NewVertexCount;
	if (MyRadialMenu.IsValid())
	{
		MyRadialMenu->VertexCount = VertexCount;
	}
}

void UTobiiRadialMenuWidget::OnSlotAdded(UPanelSlot* InSlot)
{
	// Add the child to the live canvas if it already exists
	if (MyRadialMenu.IsValid())
	{
		CastChecked<UTobiiRadialMenuSlot>(InSlot)->BuildSlot(MyRadialMenu.ToSharedRef());
	}
}

void UTobiiRadialMenuWidget::OnSlotRemoved(UPanelSlot* InSlot)
{
	// Remove the widget from the live slot if it exists.
	if (MyRadialMenu.IsValid())
	{
		TSharedPtr<SWidget> Widget = InSlot->Content->GetCachedWidget();
		if (Widget.IsValid())
		{
			MyRadialMenu->RemoveSlot(Widget.ToSharedRef());
		}
	}
}

UTobiiRadialMenuSlot* UTobiiRadialMenuWidget::AddRadialMenuChild(UWidget* Content)
{
	return Cast<UTobiiRadialMenuSlot>(Super::AddChild(Content));
}

bool UTobiiRadialMenuWidget::GetGeometryForSlot(int32 SlotIndex, FGeometry& ArrangedGeometry) const
{
	UTobiiRadialMenuSlot* PanelSlot = CastChecked<UTobiiRadialMenuSlot>(Slots[SlotIndex]);
	return GetGeometryForSlot(PanelSlot, ArrangedGeometry);
}

bool UTobiiRadialMenuWidget::GetGeometryForSlot(UTobiiRadialMenuSlot* InSlot, FGeometry& ArrangedGeometry) const
{
	if (InSlot->Content == nullptr)
	{
		return false;
	}
	
	if (MyRadialMenu.IsValid())
	{
		FArrangedChildren ArrangedChildren(EVisibility::All);
		MyRadialMenu->ArrangeChildren(MyRadialMenu->GetCachedGeometry(), ArrangedChildren);

		for (int32 ChildIndex = 0; ChildIndex < ArrangedChildren.Num(); ChildIndex++)
		{
			if (ArrangedChildren[ChildIndex].Widget == InSlot->Content->GetCachedWidget())
			{
				ArrangedGeometry = ArrangedChildren[ChildIndex].Geometry;
				return true;
			}
		}
	}

	return false;
}

#if WITH_EDITOR

const FText UTobiiRadialMenuWidget::GetPaletteCategory()
{
	return LOCTEXT("Tobii", "Tobii");
}

#endif

/////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE
