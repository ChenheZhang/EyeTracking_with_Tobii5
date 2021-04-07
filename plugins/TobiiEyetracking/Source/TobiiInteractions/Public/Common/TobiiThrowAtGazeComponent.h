/******************************************************************************
* Copyright 2017- Tobii Technology AB. All rights reserved.
*
* @author Temaran | Fredrik Lindh | fredrik.lindh@tobii.com | https://github.com/Temaran
******************************************************************************/

#pragma once

#include "TobiiInteractionsTypes.h"

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/PrimitiveComponent.h"

#include "TobiiThrowAtGazeComponent.generated.h"

/**
  * This component will generate gaze contingent throw vectors on demand.
  * You can use this vector to either simulate a trajectory yourself, or simply apply it to the rigid body of some object.
  */
UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent), ClassGroup = (Tobii))
class TOBIIINTERACTIONS_API UTobiiThrowAtGazeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTobiiThrowAtGazeComponent();

public:
	// This is the component that you're trying to hit with the throw. You can set this on the same frame you are trying to throw, but setting it earlier will allow the component to "aim" a bit better. If this is not set, you can still use the component, but it will then try to simply use the gaze vector instead.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throw@gaze")
	TWeakObjectPtr<UPrimitiveComponent> ThrowTarget;

	// Throw force applied to the throw vector can never be greater than this value.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throw@gaze")
	float MaxThrowSpeedCmPerSecs;

	// This is how high above the thrower the arc apex will be.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throw@gaze")
	float ThrowApexOffsetCm;
	   
	// This is how many times we will run the angle finding code. More iterations will mean more tested apexes between the limits below. Setting this to 0 will only run one test using the average of the apexes.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throw@gaze Configuration")
	int32 MaxIterations;

	// If this is true, the algorithm will always test low apexes first.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throw@gaze Configuration")
	bool bAlwaysPreferLowApex;

	// If this is true, the algorithm will prefer low apexes rather than mid or high ones for a more natural looking throw.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throw@gaze Configuration")
	bool bPreferLowApexForTargetsBelowThrower;
	
	// If this is true, the algorithm will attempt to trace the solution in the world to make sure nothing is in the way.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throw@gaze Configuration")
	bool bShouldTraceResult;

	// If bShouldTraceResult is true, an apex smaller than this value will never be tested
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throw@gaze Configuration")
	float ThrowApexOffsetMinimumCm;

	// If bShouldTraceResult is true, an apex larger than this value will never be tested
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throw@gaze Configuration")
	float ThrowApexOffsetMaximumCm;

	// When testing different apexes, if the delta between the last test and the next one is smaller than this value, tests will be aborted
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throw@gaze Configuration")
	float MinimumApexDeltaCm;

	// If bShouldTraceResult is true, this is the size that will be used for tracing. Should be larger or equal to the projectile size.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throw@gaze Configuration")
	float TraceRadiusCm;

	// If bShouldTraceResult is true, this is the length of each trace step done.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throw@gaze Configuration")
	int32 MaxNrTraceSteps;
	
	// If bShouldTraceResult is true, this is the length of each trace step done in seconds.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throw@gaze Configuration")
	float TraceStepSizeSecs;

	// We don't allow traces longer than this
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throw@gaze Configuration")
	float MaxTraceDistance;

	// If we don't have a ThrowTarget, we will consider hits closer than this distance to the target to be "hits"
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throw@gaze Configuration")
	float NoTargetAcceptanceThreshold;

	// This is the channel that will be used when tracing.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throw@gaze Configuration")
	TEnumAsByte<ECollisionChannel> TraceChannel;
	
	// If you have any additional actors that should be ignored by the tracing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throw@gaze Configuration")
	TArray<AActor*> TraceIgnoreActors;

	// Let's you ignore this component's owner collision when tracing.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throw@gaze Configuration")
	bool bShouldTraceIgnoreOwner;

	// If this is false, the world gravity is used for calculations. If this is true, CustomProjectileGravity is used instead.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throw@gaze Gravity")
	bool bUseCustomGravity;

	// If bUseCustomGravity is true, this is the gravity that will be used in calculations
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throw@gaze Gravity")
	FVector CustomProjectileGravity;
	
public:
	UFUNCTION(BlueprintPure, Category = "Fire@gaze")
	FVector CalculateProjectileGravityVector();

	UFUNCTION(BlueprintPure, Category = "Fire@gaze")
	bool ThrowAtGazeAvailable();

	UFUNCTION(BlueprintPure, Category = "Fire@gaze")
	bool TraceBallisticProjectilePath(FVector ProjectileOrigin, FVector ProjectileInitialVelocity, TArray<FVector>& OutTracedPath);

	/**
	  * This function is useful when throwing things in XR and you have a throw vector from the player's attempt to throw something.
	  * Don't forget to set collision on the thing you're throwing etc before using this though, otherwise tracing might not work.
	  * It will only correct trajectories where we can expect a direct hit, or where we are simply out of range.
	  *
	  * @param ThrowOrigin				This is where the projectile will start.
	  * @param OriginalThrowVector		This is the throw vector supplied by the developer to correct.
	  * @param ThrowAngleThresholdDeg	If the angle difference between the OriginalThrowVector and the suggested vector is greater than this parameter, the vector will not be corrected. This is not applied if it is 0.
	  * @param ThrowSpeedThreshold		If the OriginalThrowVector speed is lower than the suggested vector's speed by at least this amount, the vector will not be corrected. This is measured in centimeters per second. This is not applied if it is 0.
	  * @param bUseSimple				If this is true, CalculateThrowAtGazeVectorSimple is used in favor of the Complex version internally
	  * @return							The potentially corrected throw vector
	  */
	UFUNCTION(BlueprintCallable, Category = "Throw@gaze")
	FVector CorrectThrow(const FVector& ThrowOrigin, const FVector& OriginalThrowVector, float ThrowAngleThresholdDeg, float ThrowSpeedThresholdCmPerSec);

	/**
	  * Call this function to request an impulse vector to use for gaze throwing.
	  * This function will start by testing the standard apex and force, and if that works it will just return it.
	  * If no solution is found, the algorithm will test other apex heights until either MaxNrIterations is reached or a result is found.
	  *
	  * @param ThrowOrigin			This is where the projectile will start.
	  * @param OutBallisticResult	This are our results.
	  * @param OutTracedPath		If path tracing is on, this will contain the traced path.
	  * @return						The result of the calculation. Please note that in the case of OutOfRange, the best solution will still be returned in OutGazeThrowVector, adjusted to your max throw speed
	  */
	UFUNCTION(BlueprintCallable, Category = "Throw@gaze")
	ETobiiThrowAtGazeResult CalculateThrowAtGazeVector(const FVector& ThrowOrigin, FTobiiBallisticResult& OutBallisticResult, TArray<FVector>& OutTracedPath);

	/**
	  * This is a companion function to CalculateThrowAtGazeVectorComplex that lets you calculate a throw vector towards a world location.
	  * You can specify target velocity and acceleration if you would like to.
	  *
	  * @param ThrowOrigin			This is where the projectile will start.
	  * @param TargetLocation		This is where the target is.
	  * @param TargetVelocity		This is the target's velocity. Can be zero.
	  * @param TargetAcceleration	This is the target's acceleration. Can be zero.
	  * @param OutBallisticResult	This are our results.
	  * @param OutTracedPath		If path tracing is on, this will contain the traced path.
	  * @return						The result of the calculation. Please note that in the case of OutOfRange, the best solution will still be returned in OutGazeThrowVector, adjusted to your max throw speed
	  */
	UFUNCTION(BlueprintCallable, Category = "Throw@gaze")
	ETobiiThrowAtGazeResult CalculateThrowArc(const FVector& ThrowOrigin, const FVector& TargetLocation, const FVector& TargetVelocity, const FVector& TargetAcceleration, FTobiiBallisticResult& OutBallisticResult, TArray<FVector>& OutTracedPath);

public:
	void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

private:
	FVector LastTargetVelocity;
	FVector CalculatedTargetAcceleration;
};
