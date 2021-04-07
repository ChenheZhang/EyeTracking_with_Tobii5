/******************************************************************************
* Copyright 2017- Tobii Technology AB. All rights reserved.
*
* @author Temaran | Fredrik Lindh | fredrik.lindh@tobii.com | https://github.com/Temaran
******************************************************************************/

#pragma once

#include "TobiiInteractionsTypes.h"

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "TobiiInteractionsBlueprintLibrary.generated.h"

/**
 * Simplified interface for blueprint use. Only exposes the features most likely to be consumed from BP.
 */
UCLASS()
class TOBIIINTERACTIONS_API UTobiiInteractionsBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Tobii Desktop Interaction")
	static bool IsInfiniteScreenEnabled();

	//Util functions to see if the feature can be considered enabled by the user.
	UFUNCTION(BlueprintPure, Category = "Tobii Desktop Interaction")
	static bool IsCleanUIEnabled();

	UFUNCTION(BlueprintPure, Category = "Tobii Desktop Interaction")
	static bool IsAimAtGazeEnabled();

	UFUNCTION(BlueprintPure, Category = "Tobii Desktop Interaction")
	static bool IsFireAtGazeEnabled();

	UFUNCTION(BlueprintPure, Category = "Tobii Desktop Interaction")
	static bool IsThrowAtGazeEnabled();

public:
	/**
	  * This function will determine an appropriate amount to dampen infinite screen depending on view pitch.
	  */
	UFUNCTION(BlueprintCallable, Category = "Tobii Infinite Screen")
	static float CalculateSmoothPitchStep(float ViewPitch);

	/**
	  * This function will determine an appropriate amount to dampen infinite screen depending on view pitch.
	  */
	UFUNCTION(BlueprintCallable, Category = "Tobii Infinite Screen")
	static FRotator MakeInfiniteScreenCameraRotator(FRotator OriginalCameraRotation, FRotator InfiniteScreenAngles);

public:
	/* 
	 * Render UI to a texture so we can use it on arbitrary meshes.
	 * Thanks to https://answers.unrealengine.com/questions/353532/rendering-a-umg-widget-on-a-static-mesh.html for this.
	 */
	UTexture2D* TextureFromWidget(UUserWidget* const Widget, const FVector2D& DrawSize);

	/**
	  * This function will find and return all real roots to the square equation Ax^2 + Bx + C = 0
	  */
	UFUNCTION(BlueprintCallable, Category = "Tobii Math Utils")
	static void FindRealSquareRoots(float A, float B, float C, TArray<float>& OutRealRoots);

	/**
	  * This function will find and return all real roots to the cubic equation Ax^3 + Bx^2 + Cx + D = 0
	  */
	UFUNCTION(BlueprintCallable, Category = "Tobii Math Utils")
	static void FindRealCubicRoots(float A, float B, float C, float D, TArray<float>& OutRealRoots);

	/**
	  * This function will find and return all real roots to the quartic equation Ax^4 + Bx^3 + Cx^2 + Dx + E = 0
	  * WARNING: The roots of a full degree quartic are generally not cheap to compute. Use with care.
	  */
	UFUNCTION(BlueprintCallable, Category = "Tobii Math Utils")
	static void FindRealQuarticRoots(float A, float B, float C, float D, float E, TArray<float>& OutRealRoots);

	/**
	  * This function will try to find which acceleration you will need for the current frame to hit a certain target given a homing projectile that uses acceleration for it's movement behavior.
	  */
	UFUNCTION(BlueprintCallable, Category = "Tobii Math Utils")
	static bool FindNeededAccelerationForAccelerationBasedHomingProjectile(const FTobiiAccelerationBasedHomingData& InputData, FTobiiAccelerationBasedHomingResult& BestResult);

	/**
	  * This function will try to find the initial velocity needed to hit a moving target with a ballistic projectile that passes through a selected apex point.
	  * To find an arc that works with your environment and thrower you might have to call this function multiple times and trace the results in your scene.
	  * 
	  * @return Whether a valid velocity could be found. If this is false, it usually means that SourceZ + ProjectileApexOffset was smaller than TargetZ. Try increasing the apex offset and try again.
	  */
	UFUNCTION(BlueprintCallable, Category = "Tobii Math Utils")
	static bool FindNeededInitialVelocityForBallisticProjectile(const FTobiiBallisticData& InputData, TArray<FTobiiBallisticResult>& Results);
	
	/**
	  * This function will trace along a ballistic path until it hits something. It will then return the traced path as well as what it hit.
	  *
	  * @return Whether anything was hit tracing the path
	  */
	UFUNCTION(BlueprintCallable, Category = "Tobii Math Utils", meta = (WorldContext = "WorldContextObject"))
	static bool TraceBallisticProjectilePath(UObject* WorldContextObject, const FTobiiProjectileTraceData& InputData, TArray<FVector>& OutTracedPath, FHitResult& OutHitResult);
};
