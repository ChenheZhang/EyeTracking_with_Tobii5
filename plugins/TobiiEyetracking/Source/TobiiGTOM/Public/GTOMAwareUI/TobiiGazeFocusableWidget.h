/******************************************************************************
* Copyright 2017- Tobii Technology AB. All rights reserved.
*
* @author Temaran | Fredrik Lindh | fredrik.lindh@tobii.com | https://github.com/Temaran
******************************************************************************/

#pragma once

#include "TobiiGTOMTypes.h"
#include "STobiiGazeFocusableWidget.h"

#include "CoreMinimal.h"
#include "ObjectEditorUtils.h"
#include "Components/WidgetComponent.h"
#include "Components/SizeBox.h"

#include "TobiiGazeFocusableWidget.generated.h"

class UPrimitiveComponent;
class UTobiiGazeFocusManagerComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWidgetGazeFocusSignature, UTobiiGazeFocusableWidget*, FocusedWidget);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWidgetHoverSignature, UTobiiGazeFocusableWidget*, FocusedWidget);

/**
  * Use this widget to wrap other widgets you want to mark as gaze focusable.
  * In contrast to gaze focusable components, each widget you want to make focusable needs to be wrapped by its own UTobiiGazeFocusableWidget. There is more info below on why this is the case.
  * Please note that while this wraps a raw slate widget, we currently do not officially support using the slate widgets directly. Please use the UMG wrapper.
  * 
  * There are some major differences to how gaze focusable widgets versus components work:
  * 1. Since widgets don't support tags, widgets can only participate in GTOM by being wrapped by this widget.
  * 2. Actor component hierarchies can be very rigid since they can be partially constructed in cpp, both by the developer and the engine. This means that designating gaze focusability via component parenting wouldn't be reliable.
  * 3. It is very common to want to make it so that every primitive component on an actor should contribute to focusing that actor. This is not true for widgets, here it is more common that each individual widget wants to know if it has been individually focused.
  *
  * All of these things should help to illustrate why using different models for gaze focus in slate/UMG and actors is to be preferred.
  */
UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent, DisplayName= "Tobii Gaze Focusable"))
class TOBIIGTOM_API UTobiiGazeFocusableWidget : public USizeBox
{
	GENERATED_UCLASS_BODY()

	/************************************************************************/
	/* USizeBox                                                             */
	/************************************************************************/
public:
	// UVisual interface
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
protected:
	// UWidget interface
	virtual TSharedRef<SWidget> RebuildWidget() override;

public:
	//These are for pointer hovering
	UPROPERTY(BlueprintAssignable, Category = "Event")
	FWidgetHoverSignature OnHovered;

	UPROPERTY(BlueprintAssignable, Category = "Event")
	FWidgetHoverSignature OnUnhovered;

	//This function must be called to make the widget participate in GTOM.
	//WARNING: For screen space widgets, you must do this manually by sending in a nullptr HostWidget! For worldspace widgets, the UTobiiGazeFocusableComponent will do this for you though.
	UFUNCTION(BlueprintCallable, Category = "Gaze Focus")
	void RegisterWidgetToGTOM(UWidgetComponent* HostWidget);

	UFUNCTION(BlueprintPure, Category = "General")
	UWidgetComponent* GetWorldSpaceHostWidgetComponent();

	UFUNCTION(BlueprintPure, Category = "General")
	bool IsHoveredByPointer();

	UFUNCTION(BlueprintPure, Category = "General")
	bool IsWorldSpaceWidget();

	/************************************************************************/
	/* Tobii                                                                */
	/************************************************************************/
public:
	//This will be true if this is the most likely widget to currently have focus irregardless of layer filters.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gaze Focus")
	bool bHasGazeFocus;

	//Set this to false if you want to stop this widget participating in GTOM.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gaze Focus")
	bool bIsGazeFocusable;

	//The gaze focus priority is optional input that you can provide to GTOM. Objects with higher focus priority are more "important" and are therefore more likely to be construed as being in focus. If this is negative, it will not be enforced.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gaze Focus")
	float GazeFocusPriority;

	//If this object is at least this far away from the player, it will not participate in GTOM. If this is less or equal to zero, it will not be enforced.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gaze Focus")
	float MaxFocusDistance;

	//A focus manager can opt to only query a subset of focusables by using a focus layer. If this is empty, the default layer will be used (Either from a gaze focusable component, or the global default).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gaze Focus")
	FString FocusLayer;

	//This delegate will fire when this widget receives user focus
	UPROPERTY(BlueprintAssignable, Category = "Gaze Focus|Event")
	FWidgetGazeFocusSignature ReceivedGazeFocus;

	//This delegate will fire when this widget loses user focus
	UPROPERTY(BlueprintAssignable, Category = "Gaze Focus|Event")
	FWidgetGazeFocusSignature LostGazeFocus;

	//This will return true if this widget has user focus
	UFUNCTION(BlueprintPure, Category = "Gaze Focus")
	bool HasFocus();

	//This will return true if this widget is close enough to the user's gaze to be considered in focus, even though it might not be the object the user is actively focusing on.
	UFUNCTION(BlueprintPure, Category = "Gaze Focus")
	bool IsInFocusCollection();

public:
	//How should we apply CleanUI?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CleanUI")
	ETobiiCleanUIMode CleanUIMode;

	//If this is larger than 0, CleanUI is currently being suppressed.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CleanUI")
	float TimeCleanUIIsSuppressedForSecs;

	//If you want this cleanUI panel to behave differently from the default governed by CVars you can override properties here.	Any negative value will result in the CVar value being used.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CleanUI")
	float CleanUIFadeInTimeSecsOverride;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CleanUI")
	float CleanUIFadeOutTimeSecsOverride;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CleanUI")
	float CleanUIMinAlphaOverride;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CleanUI")
	float CleanUIMaxAlphaOverride;

	//If you have a game where you want this cleanUI widget to fade in on mouse over, set this to true.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CleanUI")
	bool bTriggerCleanUIOnMouseOver;

	UFUNCTION(BlueprintPure, Category = "CleanUI")
	float GetCleanUIAlpha();

public:
	//If this widget should fade out when another container is being looked at, use these functions to make that happen. This requires bUseFastHitTesting to be off.
	UFUNCTION(BlueprintCallable, Category = "Dependent Widgets")
	void AddGazeFocusableWidgetToPollHitsFrom(UTobiiGazeFocusableWidget* GazeFocusableWidgetToPoll, bool PollGazeHits = true, bool PollMouseHits = true);
	UFUNCTION(BlueprintCallable, Category = "Dependent Widgets")
	void RemoveGazeFocusableWidgetToPollHitsFrom(UTobiiGazeFocusableWidget* GazeFocusableWidgetToStopPolling);

public:
	STobiiGazeFocusableWidget* GetSlateGazeFocusableWidget();	
	
	virtual void ReceiveFocus();
	virtual void LoseFocus();

	static TMap<FTobiiFocusableUID, TWeakObjectPtr<UTobiiGazeFocusableWidget>>& GetTobiiScreenSpaceFocusableWidgets();

public:
#if WITH_EDITOR
	virtual const FText GetPaletteCategory() override;
#endif

protected:
	//This is set during run time by a UTobiiGazeFocusableComponent if this is a world space widget.
	TWeakObjectPtr<UWidgetComponent> WorldSpaceHostWidgetComponent;
	TSharedPtr<class STobiiGazeFocusableWidget> MySlateWidget;

	void SlateHandleHovered();
	void SlateHandleUnhovered();
};