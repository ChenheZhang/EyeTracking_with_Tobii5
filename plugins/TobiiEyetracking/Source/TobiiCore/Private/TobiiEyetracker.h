/******************************************************************************
* Copyright 2017- Tobii Technology AB. All rights reserved.
*
* @author Temaran | Fredrik Lindh | fredrik.lindh@tobii.com | https://github.com/Temaran
******************************************************************************/

#pragma once

#if TOBII_EYETRACKING_ACTIVE

#include "ITobiiEyeTracker.h"
#include "TobiiPlatformSpecific.h"
#include "TobiiInternalTypes.h"
#include "tobii_gameintegration.h"

#include "CoreMinimal.h"
#include "Containers/Ticker.h"
#include "GameFramework/PlayerController.h"
#include "Slate/SceneViewport.h"

/*
 * FTickerObjectBase here is ticked almost last in a frame.
 * The order seems to go 'IInputDevice', then shortly after that 'Actor/Components' and very late after that the 'TickerCore' and thereby also FTickerObjectBase.
 * No matter if we choose IInputDevice or FTickerObjectBase we will be working with complete physics data from the last frame.
 * Of course we would rather be working with current frame physics data, but doing that would make our data more TickGroup dependent.
 * Maybe that's fine, but seeing as our traces are not really precise anyways, I think the simplicity of design of using FTickerObjectBase wins out.
 *
 * The big alternative would be to move all GTOM related objects to an actor that belongs to the PostPhysics tick group together with the focus managers and introduce a tick dependency.
 * See FEngineLoop::Tick() for more detailed info.
 */
class FTobiiEyeTracker : public ITobiiEyeTracker, public FTickerObjectBase
{
public:
	FTobiiEyeTracker();
	virtual ~FTobiiEyeTracker();
	virtual bool Tick(float DeltaTime) override;
	void Shutdown();
	bool IsConnectedToEyeTracker();

	/************************************************************************/
	/* ITobiiEyetracker                                                     */
	/************************************************************************/
public:
	virtual const FTobiiGazeData& GetCombinedGazeData() const override;
	virtual const FTobiiGazeData& GetLeftGazeData() const override;
	virtual const FTobiiGazeData& GetRightGazeData() const override;
	virtual ETobiiGazeTrackerStatus GetGazeTrackerStatus() const override;
	virtual bool GetGazeTrackerCapability(ETobiiGazeTrackerCapability Capability) const override;

	virtual const FHitResult& GetCombinedWorldGazeHitData() const override;
	virtual const FHitResult& GetLeftWorldGazeHitData() const override;
	virtual const FHitResult& GetRightWorldGazeHitData() const override;
	virtual const FTobiiDisplayInfo& GetDisplayInformation() const override;
	virtual const FTobiiHeadPoseData& GetHeadPoseData() const override;
	virtual const FTobiiDesktopTrackBox& GetDesktopTrackBox() const override;
	virtual const FRotator& GetInfiniteScreenAngles() const override;

public:
	virtual void SetEyeTrackedPlayer(APlayerController* PlayerController) override;
	virtual bool IsStereoGazeDataAvailable() const override { return bIsXR; }
	virtual bool GetEyeTrackerGazeData(FEyeTrackerGazeData& OutGazeData) const override;
	virtual bool GetEyeTrackerStereoGazeData(FEyeTrackerStereoGazeData& OutGazeData) const override;
	virtual EEyeTrackerStatus GetEyeTrackerStatus() const override;

private:
	TobiiGameIntegration::ITobiiGameIntegrationApi* TgiApi;
	FTobiiPlatformNotifications PlatformNotifications;
	TWeakObjectPtr<APlayerController> ActivePlayerController;

	FHitResult CombinedWorldGazeHitData;
	FHitResult LeftWorldGazeHitData;
	FHitResult RightWorldGazeHitData;
	FTobiiGazeData LeftGazeData;
	FTobiiGazeData RightGazeData;
	FTobiiGazeData CombinedGazeData;
	ETobiiGazeTrackerStatus GazeTrackerStatus;
	FTobiiHeadPoseData HeadPoseData;
	FTobiiDisplayInfo DisplayInfo;
	FRotator InfiniteScreenAngles;
	FTobiiDesktopTrackBox DesktopTrackBox;

	FTobiiRawGazePoint RawGazePoint;
	FTobiiRawHeadPose RawHeadPose;
	FVector PrevCombinedGazeDirection;

	bool bIsXR;
	int64 GazeDataTimeStampMicroSecs;
	uint64 GazePointDeltaTimeMicroSecs;
	double CurrentAverageGazeAngularSpeedDegPerMicroSecs;
	FDateTime StartTime;

	void ResetData();
	bool UpdateLowLevelResources();
	void TickDesktop(float DeltaTime);
	void TickXR(float DeltaTime);
	void UpdateWorldSpaceData(float DeltaTime);
	void UpdateStabilityData(float DeltaTime);

	FVector2D ConvertRawGazePointUNormToGameViewportCoordinateUNorm(FSceneViewport* GameViewport, const FVector2D& InNormalizedPoint);

	FVector2D& TickEmulatedGazePointUNorm(float DeltaTime);
};

#endif //TOBII_EYETRACKING_ACTIVE
