/******************************************************************************
* Copyright 2017- Tobii Technology AB. All rights reserved.
*
* @author Temaran | Fredrik Lindh | fredrik.lindh@tobii.com | https://github.com/Temaran
******************************************************************************/

#pragma once

#include "CoreMinimal.h"
#include "TobiiTypes.h"

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Components/WidgetComponent.h"

#include "TobiiBlueprintLibrary.generated.h"

/**
  * Simplified interface for blueprint use. Only exposes the features most likely to be consumed from BP.
  */
UCLASS()
class TOBIICORE_API UTobiiBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

	/************************************************************************/
	/* Common functions                                                     */
	/************************************************************************/
public:

	/**
	  * This is just a helper function to set the eyetracking enabled CVar for blueprint convenience.
	  * Set eyetracking disabled if you don't need tracking and don't want to pay the CPU cost in some parts of your program.
	  */
	UFUNCTION(BlueprintCallable, Category = "Tobii Eyetracking")
	static void SetTobiiEyetrackingEnabled(bool bEyetrackingEnabled);

	UFUNCTION(BlueprintPure, Category = "Tobii Eyetracking")
	static bool GetTobiiEyetrackingEnabled();

	/**
	  * This is just a helper function to set the eyetracking frozen CVar for blueprint convenience.
	  * Freezing the eyetracking can be very useful for debugging as well as when trying to show someone what effect eyetracking has.
	  */
	UFUNCTION(BlueprintCallable, Category = "Tobii Eyetracking")
	static void SetTobiiEyetrackingFrozen(bool bEyetrackingFrozen);

	UFUNCTION(BlueprintPure, Category = "Tobii Eyetracking")
	static bool GetTobiiEyetrackingFrozen();

	/**
	  * This is the main method to get basic eye tracking data. You should not be using this though for most things. Use the focus system instead if you can.
	  */
	UFUNCTION(BlueprintPure, Category = "Tobii Eyetracking")
	static FTobiiGazeData GetTobiiCombinedGazeData();
	UFUNCTION(BlueprintPure, Category = "Tobii Eyetracking")
	static FTobiiGazeData GetTobiiLeftGazeData();
	UFUNCTION(BlueprintPure, Category = "Tobii Eyetracking")
	static FTobiiGazeData GetTobiiRightGazeData();
	UFUNCTION(BlueprintPure, Category = "Tobii Eyetracking")
	static FHitResult GetTobiiCombinedWorldGazeHitData();

	/**
	  * This will indicate the readiness of the gaze tracking subsystem. For most applications this will also indicate head tracking readiness.
	  */
	UFUNCTION(BlueprintPure, Category = "Tobii Eyetracking")
	static ETobiiGazeTrackerStatus GetTobiiGazeTrackerStatus();
	
	/**
	  * Gets information about the display device and window used for the application.
	  */
	UFUNCTION(BlueprintPure, Category = "Tobii Eyetracking")
	static FTobiiDisplayInfo GetTobiiDisplayInformation();

	UFUNCTION(BlueprintPure, Category = "Tobii Eyetracking")
	static FRotator GetTobiiInfiniteScreenAngles();

	/************************************************************************/
	/* Desktop functions                                                    */
	/************************************************************************/
public:
	/**
	  * This is the main method to get basic head pose data.
	  */
	UFUNCTION(BlueprintPure, Category = "Tobii Desktop Eyetracking")
	static const FTobiiHeadPoseData& GetTobiiHeadPoseData();
	/**
	  * Track box information.
	  */
	UFUNCTION(BlueprintPure, Category = "Tobii Desktop Eyetracking")
	static const FTobiiDesktopTrackBox& GetTobiiDesktopTrackBox();

	/************************************************************************/
	/* Settings functions													*/
	/************************************************************************/
public:
	//Use these functions to get and set CVars in runtime.
	UFUNCTION(BlueprintPure, Category = "Tobii Settings")
	static int32 GetTobiiInt(FString CVarName);
	UFUNCTION(BlueprintCallable, Category = "Tobii Settings")
	static void SetTobiiInt(FString CVarName, const int32 NewValue);
	UFUNCTION(BlueprintPure, Category = "Tobii Settings")
	static float GetTobiiFloat(FString CVarName);
	UFUNCTION(BlueprintCallable, Category = "Tobii Settings")
	static void SetTobiiFloat(FString CVarName, const float NewValue);

	//If you are persisting your custom CVar values, you should call this function when you start your application.
	UFUNCTION(BlueprintCallable, Category = "Tobii Settings")
	static void LoadTobiiSettings();
	//If you want to persist a CVar to .ini, use this function.
	UFUNCTION(BlueprintCallable, Category = "Tobii Settings")
	static void SaveTobiiSetting(FString CVarSettingToSave);

	/************************************************************************/
	/* Math Util functions													*/
	/************************************************************************/
public:
	/**
	  * Use this function if you want to project slate information to viewport space. Includes DPI adjustments
	  */
	UFUNCTION(BlueprintPure, Category = "Tobii Math Utils")
	static bool VirtualDesktopPixelToViewportCoordinateUNorm(const FVector2D& VirtualDesktopPixel, FVector2D& OutViewportCoordinateUNorm);

	UFUNCTION(BlueprintPure, Category = "Tobii Math Utils")
	static bool ViewportCoordinateUNormToVirtualDesktopPixel(const FVector2D& ViewportCoordinateUNorm, FVector2D& OutVirtualDesktopPixel);

	UFUNCTION(BlueprintPure, Category = "Tobii Math Utils")
	static bool ViewportPixelCoordToCmCoord(const FVector2D& InCoordinatePx, FVector2D& OutCoordinateCm);

	UFUNCTION(BlueprintPure, Category = "Tobii Math Utils")
	static bool ViewportCmCoordToPixelCoord(const FVector2D& InCoordinateCm, FVector2D& OutCoordinatePx);

	UFUNCTION(BlueprintCallable, Category = "Tobii Math Utils")
	static bool ViewportPixelCoordToUNormCoord(const FVector2D& InCoordinatePx, FVector2D& OutCoordinateUNorm);

	UFUNCTION(BlueprintCallable, Category = "Tobii Math Utils")
	static bool ViewportUNormCoordToPixelCoord(const FVector2D& InCoordinateUNorm, FVector2D& OutCoordinatePx);

	/**
	  * This function lets you move from world space to the local space of the user. In XR this would be head space, and for desktop it would be relative to the scene camera.
	  */
	UFUNCTION(BlueprintPure, Category = "Tobii Math Utils")
	static FVector ConvertGazeToUserSpace(APlayerController* PlayerController, const FVector& WorldSpaceGazeDirection);

	/************************************************************************/
	/* Util functions														*/
	/************************************************************************/
public:
	/**
	* Tests if a given player controller is XR enabled
	*/
	UFUNCTION(BlueprintPure, Category = "Tobii XR Utils")
	static bool IsXRPlayerController(const APlayerController* PlayerController);
};
