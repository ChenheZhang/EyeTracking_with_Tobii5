/******************************************************************************
* Copyright 2017- Tobii Technology AB. All rights reserved.
*
* @author Temaran | Fredrik Lindh | fredrik.lindh@tobii.com | https://github.com/Temaran
******************************************************************************/

#pragma once

#include "TobiiGazeFocusableWidget.h"
#include "TobiiGTOMTypes.h"

#include "CoreMinimal.h"
#include "SceneTypes.h"
#include "Components/ActorComponent.h"

#include "TobiiGazeFocusableComponent.generated.h"

class UPrimitiveComponent;
class UTobiiGazeFocusManagerComponent;

/**
  * This component is a helper that can make it easier to setup GTOM, as well as provide an focus event interface for this object.
  * If you don't use specific tags on your primitive components, the values on this component will be used instead.
  * This component is REQUIRED for GPU based GTOM.
  * This component is also REQUIRED for world space widget focus.
  * This should be placed on the root actor for every component tree. This means that if you have an actor that has a child actor that has primitive components you would like to control, you need to add one of these components to that child actor as well.
  * Having more than one of these components on one actor is not recommended, as it is unclear which focusable component's values will be used.
  * If you want world space UI widgets to participate in GTOM, you must attach one of these components to the actor carrying the WidgetComponent(s) to affect
  */
UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class TOBIIGTOM_API UTobiiGazeFocusableComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	//The default value for focusability to be used by all primitives on the same actor as this component. It can be useful to set this to false if you only want a few primitives on the actor to be focusable.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default Focus Settings")
	bool bDefaultIsFocusable;

	//The focus priority is optional input that you can provide to GTOM. Objects with higher focus priority are more "important" and are therefore more likely to be construed as being in focus.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default Focus Settings")
	float DefaultFocusPriority;

	//If this object is at least this far away from the player, it will not participate in GTOM. This will only be in effect if it is greater than zero.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default Focus Settings")
	float DefaultMaxFocusDistance;

	//A developer can opt to only query a subset of focusables by using a focus layer. This is the default layer that will be used if none is set with tags on primitives or on a gazefocusablewidget.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default Focus Settings")
	FName DefaultFocusLayer;

public:
	//These functions will notify when gaze focus is lost or gained irregardless of focus layers.
	UPROPERTY(BlueprintAssignable, Category = "Gaze Focus")
	FPrimitiveReceivedGazeFocusSignature OnPrimitiveReceivedGazeFocus;

	UPROPERTY(BlueprintAssignable, Category = "Gaze Focus")
	FPrimitiveLostGazeFocusSignature OnPrimitiveLostGazeFocus;

	UPROPERTY(BlueprintAssignable, Category = "Gaze Focus")
	FWidgetReceivedGazeFocusSignature OnWidgetReceivedGazeFocus;

	UPROPERTY(BlueprintAssignable, Category = "Gaze Focus")
	FWidgetLostGazeFocusSignature OnWidgetLostGazeFocus;

public:
	UFUNCTION(BlueprintCallable, Category = "CleanUI")
	void RefreshOwnedWidgets();

public:
	UTobiiGazeFocusableComponent();
	virtual void PrimitiveReceivedGazeFocus(UPrimitiveComponent* FocusedComponent);
	virtual void PrimitiveLostGazeFocus(UPrimitiveComponent* FocusedComponent);
	virtual void WidgetReceivedGazeFocus(UTobiiGazeFocusableWidget* FocusedWidget);
	virtual void WidgetLostGazeFocus(UTobiiGazeFocusableWidget* FocusedWidget);

	const TMap<FTobiiFocusableUID, TWeakObjectPtr<UTobiiGazeFocusableWidget>>& GetAllFocusableWidgets() { return AllFocusableWidgets; }
	const TArray<TWeakObjectPtr<UTobiiGazeFocusableWidget>>& GetFocusableWidgetsForPrimitiveComponent(UPrimitiveComponent* PrimitiveComponent);

	/************************************************************************/
	/* UActorComponent                                                      */
	/************************************************************************/
public:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

public:
	static void ClearFocusableComponents();
	static const TMap<FTobiiFocusableUID, TWeakObjectPtr<UTobiiGazeFocusableComponent>>& GetFocusableComponents();

	static void UpdateGazeFocusPrio(FVector POVActorLocation, int32 MaxNrFocusables);
	static const TSet<FPrimitiveComponentId>* GetGazeFocusPrioSet();

	static bool IsPrimitiveFocusable(UPrimitiveComponent* Primitive);
	static bool GetMaxFocusDistanceForPrimitive(UPrimitiveComponent* Primitive, float& OutMaxDistance);
	static float GetFocusPriorityForPrimitive(UPrimitiveComponent* Primitive);
	static FName GetFocusLayerForPrimitive(UPrimitiveComponent* Primitive);

	static bool IsWidgetFocusable(UTobiiGazeFocusableWidget* GazeFocusableWidget);
	static bool GetMaxFocusDistanceForWidget(UTobiiGazeFocusableWidget* GazeFocusableWidget, float& OutMaxDistance);
	static float GetFocusPriorityForWidget(UTobiiGazeFocusableWidget* GazeFocusableWidget);
	static FName GetFocusLayerForWidget(UTobiiGazeFocusableWidget* GazeFocusableWidget);

private:
	TMap<FTobiiFocusableUID, TWeakObjectPtr<UTobiiGazeFocusableWidget>> AllFocusableWidgets;
	TMap<FEngineFocusableUID, TArray<TWeakObjectPtr<UTobiiGazeFocusableWidget>>> PrimitiveMap;
	bool bWidgetsRefreshedOnce;

	void GatherGazeFocusableWidgets(UWidget* Parent, TArray<TWeakObjectPtr<UTobiiGazeFocusableWidget>>& WidgetArray, UWidgetComponent* OptionalHostWidgetComponent);
};
