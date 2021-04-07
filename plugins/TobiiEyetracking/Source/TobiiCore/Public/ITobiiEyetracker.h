/******************************************************************************
* Copyright 2017- Tobii Technology AB. All rights reserved.
*
* @author Temaran | Fredrik Lindh | fredrik.lindh@tobii.com | https://github.com/Temaran
******************************************************************************/

#pragma once

#include "TobiiTypes.h"

#include "CoreMinimal.h"
#include "IEyeTracker.h"

class ITobiiEyeTracker : public IEyeTracker
{
	/************************************************************************/
	/* Common Data                                                          */
	/************************************************************************/
public:
	/**
	  * Will return gaze data for a combination of both eyes.
	  * For mono trackers this might mean sending the data from the active eye.
	  * For a more advanced system, it might be some combination of both eyes.
	  * It contains all kinds of data you might need for low level interaction.
	  *
	  * @returns	Combined gaze data.
	  */
	virtual const FTobiiGazeData& GetCombinedGazeData() const = 0;

	/**
	  * Will return the gaze data for the left eye.
	  * It contains all kinds of data you might need for low level interaction.
	  *
	  * @returns	Gaze data for the right eye.
	  */
	virtual const FTobiiGazeData& GetLeftGazeData() const = 0;

	/**
	  * Will return the gaze data for the right eye.
	  * It contains all kinds of data you might need for low level interaction.
	  *
	  * @returns	Gaze data for the left eye.
	  */
	virtual const FTobiiGazeData& GetRightGazeData() const = 0;

	/**
	  * Returns information about the status of the current device.
	  *
	  * @return The status of the device.
	  */
	virtual ETobiiGazeTrackerStatus GetGazeTrackerStatus() const = 0;

	/**
	  * Some gaze trackers might only be able to deliver data for one eye, and others might be able to serve data for both.
	  * If you are planning to use other functions than GetCombinedGazeData, checking that the gaze tracker you are working with supports them is recommended.
	  *
	  * @param Capability	The capability to poll.
	  * @returns				Whether the gaze tracker supports the capability polled.
	  */
	virtual bool GetGazeTrackerCapability(ETobiiGazeTrackerCapability Capability) const = 0;

	/**
	  * This is information about where the combined gaze ray hits the world. As this is needed by many different gaze related features, we do this raycast for the developer.
	  *
	  * @returns				Relevant hit result.
	  */
	virtual const FHitResult& GetCombinedWorldGazeHitData() const = 0;

	/**
	  * This is information about where the combined gaze ray hits the world. As this is needed by many different gaze related features, we do this raycast for the developer.
	  *
	  * @returns				Relevant hit result.
	  */
	virtual const FHitResult& GetLeftWorldGazeHitData() const = 0;

	/**
	  * This is information about where the combined gaze ray hits the world. As this is needed by many different gaze related features, we do this raycast for the developer.
	  *
	  * @returns				Relevant hit result.
	  */
	virtual const FHitResult& GetRightWorldGazeHitData() const = 0;

	/**
	  * Knowledge about the monitor size when designing gaze interactions on desktop can be very useful since working in SI units can make code screen size invariant. Display information is also useful for foveation techniques both in desktop and XR.
	  *
	  * @returns				Information regarding the display associated with the currently active tracker.
	  */
	virtual const FTobiiDisplayInfo& GetDisplayInformation() const = 0;

	/************************************************************************/
	/* Head Tracker                                                         */
	/************************************************************************/
public:
	/**
	  * This gives you all the relevant data for head tracking
	  *
	  * @returns				Information regarding head position and orientation
	  */
	virtual const FTobiiHeadPoseData& GetHeadPoseData() const = 0;

	/************************************************************************/
	/* Desktop                                                              */
	/************************************************************************/
public:
	/**
	  * On desktop, the tracker can only track the user within a limited frustum.
	  *
	  * @returns				The vertices defining the tracking frustum in the Tobii User Coordinate Space.
	  */
	virtual const FTobiiDesktopTrackBox& GetDesktopTrackBox() const = 0;

	/**
	  * This is output from the Tobii Infinite screen algorithm. You ideally shouldn't use these angles by yourself, but instead use the camera modifier found in the interaction module.
	  *
	  * @returns				Infinite screen angles.
	  */
	virtual const FRotator& GetInfiniteScreenAngles() const = 0;
};
