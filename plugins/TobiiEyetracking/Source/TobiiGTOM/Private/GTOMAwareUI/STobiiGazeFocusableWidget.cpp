/******************************************************************************
* Copyright 2017- Tobii Technology AB. All rights reserved.
*
* @author Temaran | Fredrik Lindh | fredrik.lindh@tobii.com | https://github.com/Temaran
******************************************************************************/

#include "STobiiGazeFocusableWidget.h"
#include "TobiiGTOMBlueprintLibrary.h"

#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Components/WidgetComponent.h"
#include "HAL/IConsoleManager.h"
#include "Slate/SceneViewport.h"


static TAutoConsoleVariable<int32> CVarEnableCleanUI(TEXT("tobii.EnableCleanUI"), 1, TEXT("0 - CleanUI is disabled. 1 - CleanUI is enabled."));
static TAutoConsoleVariable<float> CVarCleanUIFadeInTimeSecs(TEXT("tobii.CleanUIFadeInTimeSecs"), 0.0f, TEXT("We want the fade in time to be fairly fast so that the information is instantly available to the user."));
static TAutoConsoleVariable<float> CVarCleanUIFadeOutTimeSecs(TEXT("tobii.CleanUIFadeOutTimeSecs"), 1.8f, TEXT("The fade out time should be fairly slow however to make sure that it doesn't draw peripheral vision attention."));
static TAutoConsoleVariable<float> CVarCleanUIMinAlpha(TEXT("tobii.CleanUIMinAlpha"), 0.3f, TEXT("The clean UI won't fade something out beyond this minimum alpha."));
static TAutoConsoleVariable<float> CVarCleanUIMaxAlpha(TEXT("tobii.CleanUIMaxAlpha"), 1.0f, TEXT("Change this if you don't want the clean UI to increase alpha beyond this point"));

STobiiGazeFocusableWidget::STobiiGazeFocusableWidget()
	: CleanUIAlpha(1.0f)
	, CleanUIContainersToPollHitsFrom()
{
	bCanSupportFocus = false;

	SetCanTick(true);
}

void STobiiGazeFocusableWidget::Construct(const FArguments& InArgs)
{
	SBox::FArguments ParentArgs;
	ParentArgs._HAlign = InArgs._HAlign;
	ParentArgs._VAlign = InArgs._VAlign;
	ParentArgs._Padding = InArgs._Padding;
	ParentArgs._Content = InArgs._Content;
	SBox::Construct(ParentArgs);

	OnHovered = InArgs._OnHovered;
	OnUnhovered = InArgs._OnUnhovered;
}

bool STobiiGazeFocusableWidget::HitByGaze()
{ 
	return UMGWidget.IsValid() && (UMGWidget->CleanUIMode == ETobiiCleanUIMode::FocusExclusive ? 
		UMGWidget->HasFocus() : UMGWidget->IsInFocusCollection());
}

void STobiiGazeFocusableWidget::AddExtraCleanUIContainerToPollHitsFrom(STobiiGazeFocusableWidget* CleanUIContainerToPoll, bool PollGazeHits /*= true*/, bool PollMouseHits /*= true*/)
{
	if (CleanUIContainerToPoll != nullptr)
	{
		CleanUIPollingInfo PollingInfo;
		PollingInfo.CleanUIToPollFrom = TWeakPtr<SWidget>(CleanUIContainerToPoll->AsShared());
		PollingInfo.PollGaze = PollGazeHits;
		PollingInfo.PollPointer = PollMouseHits;
		CleanUIContainersToPollHitsFrom.Add(PollingInfo);
	}
}

void STobiiGazeFocusableWidget::RemoveExtraCleanUIContainerToPollHitsFrom(STobiiGazeFocusableWidget* CleanUIContainerToStopPolling)
{
	if (CleanUIContainerToStopPolling != nullptr)
	{
		CleanUIContainersToPollHitsFrom.RemoveAll([=](CleanUIPollingInfo& CurCleanUIPollingInfo)
		{
			return !CurCleanUIPollingInfo.CleanUIToPollFrom.IsValid() || CurCleanUIPollingInfo.CleanUIToPollFrom.HasSameObject(CleanUIContainerToStopPolling);
		});
	}
}

void STobiiGazeFocusableWidget::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	if (!UMGWidget.IsValid()
		|| UMGWidget->CleanUIMode == ETobiiCleanUIMode::Disabled
		|| GEngine == nullptr
		|| GEngine->GameViewport == nullptr 
		|| GEngine->GameViewport->GetGameViewport() == nullptr
		|| GetChildren()->Num() == 0)
	{
		CleanUIAlpha = 1.0f;
		return;
	}
	
	bool bTriggerCleanUIOnMouseOver = UMGWidget.IsValid() && UMGWidget->bTriggerCleanUIOnMouseOver;
	bool bHitByGaze = HitByGaze();
	bool bShouldFadeIn = bHitByGaze || (bTriggerCleanUIOnMouseOver && bIsHovered);

	// Check if we have any hits on our CleanUIContainersToPollHitsFrom to see if we should still fade in even though we still think we should fade out
	if (!bShouldFadeIn)
	{
		for (CleanUIPollingInfo& PollingInfo : CleanUIContainersToPollHitsFrom)
		{
			if (PollingInfo.CleanUIToPollFrom.IsValid())
			{
				TSharedPtr<SWidget> CleanUIWidget = PollingInfo.CleanUIToPollFrom.Pin();
				STobiiGazeFocusableWidget* CleanUIToPollFrom = (STobiiGazeFocusableWidget*)CleanUIWidget.Get();

				if ((PollingInfo.PollGaze && CleanUIToPollFrom->HitByGaze())
					|| PollingInfo.PollPointer && bTriggerCleanUIOnMouseOver && CleanUIToPollFrom->bIsHovered)
				{
					bShouldFadeIn = true;
					break;
				}
			}
		}
	}

	UMGWidget->TimeCleanUIIsSuppressedForSecs = FMath::Max(UMGWidget->TimeCleanUIIsSuppressedForSecs -= InDeltaTime, 0.0f);
	bool bCleanUISuppressed = UMGWidget->TimeCleanUIIsSuppressedForSecs > FLT_EPSILON;
	bShouldFadeIn = bShouldFadeIn || bCleanUISuppressed;

	float MinAlpha = UMGWidget->CleanUIMinAlphaOverride >= 0.0f ? UMGWidget->CleanUIMinAlphaOverride : CVarCleanUIMinAlpha.GetValueOnAnyThread();
	float MaxAlpha = UMGWidget->CleanUIMaxAlphaOverride >= 0.0f ? UMGWidget->CleanUIMaxAlphaOverride : CVarCleanUIMaxAlpha.GetValueOnAnyThread();
	if (bShouldFadeIn)
	{
		float FadeInTime = UMGWidget->CleanUIFadeInTimeSecsOverride >= 0.0f ? UMGWidget->CleanUIFadeInTimeSecsOverride : CVarCleanUIFadeInTimeSecs.GetValueOnAnyThread();
		CleanUIAlpha = FadeInTime > 0.0f ? FMath::Clamp(CleanUIAlpha + InDeltaTime / FadeInTime, MinAlpha, MaxAlpha) : MaxAlpha;
	}
	else
	{
		float FadeOutTime = UMGWidget->CleanUIFadeOutTimeSecsOverride >= 0.0f ? UMGWidget->CleanUIFadeOutTimeSecsOverride : CVarCleanUIFadeOutTimeSecs.GetValueOnAnyThread();
		CleanUIAlpha = FadeOutTime > 0.0f ? FMath::Clamp(CleanUIAlpha - InDeltaTime / FadeOutTime, MinAlpha, MaxAlpha) : MinAlpha;
	}
}

int32 STobiiGazeFocusableWidget::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	const float Alpha = UMGWidget.IsValid() ? (UMGWidget->CleanUIMode == ETobiiCleanUIMode::Silent ? 1.0f : CleanUIAlpha) : 1.0f;
	FWidgetStyle CompoundedWidgetStyle = FWidgetStyle(InWidgetStyle).BlendColorAndOpacityTint(FLinearColor(1.0f, 1.0f, 1.0f, Alpha));
	return SBox::OnPaint(Args, AllottedGeometry, MyClippingRect, OutDrawElements, LayerId, CompoundedWidgetStyle, bParentEnabled);
}

void STobiiGazeFocusableWidget::OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	SWidget::OnMouseEnter(MyGeometry, MouseEvent);
	OnHovered.ExecuteIfBound();
	Invalidate(EInvalidateWidget::Layout);
}

void STobiiGazeFocusableWidget::OnMouseLeave(const FPointerEvent& MouseEvent)
{
	// Call parent implementation
	SWidget::OnMouseLeave(MouseEvent);
	OnUnhovered.ExecuteIfBound();
	Invalidate(EInvalidateWidget::Layout);
}

STobiiGazeFocusableWidget* STobiiGazeFocusableWidget::TryCastToTobiiGazeFocusable(SWidget* Widget)
{
 	if (Widget != nullptr && Widget->GetType() == FName("STobiiGazeFocusableWidget"))
 	{
 		return (STobiiGazeFocusableWidget*)Widget;
 	}

	return nullptr;
}