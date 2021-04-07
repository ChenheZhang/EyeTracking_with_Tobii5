/******************************************************************************
* Copyright 2017- Tobii Technology AB. All rights reserved.
*
* @author Temaran | Fredrik Lindh | fredrik.lindh@tobii.com | https://github.com/Temaran
******************************************************************************/

#include "TobiiGazeFocusableWidget.h"
#include "TobiiGTOMBlueprintLibrary.h"

#include "Components/SizeBoxSlot.h"

#define LOCTEXT_NAMESPACE "UMG"

static TMap<FTobiiFocusableUID, TWeakObjectPtr<UTobiiGazeFocusableWidget>> GRegisteredTobiiScreenSpaceFocusableWidgets;

UTobiiGazeFocusableWidget::UTobiiGazeFocusableWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)

	, OnHovered()
	, OnUnhovered()

	, bHasGazeFocus(false)
	, bIsGazeFocusable(true)
	, GazeFocusPriority(0.0f)
	, MaxFocusDistance(-1.0f)
	, FocusLayer("")
	, ReceivedGazeFocus()
	, LostGazeFocus()

	, CleanUIMode(ETobiiCleanUIMode::Disabled)
	, TimeCleanUIIsSuppressedForSecs(0.0f)
	, CleanUIFadeInTimeSecsOverride(-1.0f)
	, CleanUIFadeOutTimeSecsOverride(-1.0f)
	, CleanUIMinAlphaOverride(-1.0f)
	, CleanUIMaxAlphaOverride(-1.0f)
	, bTriggerCleanUIOnMouseOver(false)

	, WorldSpaceHostWidgetComponent()
	, MySlateWidget()
{
	Visibility = ESlateVisibility::Visible;
	bIsVariable = true;
}

void UTobiiGazeFocusableWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	MySlateWidget.Reset();
}

TSharedRef<SWidget> UTobiiGazeFocusableWidget::RebuildWidget()
{
	MySizeBox = MySlateWidget = SNew(STobiiGazeFocusableWidget)
		.OnHovered_UObject(this, &ThisClass::SlateHandleHovered)
		.OnUnhovered_UObject(this, &ThisClass::SlateHandleUnhovered);

	MySlateWidget->UMGWidget = this;

	if (GetChildrenCount() > 0)
	{
		Cast<USizeBoxSlot>(GetContentSlot())->BuildSlot(MySizeBox.ToSharedRef());
	}
	 
	return MySizeBox.ToSharedRef();
}

bool UTobiiGazeFocusableWidget::IsHoveredByPointer()
{
	if (MySlateWidget.IsValid())
	{
		return MySlateWidget->IsHoveredByPointer();
	}

	return false;
}

void UTobiiGazeFocusableWidget::RegisterWidgetToGTOM(UWidgetComponent* HostWidget)
{
	if (HostWidget != nullptr)
	{
		WorldSpaceHostWidgetComponent = HostWidget;
	}
	else
	{
		uint32 Uid = GetUniqueID();
		if (GRegisteredTobiiScreenSpaceFocusableWidgets.Contains(Uid))
		{
			//Overwrite
			GRegisteredTobiiScreenSpaceFocusableWidgets[Uid] = this;
		}
		else
		{
			GRegisteredTobiiScreenSpaceFocusableWidgets.Add(Uid, this);
		}
	}
}

bool UTobiiGazeFocusableWidget::IsWorldSpaceWidget()
{
	return WorldSpaceHostWidgetComponent.IsValid();
}

UWidgetComponent* UTobiiGazeFocusableWidget::GetWorldSpaceHostWidgetComponent()
{
	return WorldSpaceHostWidgetComponent.IsValid() ? WorldSpaceHostWidgetComponent.Get() : nullptr;
}

bool UTobiiGazeFocusableWidget::HasFocus()
{
	FTobiiGazeFocusData FocusData;
	return UTobiiGTOMBlueprintLibrary::GetGazeFocusData(FocusData) && FocusData.FocusedWidget == this;
}

bool UTobiiGazeFocusableWidget::IsInFocusCollection()
{
	TArray<FTobiiGazeFocusData> AllFocusData;
	if (UTobiiGTOMBlueprintLibrary::GetAllGazeFocusData(AllFocusData))
	{
		for (const FTobiiGazeFocusData& FocusData : AllFocusData)
		{
			if (FocusData.FocusedWidget == this)
			{
				return true;
			}
		}
	}

	return false;
}

float UTobiiGazeFocusableWidget::GetCleanUIAlpha()
{
	if (MySlateWidget.IsValid())
	{
		return MySlateWidget->GetCleanUIAlpha();
	}

	return 1.0f;
}

void UTobiiGazeFocusableWidget::AddGazeFocusableWidgetToPollHitsFrom(UTobiiGazeFocusableWidget* GazeFocusableWidgetToPoll, bool PollGazeHits /*= true*/, bool PollMouseHits /*= true*/)
{
	if (GazeFocusableWidgetToPoll != nullptr)
	{
		STobiiGazeFocusableWidget* SlateCleanUIContainerToPoll = GazeFocusableWidgetToPoll->GetSlateGazeFocusableWidget();
		if (SlateCleanUIContainerToPoll != nullptr && MySlateWidget.IsValid())
		{
			MySlateWidget->AddExtraCleanUIContainerToPollHitsFrom(SlateCleanUIContainerToPoll, PollGazeHits, PollMouseHits);
		}
	}
}

void UTobiiGazeFocusableWidget::RemoveGazeFocusableWidgetToPollHitsFrom(UTobiiGazeFocusableWidget* GazeFocusableWidgetToStopPolling)
{
	if (GazeFocusableWidgetToStopPolling != nullptr)
	{
		STobiiGazeFocusableWidget* SlateCleanUIContainerToStopPolling = GazeFocusableWidgetToStopPolling->GetSlateGazeFocusableWidget();
		if (SlateCleanUIContainerToStopPolling != nullptr && MySlateWidget.IsValid())
		{
			MySlateWidget->RemoveExtraCleanUIContainerToPollHitsFrom(SlateCleanUIContainerToStopPolling);
		}		
	}
}

void UTobiiGazeFocusableWidget::ReceiveFocus()
{
	ReceivedGazeFocus.Broadcast(this);
}

void UTobiiGazeFocusableWidget::LoseFocus()
{
	LostGazeFocus.Broadcast(this);
}

void UTobiiGazeFocusableWidget::SlateHandleHovered()
{
	OnHovered.Broadcast(this);
}

void UTobiiGazeFocusableWidget::SlateHandleUnhovered()
{
	OnUnhovered.Broadcast(this);
}

STobiiGazeFocusableWidget* UTobiiGazeFocusableWidget::GetSlateGazeFocusableWidget()
{
	if (MySlateWidget.IsValid())
	{
		return MySlateWidget.Get();
	}

	return nullptr;
}

TMap<FTobiiFocusableUID, TWeakObjectPtr<UTobiiGazeFocusableWidget>>& UTobiiGazeFocusableWidget::GetTobiiScreenSpaceFocusableWidgets()
{
	TArray<FTobiiFocusableUID> IdsToPrune;
	for (auto& WidgetPair : GRegisteredTobiiScreenSpaceFocusableWidgets)
	{
		if (!WidgetPair.Value.IsValid())
		{
			IdsToPrune.Add(WidgetPair.Key);
		}
	}
	for (FTobiiFocusableUID IdToPrune : IdsToPrune)
	{
		GRegisteredTobiiScreenSpaceFocusableWidgets.Remove(IdToPrune);
	}

	return GRegisteredTobiiScreenSpaceFocusableWidgets;
}

#if WITH_EDITOR
const FText UTobiiGazeFocusableWidget::GetPaletteCategory()
{
	return LOCTEXT("Tobii", "Tobii");
}
#endif

///////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE
