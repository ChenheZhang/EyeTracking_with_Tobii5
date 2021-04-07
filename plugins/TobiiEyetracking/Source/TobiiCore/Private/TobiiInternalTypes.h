/******************************************************************************
* Copyright 2017- Tobii Technology AB. All rights reserved.
*
* @author Temaran | Fredrik Lindh | fredrik.lindh@tobii.com | https://github.com/Temaran
******************************************************************************/

#pragma once

#include "TobiiTypes.h"

#include "CoreMinimal.h"

class FEyetrackingUtils
{
public:
	static float CalculateFovealRegionHeightPx(const float MainViewportHeightCm, const float MainViewportHeightPx, const float HeadDistanceCm, const float FovealRegionAngleDeg)
	{
		if (MainViewportHeightCm > FLT_EPSILON && MainViewportHeightPx > FLT_EPSILON && HeadDistanceCm > FLT_EPSILON)
		{
			const float FovealRegionSizeCm = HeadDistanceCm * FMath::Tan(FMath::DegreesToRadians(FovealRegionAngleDeg));
			return FovealRegionSizeCm / MainViewportHeightCm * MainViewportHeightPx;
		}

		return 100.0f;
	}
};

struct FTobiiRawGazePoint
{
public:
	FVector2D GazePointNormalized;
	FDateTime TimeStamp;

	FTobiiRawGazePoint()
		: GazePointNormalized(0.0f, 0.0f)
		, TimeStamp()
	{ }
};

struct FTobiiRawHeadPose
{
public:
	FVector HeadPositionCm;
	FRotator HeadOrientation;
	FDateTime TimeStamp;

public:
	FTobiiRawHeadPose()
		: HeadPositionCm(0.0f, 0.0f, 0.0f)
		, HeadOrientation(0.0f, 0.0f, 0.0f)
		, TimeStamp()
	{}
};

struct FTobiiRawXREyetrackingData
{
public:
	FVector RawLeftEyeDirection;
	FVector RawRightEyeDirection;
	FDateTime EngineTimeStamp;
	int64 EyetrackerTimeStamp;
	bool bLeftEyeValid;
	bool bRightEyeValid;

	FTobiiRawXREyetrackingData()
		: RawLeftEyeDirection()
		, RawRightEyeDirection()
		, EngineTimeStamp()
		, EyetrackerTimeStamp(-1)
		, bLeftEyeValid(false)
		, bRightEyeValid(false)
	{
	}
};

DEFINE_LOG_CATEGORY_STATIC(LogTobiiEyetracking, All, All);
