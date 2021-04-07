/******************************************************************************
* Copyright 2017- Tobii Technology AB. All rights reserved.
*
* @author Temaran | Fredrik Lindh | fredrik.lindh@tobii.com | https://github.com/Temaran
******************************************************************************/

#pragma once

#if TOBII_EYETRACKING_ACTIVE

#include "TobiiGazeFocusableComponent.h"
#include "tobii_g2om.h"

#include "CoreMinimal.h"
#include "IEyeTracker.h"
#include "GameFramework/PlayerController.h"

enum class ETobiiTrackingBlastMode : uint8
{
	NoTrackingBlasts = 0
	, OnlyTrackMissedObjects = 1
	, TrackAllObjects = 2
};

struct FTobiiGTOMOcclusionData
{
public:
	TWeakObjectPtr<AActor> ParentActor;
	TWeakObjectPtr<UPrimitiveComponent> PrimitiveComponent;
	TWeakObjectPtr<UTobiiGazeFocusableComponent> FocusableComponent;
	FVector LastKnownVisibleWorldSpaceLocation;
	FDateTime LastHitTime;
};

class FTobiiGTOMOcclusionTester
{
public:
	FTobiiGTOMOcclusionTester();
	virtual ~FTobiiGTOMOcclusionTester() {}

	//Returns the visible set
	const TMap<FEngineFocusableUID, FTobiiGTOMOcclusionData>& Tick(float DeltaTimeSecs, APlayerController* PlayerController, const FEyeTrackerGazeData& GazeData, g2om_gaze_data& G2OMGazeData, g2om_context* G2OMContext);

private:
	TMap<FEngineFocusableUID, FTobiiGTOMOcclusionData> VisibleSet;

	void TestRay(const FDateTime& UtcNow, APlayerController* PlayerController, const FVector& Origin, const FVector& Direction, const FCollisionQueryParams& Params);
};

#endif //TOBII_EYETRACKING_ACTIVE
