/******************************************************************************
* Copyright 2017- Tobii Technology AB. All rights reserved.
*
* @author Temaran | Fredrik Lindh | fredrik.lindh@tobii.com | https://github.com/Temaran
******************************************************************************/

#pragma once

#include "GTOMAwareUI/TobiiGazeFocusableWidget.h"

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "Components/WidgetInteractionComponent.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "TobiiGTOMBlueprintLibrary.generated.h"

/**
  * Simplified interface for blueprint use. Only exposes the features most likely to be consumed from BP.
  */
UCLASS()
class TOBIIGTOM_API UTobiiGTOMBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

public:
	/**
	  * Set which player controller should be used for GTOM.
	  */
	UFUNCTION(BlueprintCallable, Category = "Tobii GTOM")
	static void SetGTOMPlayerController(APlayerController* NewGTOMPlayerController);

	/**
	  * Get the component the focus system believes the user is looking at.
	  */
	UFUNCTION(BlueprintPure, Category = "Tobii GTOM")
	static bool GetGazeFocusData(FTobiiGazeFocusData& OutFocusData);

	/**
	  * Get a sorted list of components that the focus system believes the user is looking at.
	  */
	UFUNCTION(BlueprintPure, Category = "Tobii GTOM")
	static bool GetAllGazeFocusData(TArray<FTobiiGazeFocusData>& OutFocusData);

	/**
	  * Get the component the focus system believes the user is looking at. This variant allows you to filter out certain objects in accordance with a filter list.
	  *
	  * @param FocusLayerFilterList	These are the filters that will determine what focus data you will receive. The behavior of the filters are controlled by bIsWhiteList.
	  * @param bIsWhiteList			This controls how the filters in FocusFilterList are used. If this is true, only focusables with a layer that is in the FocusFilterList will be considered. If this is false, the FocusFilterList will act like a black list, excluding any focusables whos layer can be found in the array.
	  * @param bWantPrimitives		If this is true, focus data with valid UPrimitiveComponents will be included in the output results.
	  * @param bWantWidgets			If this is true, focus data with valid UWidgets will be included in the output results.
	  * @param OutFocusData			Output focus data.
	  */
	UFUNCTION(BlueprintPure, Category = "Tobii GTOM")
	static bool GetFilteredGazeFocusData(const TArray<FName>& FocusLayerFilterList, const bool bIsWhiteList, const bool bWantPrimitives, const bool bWantWidgets, FTobiiGazeFocusData& OutFocusData);

	/**
	  * Get a sorted list of components that the focus system believes the user is looking at. This variant allows you to filter out certain objects in accordance with a filter list.
	  *
	  * @param FocusLayerFilterList	These are the filters that will determine what focus data you will receive. The behavior of the filters are controlled by bIsWhiteList.
	  * @param bIsWhiteList			This controls how the filters in FocusFilterList are used. If this is true, only focusables with a layer that is in the FocusFilterList will be considered. If this is false, the FocusFilterList will act like a black list, excluding any focusables whos layer can be found in the array.
	  * @param bWantPrimitives		If this is true, focus data with valid UPrimitiveComponents will be included in the output results.
	  * @param bWantWidgets			If this is true, focus data with valid UWidgets will be included in the output results.
	  * @param OutFocusData			Output focus data.
	  */
	UFUNCTION(BlueprintPure, Category = "Tobii GTOM")
	static bool GetAllFilteredGazeFocusData(const TArray<FName>& FocusLayerFilterList, const bool bIsWhiteList, const bool bWantPrimitives, const bool bWantWidgets, TArray<FTobiiGazeFocusData>& OutFocusData);


	/************************************************************************/
	/* Utils                                                                */
	/************************************************************************/
	/**
 	 * Gets a simple ray hit along combined gaze
	 */
	UFUNCTION(BlueprintPure, Category = "Tobii GTOM Utils")
	static const FHitResult& GetNaiveGazeHit();

	/**
	  * Tests to see whether a primitive component is gaze focusable
	  */
	UFUNCTION(BlueprintPure, Category = "Tobii GTOM Utils")
	static bool IsPrimitiveComponentGazeFocusable(UPrimitiveComponent* PrimitiveComponent);

	/**
	  * Tests to see whether a widget is gaze focusable
	  */
	UFUNCTION(BlueprintPure, Category = "Tobii GTOM Utils")
	static bool IsWidgetGazeFocusable(UWidget* Widget);

	/**
	  * Gets the *untransformed* focus offset for a primitive component.
	  */
	UFUNCTION(BlueprintPure, Category = "Tobii GTOM Utils")
	static bool GetPrimitiveComponentFocusOffset(USceneComponent* Component, FVector& OutFocusOffset);

	/**
	  * Gets the focus point on the primitive.
	  */
	UFUNCTION(BlueprintPure, Category = "Tobii GTOM Utils")
	static bool GetPrimitiveComponentFocusLocation(USceneComponent* Component, FVector& OutFocusLocation);

	/* 
	 * This function will traverse a widget tree given a root and add any UTobiiGazeFocusableWidget to the screen space widget collection.
	 */
	UFUNCTION(BlueprintCallable, Category = "Tobii GTOM Utils")
	static void RegisterScreenSpaceGazeFocusableWidgets(UWidget* Root);

	/**
	  * Helper function to simulate gaze focus using a widget interaction component
	  */
	UFUNCTION(BlueprintCallable, Category = "Tobii GTOM Utils")
	static bool MakeGazeFocusDataForWidgetInteractionComponent(UWidgetInteractionComponent* WidgetInteraction, ECollisionChannel CollisionChannel, FTobiiGazeFocusData& OutGazeFocusData);

	/**
	  * Helper function to simulate gaze focus using a widget interaction component
	  */
	UFUNCTION(BlueprintCallable, Category = "Tobii GTOM Utils")
	static void EmulateGazeFocusUsingWidgetInteractionComponent(UWidgetInteractionComponent* WidgetInteraction, ECollisionChannel CollisionChannel);

	/**
	  * This function will find the largest possible rectangle that can be inscribed into a given circle.
	  */
	UFUNCTION(BlueprintCallable, Category = "Tobii Math Utils Utils")
	static bool FindLargestInscribedAlignedRect(float CircleSegmentAngleRad, float CircleRadius, FVector2D& LargestInscribedRectSize, float& DistanceToCenter);

	/**
      * This function will project a local point in a widget hosted by a widget component into world space
      */
	UFUNCTION(BlueprintCallable, Category = "Tobii Math Utils")
	static bool TransformWidgetLocalPointToWorld(UWidgetComponent* Component, const FVector2D& LocalWidgetLocation, FVector& OutWorldLocation);

	/**
	  * This function will project a world point to the widget's local space
	  */
	UFUNCTION(BlueprintCallable, Category = "Tobii Math Utils")
	static bool TransformWorldPointToWidgetLocal(UWidgetComponent* Component, const FVector& WorldLocation, const FVector& WorldDirection, FVector2D& OutLocalWidgetLocation);

	/**
	  * This function will test whether a right angle cone and a sphere intersects. Very useful for world space eye tracking.
	  */
	UFUNCTION(BlueprintCallable, Category = "Tobii Math Utils")
	static bool TestConeSphereIntersection(const FVector& ConeApex, const FVector& ConeDirection, const float ConeAngleDeg, const FVector& SphereCenter, const float SphereRadius);

	/**
	  * This function will test whether a rectangle and a rotated ellipse intersects. Very useful for screen space eye tracking.
	  */
	UFUNCTION(BlueprintCallable, Category = "Tobii Math Utils")
	static bool TestRectEllipseIntersection(const FVector2D& RectangleCenter, const FVector2D& RectangleRightAxis, const FVector2D& RectangleUpAxis, const FVector2D& RectangleExtents, const FVector2D& EllipseCenter, const FVector2D& EllipseRadii, const float& EllipseRotationDeg);

	/**
	  * Get a random point on a circle centered at the origin that is uniformly distributed.
	  */
	UFUNCTION(BlueprintPure, Category = "Tobii Math Utils")
	static FVector2D GetUniformlyDistributedRandomCirclePoint(float CircleRadius);
};
