/******************************************************************************
* Copyright 2017- Tobii Technology AB. All rights reserved.
*
* @author Temaran | Fredrik Lindh | fredrik.lindh@tobii.com | https://github.com/Temaran
******************************************************************************/

#pragma once

#include "CoreMinimal.h"
#include "CollisionQueryParams.h"

#include "TobiiInteractionsTypes.generated.h"

UENUM(BlueprintType)
enum class ETobiiInterceptType : uint8
{
	ClosestApproach,
	DirectHit
};

UENUM(BlueprintType)
enum class ETobiiThrowAtGazeResult : uint8
{
	// Bad input in some way
	UnknownError,

	// Neither a throw target, nor gaze input is available
	NoEyetrackingInput,

	// If no valid path could be found
	NoPath,

	// A path is mathematically possible, but it is blocked by something and out of range.
	BlockedByWorldAndOutOfRange,

	// A path is mathematically possible and is within range, but it is blocked by something.
	BlockedByWorldInRange,

	// If a valid path could be found, but it requires more throw force than allowed
	OutOfRange,

	// If a valid path exists and the force requirement is within limits
	DirectHit						
};

USTRUCT(BlueprintType)
struct FTobiiAccelerationBasedHomingData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "General Data")
	bool bAttemptClosestApproachSolution;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Source Data")
	FVector ProjectilePosition;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Source Data")
	FVector ProjectileVelocity;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Source Data")
	float ProjectileAccelerationMagnitude;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target Data")
	FVector TargetPosition;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target Data")
	FVector TargetVelocity;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target Data")
	FVector TargetAcceleration;
};

USTRUCT(BlueprintType)
struct FTobiiBallisticData
{
	GENERATED_BODY()

public:
	//This is how high the projectile should be above the initial position at it's midpoint
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "General Data")
	float ProjectileApexOffsetCm;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Source Data")
	FVector ProjectileInitialPosition;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Source Data")
	FVector ProjectileAcceleration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target Data")
	FVector TargetPosition;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target Data")
	FVector TargetVelocity;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target Data")
	FVector TargetAcceleration;
};

USTRUCT(BlueprintType)
struct FTobiiAccelerationBasedHomingResult
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intercept Data")
	ETobiiInterceptType Type;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intercept Data")
	FVector SuggestedAcceleration;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intercept Data")
	FVector ExpectedInterceptLocation;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intercept Data")
	float ExpectedInterceptTimeSecs;
};

USTRUCT(BlueprintType)
struct FTobiiBallisticResult
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intercept Data")
	FVector SuggestedInitialVelocity;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intercept Data")
	FVector ExpectedInterceptLocation;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intercept Data")
	float ExpectedInterceptTimeSecs;
};

USTRUCT(BlueprintType)
struct FTobiiProjectileTraceData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "General Data")
	TEnumAsByte<ECollisionChannel> TraceChannel;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "General Data")
	TArray<AActor*> IgnoredActors;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "General Data")
	int32 MaxNrSteps;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "General Data")
	float StepSizeSecs;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "General Data")
	float TraceRadiusCm;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Source Data")
	FVector ProjectileInitialPosition;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Source Data")
	FVector ProjectileVelocity;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Source Data")
	FVector ProjectileAcceleration;
};
