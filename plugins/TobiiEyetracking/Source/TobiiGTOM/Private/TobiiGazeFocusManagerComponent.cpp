/******************************************************************************
* Copyright 2017- Tobii Technology AB. All rights reserved.
*
* @author Temaran | Fredrik Lindh | fredrik.lindh@tobii.com | https://github.com/Temaran
******************************************************************************/

#include "TobiiGazeFocusManagerComponent.h"
#include "TobiiGazeFocusableComponent.h"
#include "TobiiGTOMModule.h"
#include "TobiiGTOMBlueprintLibrary.h"

#include "DrawDebugHelpers.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/Engine.h"
#include "HAL/IConsoleManager.h"


UTobiiGazeFocusManagerComponent::UTobiiGazeFocusManagerComponent()
	: bWantPrimitives(true)
	, bWantWidgets(true)
	, bIsWhiteList(false)
	, FocusLayerFilters()
	, PreviouslyFocusedPrimitiveComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	ResetFocusData();
}

void UTobiiGazeFocusManagerComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	ResetFocusData();

	UTobiiGTOMBlueprintLibrary::GetAllFilteredGazeFocusData(FocusLayerFilters, bIsWhiteList, bWantPrimitives, bWantWidgets, AllFocusData);
	for (const FTobiiGazeFocusData& FocusData : AllFocusData)
	{
		if (bWantPrimitives 
			&& !BestPrimitiveComponentFocusData.FocusedPrimitiveComponent.IsValid()
			&& (FocusData.FocusedPrimitiveComponent.IsValid() && !FocusData.FocusedWidget.IsValid()))
		{
			BestPrimitiveComponentFocusData = FocusData;
		}

		if (bWantWidgets 
			&& !BestWidgetFocusData.FocusedWidget.IsValid()
			&& FocusData.FocusedWidget.IsValid())
		{
			BestWidgetFocusData = FocusData;
		}

		if((!bWantPrimitives || (BestPrimitiveComponentFocusData.FocusedPrimitiveComponent.IsValid() && !BestPrimitiveComponentFocusData.FocusedWidget.IsValid()))
			&& (!bWantWidgets || BestWidgetFocusData.FocusedWidget.IsValid()))
		{
			//We found the best candidates we were after
			break;
		}
	}
	
	//Primitive components
	if(bWantPrimitives)
	{
		TWeakObjectPtr<UPrimitiveComponent> NewTopFocusPrimitive = BestPrimitiveComponentFocusData.FocusedPrimitiveComponent;

		if (!NewTopFocusPrimitive.IsValid() && PreviouslyFocusedPrimitiveComponent.IsValid())
		{
			//We lost focus completely
			OnPrimitiveLostGazeFocus.Broadcast(PreviouslyFocusedPrimitiveComponent.Get());
		}
		else if (NewTopFocusPrimitive.IsValid() && !PreviouslyFocusedPrimitiveComponent.IsValid())
		{
			//We gained focus from no focus
			OnPrimitiveReceivedGazeFocus.Broadcast(NewTopFocusPrimitive.Get());
		}
		else if (NewTopFocusPrimitive.IsValid() && PreviouslyFocusedPrimitiveComponent.IsValid() && NewTopFocusPrimitive.Get() != PreviouslyFocusedPrimitiveComponent.Get())
		{
			//Focus shifted from one object to another
			OnPrimitiveLostGazeFocus.Broadcast(PreviouslyFocusedPrimitiveComponent.Get());
			OnPrimitiveReceivedGazeFocus.Broadcast(NewTopFocusPrimitive.Get());
		}

		PreviouslyFocusedPrimitiveComponent = NewTopFocusPrimitive;
	}

	//Widgets
	if(bWantWidgets)
	{
		TWeakObjectPtr<UTobiiGazeFocusableWidget> NewTopFocusWidget = BestPrimitiveComponentFocusData.FocusedWidget;

		if (!NewTopFocusWidget.IsValid() && PreviouslyFocusedWidget.IsValid())
		{
			//We lost focus completely
			OnWidgetLostGazeFocus.Broadcast(PreviouslyFocusedWidget.Get());
		}
		else if (NewTopFocusWidget.IsValid() && !PreviouslyFocusedWidget.IsValid())
		{
			//We gained focus from no focus
			OnWidgetReceivedGazeFocus.Broadcast(NewTopFocusWidget.Get());
		}
		else if (NewTopFocusWidget.IsValid() && PreviouslyFocusedWidget.IsValid() && NewTopFocusWidget.Get() != PreviouslyFocusedWidget.Get())
		{
			//Focus shifted from one object to another
			OnWidgetLostGazeFocus.Broadcast(PreviouslyFocusedWidget.Get());
			OnWidgetReceivedGazeFocus.Broadcast(NewTopFocusWidget.Get());
		}

		PreviouslyFocusedWidget = NewTopFocusWidget;
	}
}

void UTobiiGazeFocusManagerComponent::ResetFocusData()
{
	AllFocusData.Empty();

	BestPrimitiveComponentFocusData.FocusedPrimitiveComponent.Reset();
	BestPrimitiveComponentFocusData.FocusedActor.Reset();
	BestPrimitiveComponentFocusData.FocusedWidget.Reset();

	BestWidgetFocusData.FocusedPrimitiveComponent.Reset();
	BestWidgetFocusData.FocusedActor.Reset();
	BestWidgetFocusData.FocusedWidget.Reset();
}

void UTobiiGazeFocusManagerComponent::GetBestFilteredPrimitiveComponentFocusData(FTobiiGazeFocusData& OutFocusData) const
{
	OutFocusData = BestPrimitiveComponentFocusData;
}

void UTobiiGazeFocusManagerComponent::GetBestFilteredWidgetComponentFocusData(FTobiiGazeFocusData& OutFocusData) const
{
	OutFocusData = BestWidgetFocusData;
}

void UTobiiGazeFocusManagerComponent::GetAllFilteredFocusData(TArray<FTobiiGazeFocusData>& OutFocusData) const
{
	OutFocusData = AllFocusData;
}
