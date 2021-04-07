/******************************************************************************
* Copyright 2017- Tobii Technology AB. All rights reserved.
*
* @author Temaran | Fredrik Lindh | fredrik.lindh@tobii.com | https://github.com/Temaran
******************************************************************************/

#include "TobiiThrowAtGazeComponent.h"
#include "TobiiInteractionsBlueprintLibrary.h"
#include "../TobiiInteractionsInternalTypes.h"

#include "IEyeTracker.h"
#include "DrawDebugHelpers.h"
#include "Camera/CameraComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"

static TAutoConsoleVariable<int32> CVarThrowAtGazeEnabled(TEXT("tobii.interaction.ThrowAtGazeEnabled"), 1, TEXT("Throw where your are looking."));
static TAutoConsoleVariable<int32> CVarThrowAtGazeDebug(TEXT("tobii.debug.ThrowAtGaze"), 1, TEXT("Draw throw at gaze debug data."));

UTobiiThrowAtGazeComponent::UTobiiThrowAtGazeComponent()
	: ThrowTarget(nullptr)
	, MaxThrowSpeedCmPerSecs(1000.0f)

	, MaxIterations(5)
	, bAlwaysPreferLowApex(false)
	, bPreferLowApexForTargetsBelowThrower(true)
	, bShouldTraceResult(true)
	, ThrowApexOffsetMinimumCm(10.0f)
	, ThrowApexOffsetMaximumCm(500.0f)
	, MinimumApexDeltaCm(10.0f)
	, TraceRadiusCm(1.0f)
	, MaxNrTraceSteps(50)
	, TraceStepSizeSecs(0.1f)
	, MaxTraceDistance(10000.0f)
	, NoTargetAcceptanceThreshold(10.0f)
	, TraceChannel(ECC_Visibility)
	, TraceIgnoreActors()
	, bShouldTraceIgnoreOwner(true)

	, bUseCustomGravity(false)
	, CustomProjectileGravity(FVector::ZeroVector)

	, LastTargetVelocity(FVector::ZeroVector)
	, CalculatedTargetAcceleration(FVector::ZeroVector)
{
}

FVector UTobiiThrowAtGazeComponent::CalculateProjectileGravityVector()
{
	return bUseCustomGravity ? CustomProjectileGravity : FVector(0.0f, 0.0f, GetWorld()->GetGravityZ());
}

bool UTobiiThrowAtGazeComponent::ThrowAtGazeAvailable()
{
	return UTobiiInteractionsBlueprintLibrary::IsThrowAtGazeEnabled();
}

bool UTobiiThrowAtGazeComponent::TraceBallisticProjectilePath(FVector ProjectileOrigin, FVector ProjectileInitialVelocity, TArray<FVector>& OutTracedPath)
{
	TArray<AActor*> IgnoredActors = TraceIgnoreActors;
	if (bShouldTraceIgnoreOwner)
	{
		IgnoredActors.Add(GetOwner());
	}

	FTobiiProjectileTraceData TraceData;
	TraceData.TraceChannel = TraceChannel;
	TraceData.IgnoredActors = IgnoredActors;
	TraceData.MaxNrSteps = MaxNrTraceSteps;
	TraceData.StepSizeSecs = TraceStepSizeSecs;
	TraceData.TraceRadiusCm = TraceRadiusCm;
	TraceData.ProjectileInitialPosition = ProjectileOrigin;
	TraceData.ProjectileVelocity = ProjectileInitialVelocity;
	TraceData.ProjectileAcceleration = CalculateProjectileGravityVector();

	FHitResult HitResult;
	return UTobiiInteractionsBlueprintLibrary::TraceBallisticProjectilePath(GetWorld(), TraceData, OutTracedPath, HitResult);
}

void UTobiiThrowAtGazeComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	if (ThrowTarget.IsValid())
	{
		const float AccelerationLerpSpeed = 0.7f;
		FVector CurrentTargetVelocity = ThrowTarget->GetOwner()->GetVelocity();
		CalculatedTargetAcceleration = FMath::Lerp(CalculatedTargetAcceleration, CurrentTargetVelocity - LastTargetVelocity, AccelerationLerpSpeed);
		LastTargetVelocity = CurrentTargetVelocity;
	}
}

FVector UTobiiThrowAtGazeComponent::CorrectThrow(const FVector& ThrowOrigin, const FVector& OriginalThrowVector, float ThrowAngleThresholdDeg, float ThrowSpeedThreshold)
{
	FTobiiBallisticResult BallisticResult;
	TArray<FVector> TracedPath;
	ETobiiThrowAtGazeResult Result = CalculateThrowAtGazeVector(ThrowOrigin, BallisticResult, TracedPath);

	switch (Result)
	{
		case ETobiiThrowAtGazeResult::OutOfRange:
		case ETobiiThrowAtGazeResult::DirectHit:
		{
			if (ThrowAngleThresholdDeg > FLT_EPSILON)
			{
				FVector SuggestedDirection = BallisticResult.SuggestedInitialVelocity;
				FVector OriginalDirection = OriginalThrowVector;
				bool bHasValidVectors = SuggestedDirection.Normalize();
				bHasValidVectors = bHasValidVectors && OriginalDirection.Normalize();

				if (bHasValidVectors)
				{
					float SeparationDot = FVector::DotProduct(SuggestedDirection, OriginalDirection);
					float SeparationAngleDeg = FMath::RadiansToDegrees(FMath::Acos(SeparationDot));
					if (SeparationAngleDeg > ThrowAngleThresholdDeg)
					{
						static const auto DrawDebugCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("tobii.debug"));
						if (DrawDebugCVar->GetInt() && CVarThrowAtGazeDebug.GetValueOnGameThread())
						{
							UE_LOG(LogTemp, Warning, TEXT("Throw diverged too much. Separation angle was: %f"), SeparationAngleDeg);
						}

						return OriginalThrowVector;
					}
				}
			}

			if (ThrowSpeedThreshold > FLT_EPSILON)
			{
				float SuggestedSpeedSq = BallisticResult.SuggestedInitialVelocity.SizeSquared();
				float OriginalSpeedSq = OriginalThrowVector.SizeSquared();
				float ThresholdSq = ThrowSpeedThreshold * ThrowSpeedThreshold;

				//Only invalidate too weak throws
				if (OriginalSpeedSq + ThresholdSq < SuggestedSpeedSq)
				{
					static const auto DrawDebugCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("tobii.debug"));
					if (DrawDebugCVar->GetInt() && CVarThrowAtGazeDebug.GetValueOnGameThread())
					{
						UE_LOG(LogTemp, Warning, TEXT("Throw was too weak. Throw speed was: %f. Suggested speed was: %f"), OriginalThrowVector.Size(), BallisticResult.SuggestedInitialVelocity.Size());
					}

					return OriginalThrowVector;
				}
			}
			
			return BallisticResult.SuggestedInitialVelocity;
		}

		default:
		{
			return OriginalThrowVector;
		}
	}
}

ETobiiThrowAtGazeResult UTobiiThrowAtGazeComponent::CalculateThrowAtGazeVector(const FVector& ThrowOrigin, FTobiiBallisticResult& OutBallisticResult, TArray<FVector>& OutTracedPath)
{
	UWorld* World = GetWorld();
	if (World == nullptr || GEngine == nullptr || !GEngine->EyeTrackingDevice.IsValid())
	{
		return ETobiiThrowAtGazeResult::UnknownError;
	}

	if (ThrowTarget.IsValid())
	{
		return CalculateThrowArc(ThrowOrigin, ThrowTarget->GetCenterOfMass(), ThrowTarget->GetOwner()->GetVelocity(), CalculatedTargetAcceleration, OutBallisticResult, OutTracedPath);
	}
	else if (World->GetFirstPlayerController() != nullptr && World->GetFirstPlayerController()->PlayerCameraManager != nullptr)
	{
		FVector GazeTargetPosition;
		FVector GazeRayOrigin = FVector::ZeroVector;
		FVector GazeRayDirection = FVector::ZeroVector;

		FEyeTrackerGazeData CombinedGazeData;
		GEngine->EyeTrackingDevice->GetEyeTrackerGazeData(CombinedGazeData);
		if (CombinedGazeData.ConfidenceValue > 0.5f)
		{
			GazeRayOrigin = CombinedGazeData.GazeOrigin;
			GazeRayDirection = CombinedGazeData.GazeDirection;
		}
		else
		{
			// If eyetracking data is not available this frame, just use the center of the screen
			GazeRayOrigin = World->GetFirstPlayerController()->PlayerCameraManager->GetCameraLocation();
			GazeRayDirection = World->GetFirstPlayerController()->PlayerCameraManager->GetCameraRotation().Vector();
		}

		FVector EndPoint = GazeRayOrigin + GazeRayDirection * MaxTraceDistance;
		FHitResult HitResult;
		if (World->LineTraceSingleByChannel(HitResult, GazeRayOrigin, EndPoint, TraceChannel))
		{
			GazeTargetPosition = HitResult.Location;
		}
		else
		{
			GazeTargetPosition = EndPoint;
		}

		return CalculateThrowArc(ThrowOrigin, GazeTargetPosition, FVector::ZeroVector, FVector::ZeroVector, OutBallisticResult, OutTracedPath);
	}

	return ETobiiThrowAtGazeResult::NoEyetrackingInput;
}

ETobiiThrowAtGazeResult UTobiiThrowAtGazeComponent::CalculateThrowArc(const FVector& ThrowOrigin, const FVector& TargetLocation, const FVector& TargetVelocity, const FVector& TargetAcceleration, FTobiiBallisticResult& OutBallisticResult, TArray<FVector>& OutTracedPath)
{
	OutBallisticResult = FTobiiBallisticResult();
	OutTracedPath = TArray<FVector>();
	if (GetWorld() == nullptr)
	{
		return ETobiiThrowAtGazeResult::UnknownError;
	}

	static const auto DrawDebugCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("tobii.debug"));
	const FVector ProjectileAcceleration = CalculateProjectileGravityVector();
	const float BaseAcceptanceThreshold = TraceRadiusCm + NoTargetAcceptanceThreshold;
	const float TraceAcceptanceThresholdSq = BaseAcceptanceThreshold * BaseAcceptanceThreshold;

	bool bShouldDrawDebug = DrawDebugCVar->GetInt() && CVarThrowAtGazeDebug.GetValueOnGameThread();
	if (bShouldDrawDebug)
	{
		DrawDebugSphere(GetWorld(), TargetLocation, 20.0f, 16, FColor::Yellow, false, 5.0f);
	}

	FTobiiBallisticData BallisticData;
	BallisticData.ProjectileInitialPosition = ThrowOrigin;
	BallisticData.ProjectileAcceleration = ProjectileAcceleration;
	BallisticData.TargetPosition = TargetLocation;
	BallisticData.TargetVelocity = TargetVelocity;
	BallisticData.TargetAcceleration = TargetAcceleration;

	TArray<AActor*> IgnoredActors = TraceIgnoreActors;
	if (bShouldTraceIgnoreOwner)
	{
		IgnoredActors.Add(GetOwner());
	}

	FTobiiProjectileTraceData TraceData;
	TraceData.TraceChannel = TraceChannel;
	TraceData.IgnoredActors = IgnoredActors;
	TraceData.MaxNrSteps = MaxNrTraceSteps;
	TraceData.StepSizeSecs = TraceStepSizeSecs;
	TraceData.TraceRadiusCm = TraceRadiusCm;
	TraceData.ProjectileInitialPosition = ThrowOrigin;
	TraceData.ProjectileAcceleration = ProjectileAcceleration;

	TArray<float> ApexesToTest;
	if (MaxIterations > 0)
	{
		//We want to first decide which apexes we should test, and then we can use logic to suss out which one ends up being the best.
		int32 Iterations = FMath::Max(0, MaxIterations);
		float ApexStep = (ThrowApexOffsetMaximumCm - ThrowApexOffsetMinimumCm) / Iterations;
		for (int32 ApexIdx = 0; ApexIdx <= Iterations; ApexIdx++)
		{
			float CurrentApex = ThrowApexOffsetMinimumCm + ApexIdx * ApexStep;
			ApexesToTest.Add(CurrentApex);
		}
	}
	else
	{
		ApexesToTest.Add((ThrowApexOffsetMaximumCm + ThrowApexOffsetMinimumCm) / 2);
	}

	// Loop variables
	float BestResultDistanceToTargetSq = FLT_MAX;
	ETobiiThrowAtGazeResult BestResultStatus = ETobiiThrowAtGazeResult::NoPath;
	while (ApexesToTest.Num() > 0)
	{
		// Pick the next apex
		int32 SelectedApexIndex;
		if (bAlwaysPreferLowApex || (bPreferLowApexForTargetsBelowThrower && TargetLocation.Z < ThrowOrigin.Z))
		{
			SelectedApexIndex = 0;
		}
		else
		{
			SelectedApexIndex = (int32)(ApexesToTest.Num() / 2.0f);
		}
		SelectedApexIndex = FMath::Clamp(SelectedApexIndex, 0, ApexesToTest.Num() - 1);
		BallisticData.ProjectileApexOffsetCm = ApexesToTest[SelectedApexIndex];
		ApexesToTest.RemoveAt(SelectedApexIndex);

		// Find initial velocity for given apex
		TArray<FTobiiBallisticResult> Results;
		if (UTobiiInteractionsBlueprintLibrary::FindNeededInitialVelocityForBallisticProjectile(BallisticData, Results))
		{
			for (FTobiiBallisticResult& Result : Results)
			{
				float SuggestedInitialSpeed = Result.SuggestedInitialVelocity.Size();
				if (SuggestedInitialSpeed < FLT_EPSILON)
				{
					continue;
				}

				bool bIsInRange = SuggestedInitialSpeed < MaxThrowSpeedCmPerSecs;
				if (!bIsInRange)
				{
					Result.SuggestedInitialVelocity.Normalize();
					Result.SuggestedInitialVelocity *= MaxThrowSpeedCmPerSecs;
				}

				TArray<FVector> TracedPath;
				bool bWayIsClear = true;
				float DistanceToTargetSq = FLT_MAX;
				if (bShouldTraceResult)
				{
					TraceData.ProjectileVelocity = Result.SuggestedInitialVelocity;

					FHitResult HitResult;
					if (UTobiiInteractionsBlueprintLibrary::TraceBallisticProjectilePath(GetWorld(), TraceData, TracedPath, HitResult))
					{
						FVector HitToTarget = TargetLocation - HitResult.Location;
						float ToTargetDistSq = HitToTarget.SizeSquared();

						if (ThrowTarget.IsValid())
						{
							if (HitResult.Actor != ThrowTarget->GetOwner())
							{
								bWayIsClear = false; // If we have a target, and hit something, that's okay as long as it's our target.
							}
						}
						else
						{
							bWayIsClear = ToTargetDistSq < TraceAcceptanceThresholdSq;
						}

						DistanceToTargetSq = ToTargetDistSq;
					}

					if (bShouldDrawDebug)
					{
						if (TracedPath.Num() > 1)
						{
							FColor DrawColor = bWayIsClear ? FColor::Green : FColor::Red;
							FVector PrevPoint = TracedPath[0];
							int32 MiddlePointIdx = (int32)(TracedPath.Num() / 2.0f);
							if (MiddlePointIdx >= 0 && MiddlePointIdx < TracedPath.Num())
							{
								DrawDebugString(GetWorld(), TracedPath[MiddlePointIdx] + FVector(0.0f, 0.0f, 10.0f), FString::Printf(TEXT("%f"), BallisticData.ProjectileApexOffsetCm), nullptr, DrawColor, 5.0f);
							}
							for (auto& TracePoint : TracedPath)
							{
								DrawDebugLine(GetWorld(), PrevPoint, TracePoint, DrawColor, false, 5.0f, 0, 1.0f);
								PrevPoint = TracePoint;
							}
						}
					}
				}

				if (bWayIsClear)
				{
					if (bIsInRange)
					{
						//We're done! Best outcome.
						OutBallisticResult = Result;
						OutTracedPath = TracedPath;
						BestResultDistanceToTargetSq = DistanceToTargetSq;
						BestResultStatus = ETobiiThrowAtGazeResult::DirectHit;
						break;
					}
					else if (BestResultStatus <= ETobiiThrowAtGazeResult::OutOfRange)
					{
						bool bIsFaster = Result.ExpectedInterceptTimeSecs < OutBallisticResult.ExpectedInterceptTimeSecs;
						if (bIsFaster)
						{
							OutBallisticResult = Result;
							OutTracedPath = TracedPath;
							BestResultDistanceToTargetSq = DistanceToTargetSq;
							BestResultStatus = ETobiiThrowAtGazeResult::OutOfRange;
						}
					}
				}
				else if (TracedPath.Num() > 0)
				{
					if (bIsInRange)
					{
						if (BestResultStatus <= ETobiiThrowAtGazeResult::BlockedByWorldInRange)
						{
							bool bIsCloser = DistanceToTargetSq < BestResultDistanceToTargetSq;
							if (bIsCloser)
							{
								OutBallisticResult = Result;
								OutTracedPath = TracedPath;
								BestResultDistanceToTargetSq = DistanceToTargetSq;
								BestResultStatus = ETobiiThrowAtGazeResult::BlockedByWorldInRange;
							}
						}
					}
					else
					{
						if (BestResultStatus <= ETobiiThrowAtGazeResult::BlockedByWorldAndOutOfRange)
						{
							bool bIsCloser = DistanceToTargetSq < BestResultDistanceToTargetSq;
							if (bIsCloser)
							{
								OutBallisticResult = Result;
								OutTracedPath = TracedPath;
								BestResultDistanceToTargetSq = DistanceToTargetSq;
								BestResultStatus = ETobiiThrowAtGazeResult::BlockedByWorldAndOutOfRange;
							}
						}
					}
				}
			}
		}

		if (BestResultStatus == ETobiiThrowAtGazeResult::DirectHit)
		{
			break;
		}

		//Use what we have learned to prune apexes for future loops
		if (Results.Num() == 0)
		{
			//If we didn't have any solutions or we are out of range, it means the apex is too low. Clear all apexes that are smaller than our current.
			for (int32 CurrentApexIdx = 0; CurrentApexIdx < ApexesToTest.Num(); CurrentApexIdx++)
			{
				if (ApexesToTest[CurrentApexIdx] <= BallisticData.ProjectileApexOffsetCm)
				{
					ApexesToTest.RemoveAt(CurrentApexIdx);
					CurrentApexIdx--;
				}
			}
		}
		else if (BestResultStatus == ETobiiThrowAtGazeResult::OutOfRange)
		{
			//If we are out of range, we need to select an apex closer to a 45 degree throw angle.
			//@TODO: Should we maybe do a solve for throw force given 45 degree throw angle here to optimize?
			FVector VelNorm = OutBallisticResult.SuggestedInitialVelocity;
			FVector ProjectedToXY = FVector(VelNorm.X, VelNorm.Y, 0.0f);
			bool bHasValidVectors = VelNorm.Normalize();
			bHasValidVectors = bHasValidVectors && ProjectedToXY.Normalize();
			if (bHasValidVectors)
			{
				float ThrowAngleCos = FVector::DotProduct(VelNorm, ProjectedToXY);

				//1 means it is along the ground, 0 is straight up. So if the value is between 0.5 and 1 we need to aim higher and if it's between 0 and 0.5 we need to go lower
				if (ThrowAngleCos < 0.5f)
				{
					for (int32 CurrentApexIdx = 0; CurrentApexIdx < ApexesToTest.Num(); CurrentApexIdx++)
					{
						if (ApexesToTest[CurrentApexIdx] >= BallisticData.ProjectileApexOffsetCm)
						{
							ApexesToTest.RemoveAt(CurrentApexIdx);
							CurrentApexIdx--;
						}
					}
				}
				else
				{
					for (int32 CurrentApexIdx = 0; CurrentApexIdx < ApexesToTest.Num(); CurrentApexIdx++)
					{
						if (ApexesToTest[CurrentApexIdx] <= BallisticData.ProjectileApexOffsetCm)
						{
							ApexesToTest.RemoveAt(CurrentApexIdx);
							CurrentApexIdx--;
						}
					}
				}
			}
		}
	}

	// Output results
	return BestResultStatus;
}
