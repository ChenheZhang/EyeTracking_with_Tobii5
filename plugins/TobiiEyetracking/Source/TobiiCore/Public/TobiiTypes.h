/******************************************************************************
* Copyright 2017- Tobii Technology AB. All rights reserved.
*
* @author Temaran | Fredrik Lindh | fredrik.lindh@tobii.com | https://github.com/Temaran
******************************************************************************/

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"

#include "TobiiTypes.generated.h"

/**
  * Structure that contains gaze information from one eye in both screen and world space.
  */
USTRUCT(BlueprintType)
struct FTobiiGazeData
{
	GENERATED_USTRUCT_BODY()

public:
	//Origin of the eye's gaze ray in Unreal world space.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Space Gaze Data")
	FVector WorldGazeOrigin;
	//Forward direction of the eye's gaze ray this frame in Unreal world space.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Space Gaze Data")
	FVector WorldGazeDirection;
	//Due to how the eye works and imperfections in eye tracking technology, it makes more sense to express the world gaze field as a cone rather than a ray. This angle is the angle between the Gaze Direction and the side of the cone expressed in degrees.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Space Data")
	float WorldGazeConeAngleDegrees;

	//The gaze point in screen space in pixels this frame.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Space Gaze Data")
	FVector2D ScreenGazePointPx;
	//Due to how the eye works and imperfections in eye tracking technology, it makes more sense to express the screen space gaze field as an ellipse rather than a point. This is the extent of that ellipse expressed in pixels for the eye.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Space Data")
	FVector2D ScreenGazeCircleRadiiPx;

	//Time when the gaze point was created.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "General Data")
	FDateTime TimeStamp;
	//This is how open the eye is. 0 means closed, 1 means open.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "General Data")
	float EyeOpenness;
	//If the gaze point is moving slowly enough, it is considered stable. This is useful when trying to prevent false positives when the gaze point is moving past objects.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "General Data")
	bool bIsStable;
	//If this is true, all other properties related to the eye should be safe to use. This might be false because the tracking system is a mono tracker, or if the data is bad for this frame.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "General Data")
	bool bIsGazeDataValid;

	FTobiiGazeData()
		: bIsGazeDataValid(false)
	{
	}
};

/**
  * Location and orientation of the head.
  */
USTRUCT(BlueprintType)
struct FTobiiHeadPoseData
{
	GENERATED_USTRUCT_BODY()

public:
	FTobiiHeadPoseData()
		: HeadLocation(0.0f, 0.0f, 0.0f)
		, HeadOrientation(0.0f, 0.0f, 0.0f)
	{}

	//This is the local space location of the head relative to the device specified origin in centimeters.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Head Pose Data")
	FVector HeadLocation;
	//This is the local space orientation of the head.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Head Pose Data")
	FRotator HeadOrientation;
};

/**
  * Information regarding the display associated with the currently active eye tracker.
  */
USTRUCT(BlueprintType)
struct FTobiiDisplayInfo
{
	GENERATED_USTRUCT_BODY()

public:
	//This is the width in pixels of the monitor the game is running on as reported by windows.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Display Info")
	int32 MonitorWidthPx;
	//This is the height in pixels of the monitor the game is running on as reported by windows.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Display Info")
	int32 MonitorHeightPx;
	//This is the width in centimeters of the monitor the active eyetracker is attached to as reported by the eyetracker. Centimeters is also the measurement used for unreal units.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Display Info")
	float MonitorWidthCm;
	//This is the height in centimeters of the monitor the active eyetracker is attached to as reported by the eyetracker. Centimeters is also the measurement used for unreal units.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Display Info")
	float MonitorHeightCm;

	//This is the width in pixels of the main game viewport as reported by the game.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Display Info")
	int32 MainViewportWidthPx;
	//This is the height in pixels of the main game viewport as reported by the game.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Display Info")
	int32 MainViewportHeightPx;
	//This is the width of the main game viewport in centimeters as calculated using data from the game and the eyetracker. Centimeters is also the measurement used for unreal units.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Display Info")
	float MainViewportWidthCm;
	//This is the height of the main game viewport in centimeters as calculated using data from the game and the eyetracker. Centimeters is also the measurement used for unreal units.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Display Info")
	float MainViewportHeightCm;

	//This is the dpi scale of the monitor the game is running on
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Display Info")
	float DpiScale;

public:
	void* GameWindowHandle;
	void* GameMonitorHandle;

public:
	FTobiiDisplayInfo()
		: MonitorWidthPx(0)
		, MonitorHeightPx(0)
		, MonitorWidthCm(0.0f)
		, MonitorHeightCm(0.0f)

		, MainViewportWidthPx(0)
		, MainViewportHeightPx(0)
		, MainViewportWidthCm(0.0f)
		, MainViewportHeightCm(0.0f)

		, DpiScale(1.0f)

		, GameWindowHandle(nullptr)
		, GameMonitorHandle(nullptr)
	{}
};

/**
  * This is the vertices for the desktop only track box expressed in Tobii User Coordinate System coordinates.
  * This coordinate system has its origin in the middle of the active display. X+ is towards the user's right. Y+ is up. Z+ is away from the screen towards the user. 
  * See general API documentation for details.
  */
USTRUCT(BlueprintType)
struct FTobiiDesktopTrackBox
{
	GENERATED_USTRUCT_BODY()

public:
	//These are the extreme points of the eye tracker tracking frustum where we get optimal tracking quality in centimeters. These are in the Tobii User Coordinate System.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Track Box")
	FVector FrontUpperRightPointCm;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Track Box")
	FVector FrontUpperLeftPointCm;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Track Box")
	FVector FrontLowerLeftPointCm;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Track Box")
	FVector FrontLowerRightPointCm;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Track Box")
	FVector BackUpperRightPointCm;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Track Box")
	FVector BackUpperLeftPointCm;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Track Box")
	FVector BackLowerLeftPointCm;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Track Box")
	FVector BackLowerRightPointCm;

public:
	FTobiiDesktopTrackBox()
		: FrontUpperRightPointCm(0.0f, 0.0f, 0.0f)
		, FrontUpperLeftPointCm(0.0f, 0.0f, 0.0f)
		, FrontLowerLeftPointCm(0.0f, 0.0f, 0.0f)
		, FrontLowerRightPointCm(0.0f, 0.0f, 0.0f)
		, BackUpperRightPointCm(0.0f, 0.0f, 0.0f)
		, BackUpperLeftPointCm(0.0f, 0.0f, 0.0f)
		, BackLowerLeftPointCm(0.0f, 0.0f, 0.0f)
		, BackLowerRightPointCm(0.0f, 0.0f, 0.0f)
	{}
};

/**
  * Describes the current state of the gaze tracker. Is designed in such a way as later values indicates a higher level of readiness to make it easy to use in interactions.
  */
UENUM(BlueprintType)
enum class ETobiiGazeTrackerStatus : uint8
{
	/** The gaze tracker is not connected to UE4 for some reason. The tracker might not be plugged in, the game window is currently running on a screen without a gaze tracker or is otherwise not available. */
	NotConnected,

	/** The gaze tracker is connected, but cannot track as it is missing some important configuration step. */
	NotConfigured,

	/** Gaze Tracking has been disabled by the user or developer or some other eventuality, like for example the game being minimized which might suspend the tracking. */
	Disabled,

	/** The gaze tracker is running but has not yet detected a user. Not relevant in XR.  */
	UserNotPresent,

	/** The gaze tracker has detected a user and is actively tracking them. They appear not to be focusing on the game window at the moment however, so be careful about how you are using the data. */
	UserPresent
};

/**
  * Some gaze trackers might only be able to deliver data for one eye, and others might be able to serve data for both.
  * If you are planning to use the left or right gaze data functions, checking that the gaze tracker you are working with supports them is recommended.
  */
UENUM(BlueprintType)
enum class ETobiiGazeTrackerCapability : uint8
{
	/** This gaze tracker can deliver combined gaze data. */
	CombinedGazeData = 1	UMETA(DisplayName = "Supports Combined Gaze Data"),

	/** This gaze tracker can deliver individual gaze data for the left eye. */
	LeftGazeData = 2		UMETA(DisplayName = "Supports Left Eye Gaze Data"),

	/** This gaze tracker can deliver individual gaze data for the right eye. */
	RightGazeData = 4		UMETA(DisplayName = "Supports Right Eye Gaze Data")
};
