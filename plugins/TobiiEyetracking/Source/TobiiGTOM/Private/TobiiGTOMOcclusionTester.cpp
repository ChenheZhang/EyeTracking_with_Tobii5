/******************************************************************************
* Copyright 2017- Tobii Technology AB. All rights reserved.
*
* @author Temaran | Fredrik Lindh | fredrik.lindh@tobii.com | https://github.com/Temaran
******************************************************************************/

#if TOBII_EYETRACKING_ACTIVE

#include "TobiiGTOMOcclusionTester.h"
#include "TobiiGazeFocusableComponent.h"
#include "TobiiGTOMInternalTypes.h"
#include "tobii_g2om.h"

#include "DrawDebugHelpers.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/Engine.h"
#include "Engine/LocalPlayer.h"

#define TOBII_MAX_RAYS_PER_FRAME (15)

static TAutoConsoleVariable<int32> CVarOcclusionTesterTrackingBlastMode(TEXT("tobii.gtom.OcclusionTesterTrackingBlastMode"), (int32)ETobiiTrackingBlastMode::OnlyTrackMissedObjects, TEXT("0 - Do not use tracking blasts. This will be very fast, but quite unstable. 1 - Only do tracking blasts when an object was not hit by a shotgun pellet to reduce hysteres somewhat. Prefer this mode if you have a lot of small objects constantly in the active set and mode two is working too slowly. 2 - Do tracking blasts for every focus component in the active group. This greatly helps reduce hysteres and leads to a LOT more stable last visible locations by trading off performance since it will require an additional extra line cast per active focus object and is such O(n)."));
static TAutoConsoleVariable<float> CVarOcclusionTesterTracesPerSecond(TEXT("tobii.gtom.OcclusionTesterTracesPerSecond"), 700.0f, TEXT("This is the approximate number of additional line casts (in addition to the center one) that will be performed per second. Setting this to zero will make the shotgun behave like a normal line scorer. A higher number will improve reliability at the cost of performance."));
static TAutoConsoleVariable<float> CVarOcclusionTesterTimeToLive(TEXT("tobii.gtom.OcclusionTesterTimeToLive"), 0.5f, TEXT("This is how long an object will be kept in the visibility set given it hasn't been detected. Having this too high can make the occlusion testing system output objects that are no longer visible. Having it too low may lead to hysteres."));

static TAutoConsoleVariable<int32> CVarDebugDisplayGTOMVisibilityRays(TEXT("tobii.debug.DisplayGTOMVisibilityRays"), 1, TEXT("1 means we visualize the rays from GTOM"));

g2om_gaze_ray RaysThisFrame[TOBII_MAX_RAYS_PER_FRAME];

FTobiiGTOMOcclusionTester::FTobiiGTOMOcclusionTester()
{
}

const TMap<FEngineFocusableUID, FTobiiGTOMOcclusionData>& FTobiiGTOMOcclusionTester::Tick(float DeltaTimeSecs, APlayerController* PlayerController, const FEyeTrackerGazeData& GazeData, g2om_gaze_data& G2OMGazeData, g2om_context* G2OMContext)
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_TobiiEyetracking_GTOM_OcclusionTesting);

 	if (GEngine == nullptr
 		|| GEngine->GameViewport == nullptr
 		|| PlayerController == nullptr
 		|| PlayerController->GetWorld() == nullptr)
	{
		VisibleSet.Empty();
		return VisibleSet;
	}

	static const auto FovealAngleDegCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("tobii.FovealConeAngleDegrees"));
	const float FovealAngleDeg = FovealAngleDegCVar->GetFloat();
	
	const ETobiiTrackingBlastMode TrackingBlastMode = (ETobiiTrackingBlastMode)CVarOcclusionTesterTrackingBlastMode.GetValueOnGameThread();
	const int32 NrCastsThisFrame = FMath::Clamp(FMath::CeilToInt(CVarOcclusionTesterTracesPerSecond.GetValueOnGameThread() * DeltaTimeSecs), 3, TOBII_MAX_RAYS_PER_FRAME);
	const FDateTime UtcNow = FDateTime::UtcNow();
	
	FMemory::Memzero(RaysThisFrame, TOBII_MAX_RAYS_PER_FRAME * sizeof(g2om_gaze_ray));
	g2om_get_candidate_search_pattern(G2OMContext, &G2OMGazeData, NrCastsThisFrame, RaysThisFrame);

	FCollisionQueryParams CollisionQueryParams;
	CollisionQueryParams.AddIgnoredActor(PlayerController);
	CollisionQueryParams.AddIgnoredActor(PlayerController->GetPawn());

	//Do shotgun blast
	{
		QUICK_SCOPE_CYCLE_COUNTER(STAT_TobiiEyetracking_GTOM_OcclusionTesting_Shotgun);

		for (int32 RayIdx = 0; RayIdx < NrCastsThisFrame; RayIdx++)
		{
			g2om_gaze_ray& G2OMRay = RaysThisFrame[RayIdx];
			if (!G2OMRay.is_valid)
			{
				continue;
			}

			FVector CurrentDirection = FTobiiGTOMUtils::G2OMVectorToUE4Vector(G2OMRay.ray.direction);
			if (CurrentDirection.Normalize())
			{
				TestRay(UtcNow, PlayerController, GazeData.GazeOrigin, CurrentDirection, CollisionQueryParams);
			}
		}
	}
	
	//Do tracking rays
	if (TrackingBlastMode != ETobiiTrackingBlastMode::NoTrackingBlasts)
	{
		QUICK_SCOPE_CYCLE_COUNTER(STAT_TobiiEyetracking_GTOM_OcclusionTesting_TrackingBlasts);

		//First we should check if our previous objects are still visible using so called "tracking blasts". 
		//This is both to provide more stable last known locations as well as to avoid hysteresis.
		for (auto RecordIterator = VisibleSet.CreateIterator(); RecordIterator; ++RecordIterator)
		{
			const FTobiiGTOMOcclusionData& Record = RecordIterator.Value();

			if (TrackingBlastMode == ETobiiTrackingBlastMode::OnlyTrackMissedObjects && Record.LastHitTime == UtcNow)
			{
				continue;
			}

			FVector TrackingBlastDirection = Record.LastKnownVisibleWorldSpaceLocation - GazeData.GazeOrigin;
			if (TrackingBlastDirection.Normalize())
			{
				const float DotBetweenBlastDirAndGaze = FVector::DotProduct(GazeData.GazeDirection, TrackingBlastDirection);
				const float AngleBetweenObjectAndGazeDeg = FMath::RadiansToDegrees(FMath::Acos(DotBetweenBlastDirAndGaze));

				if (AngleBetweenObjectAndGazeDeg > FovealAngleDeg)
				{
					//We need to clamp the rotation amount.
					FVector RotationAxis = FVector::CrossProduct(GazeData.GazeDirection, TrackingBlastDirection);
					RotationAxis.Normalize();
					TrackingBlastDirection = GazeData.GazeDirection.RotateAngleAxis(FovealAngleDeg, RotationAxis);
				}

				TestRay(UtcNow, PlayerController, GazeData.GazeOrigin, TrackingBlastDirection, CollisionQueryParams);
			}
		}
	}

	//Decay old objects
	{
		QUICK_SCOPE_CYCLE_COUNTER(STAT_TobiiEyetracking_GTOM_OcclusionTesting_Decay);

		TArray<FEngineFocusableUID> DecayedIds;
		for (auto RecordIterator = VisibleSet.CreateIterator(); RecordIterator; ++RecordIterator)
		{
			const FTobiiGTOMOcclusionData& Record = RecordIterator.Value();
			if ((UtcNow - Record.LastHitTime).GetTotalSeconds() > CVarOcclusionTesterTimeToLive.GetValueOnGameThread())
			{
				DecayedIds.Add(RecordIterator.Key());
			}
		}

		for (FEngineFocusableUID DecayID : DecayedIds)
		{
			VisibleSet.Remove(DecayID);
		}
	}

	return VisibleSet;
}



void FTobiiGTOMOcclusionTester::TestRay(const FDateTime& UtcNow, APlayerController* PlayerController, const FVector& Origin, const FVector& Direction, const FCollisionQueryParams& Params)
{
	static const auto DrawDebugCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("tobii.debug"));
	const float DebugBoxSize = 2.0f;

	static const auto FocusTraceChannelCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("tobii.FocusTraceChannel"));
	static const auto MaximumTraceDistanceCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("tobii.MaximumTraceDistance"));
	const float MaxTraceDistance = MaximumTraceDistanceCVar->GetFloat();
	const ECollisionChannel TraceChannel = (ECollisionChannel)FocusTraceChannelCVar->GetInt();

	const FVector RayEndPoint = Origin + (Direction * MaxTraceDistance);

	FHitResult RayHitResult;
	if (PlayerController->GetWorld()->LineTraceSingleByChannel(RayHitResult, Origin, RayEndPoint, TraceChannel, Params))
	{
		if (DrawDebugCVar->GetInt() != 0 && CVarDebugDisplayGTOMVisibilityRays.GetValueOnGameThread())
		{
			DrawDebugBox(PlayerController->GetWorld(), RayHitResult.Location, FVector(DebugBoxSize, DebugBoxSize, DebugBoxSize), FColor::Green, false, 0.0f, 0, 2.0f);
		}

		if (UTobiiGazeFocusableComponent::IsPrimitiveFocusable(RayHitResult.GetComponent()))
		{
			//Range test
			bool bIsInRange = true;
			float MaxDistance;
			const float DistanceToPrimitive = FVector::Distance(Origin, RayHitResult.Location);
			if (UTobiiGazeFocusableComponent::GetMaxFocusDistanceForPrimitive(RayHitResult.GetComponent(), MaxDistance))
			{
				bIsInRange = DistanceToPrimitive <= MaxDistance;
			}

			if (bIsInRange)
			{
				FEngineFocusableUID Id = RayHitResult.GetComponent()->GetUniqueID();
				if (VisibleSet.Contains(Id))
				{
					VisibleSet[Id].LastKnownVisibleWorldSpaceLocation = RayHitResult.Location;
					VisibleSet[Id].LastHitTime = UtcNow;
				}
				else
				{
					FTobiiGTOMOcclusionData NewRecord;
					NewRecord.ParentActor = RayHitResult.GetActor();
					NewRecord.PrimitiveComponent = RayHitResult.GetComponent();
					NewRecord.LastKnownVisibleWorldSpaceLocation = RayHitResult.Location;
					NewRecord.LastHitTime = UtcNow;

					if (NewRecord.ParentActor.IsValid())
					{
						NewRecord.FocusableComponent = (UTobiiGazeFocusableComponent*)NewRecord.ParentActor->GetComponentByClass(UTobiiGazeFocusableComponent::StaticClass());
					}

					VisibleSet.Add(Id, MoveTemp(NewRecord));
				}
			}
		}
	}
	else if (DrawDebugCVar->GetInt() != 0 && CVarDebugDisplayGTOMVisibilityRays.GetValueOnGameThread())
	{
		DrawDebugBox(PlayerController->GetWorld(), RayEndPoint, FVector(DebugBoxSize, DebugBoxSize, DebugBoxSize), FColor::Red, false, 0.0f, 0, 2.0f);
	}
}

#endif //TOBII_EYETRACKING_ACTIVE
