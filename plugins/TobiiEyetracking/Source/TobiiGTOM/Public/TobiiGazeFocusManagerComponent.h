/******************************************************************************
* Copyright 2017- Tobii Technology AB. All rights reserved.
*
* @author Temaran | Fredrik Lindh | fredrik.lindh@tobii.com | https://github.com/Temaran
******************************************************************************/

#pragma once

#include "TobiiGTOMTypes.h"

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Delegates/DelegateCombinations.h"

#include "TobiiGazeFocusManagerComponent.generated.h"

/*
 * Tobii focus managers are an easier way to set up focus layer filters and access the focusables associated with them.
 * It also offers a way to get notified of changes in gaze focus via events.
 */
UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class TOBIIGTOM_API UTobiiGazeFocusManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTobiiGazeFocusManagerComponent();

public:
	//If this is true, focus data with valid UPrimitiveComponents will be included in the output results.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gaze Focus Management")
	bool bWantPrimitives;

	//If this is true, focus data with valid UWidgets will be included in the output results
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gaze Focus Management")
	bool bWantWidgets;

	//This controls how the filters in FocusFilterList are used. If this is true, only focusables with a layer that is in the FocusFilterList will be considered. If this is false, the FocusFilterList will act like a black list, excluding any focusables whos layer can be found in the array.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gaze Focus Management")
	bool bIsWhiteList;

	//These are the filters that will determine what focus data you will receive. The behavior of the filters are controlled by bIsWhiteList.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gaze Focus Management")
	TArray<FName> FocusLayerFilters;

public:
	//These functions will notify when gaze focus is lost for this focus manager's filtered set.
	UPROPERTY(BlueprintAssignable, Category = "Gaze Focus Management")
	FPrimitiveReceivedGazeFocusSignature OnPrimitiveReceivedGazeFocus;

	UPROPERTY(BlueprintAssignable, Category = "Gaze Focus Management")
	FPrimitiveLostGazeFocusSignature OnPrimitiveLostGazeFocus;

	UPROPERTY(BlueprintAssignable, Category = "Gaze Focus Management")
	FWidgetReceivedGazeFocusSignature OnWidgetReceivedGazeFocus;

	UPROPERTY(BlueprintAssignable, Category = "Gaze Focus Management")
	FWidgetLostGazeFocusSignature OnWidgetLostGazeFocus;

public:
	//Get the component the focus system believes the user is looking at, given the filters the focus manager is configured with.
	UFUNCTION(BlueprintPure, Category = "Gaze Focus Management")
	void GetBestFilteredPrimitiveComponentFocusData(FTobiiGazeFocusData& OutFocusData) const;

	UFUNCTION(BlueprintPure, Category = "Gaze Focus Management")
	void GetBestFilteredWidgetComponentFocusData(FTobiiGazeFocusData& OutFocusData) const;

	//Get a sorted list of components that the focus system believes the user is looking at, given the filters the focus manager is configured with.
	UFUNCTION(BlueprintPure, Category = "Gaze Focus Management")
	void GetAllFilteredFocusData(TArray<FTobiiGazeFocusData>& OutFocusData) const;

	/************************************************************************/
	/* UActorComponent                                                      */
	/************************************************************************/
public:
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

private:
	TWeakObjectPtr<UPrimitiveComponent> PreviouslyFocusedPrimitiveComponent;
	TWeakObjectPtr<UTobiiGazeFocusableWidget> PreviouslyFocusedWidget;
	FTobiiGazeFocusData BestPrimitiveComponentFocusData;
	FTobiiGazeFocusData BestWidgetFocusData;
	TArray<FTobiiGazeFocusData> AllFocusData;

	void ResetFocusData();
	void NotifyPrimitiveComponentGazeFocusReceived(UPrimitiveComponent& PrimitiveComponentToReceiveFocus);
	void NotifyPrimitiveComponentGazeFocusLost(UPrimitiveComponent& PrimitiveComponentToLoseFocus);
};
