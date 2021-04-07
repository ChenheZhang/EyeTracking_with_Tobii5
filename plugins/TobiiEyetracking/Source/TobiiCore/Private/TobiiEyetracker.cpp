/******************************************************************************
* Copyright 2017- Tobii Technology AB. All rights reserved.
*
* @author Temaran | Fredrik Lindh | fredrik.lindh@tobii.com | https://github.com/Temaran
******************************************************************************/

#include "TobiiEyetracker.h"
#include "TobiiBlueprintLibrary.h"

#include "DrawDebugHelpers.h"
#include "IXRTrackingSystem.h"
#include "Engine/Engine.h"
#include "Engine/Canvas.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/HUD.h"
#include "GameFramework/PlayerController.h"
#include "Slate/SceneViewport.h"
#include "Widgets/SWindow.h"
#include "Misc/App.h"

#include "tobii_gameintegration.h"

/************************************************************************/
/* Configuration CVars                                                  */
/************************************************************************/
static TAutoConsoleVariable<int32> CVarTobiiEnableEyetracking(TEXT("tobii.EnableEyetracking"), 1, TEXT("0 - Eyetracking is disabled. 1 - Eyetracking is enabled."));
static TAutoConsoleVariable<int32> CVarTobiiFreezingGazeData(TEXT("tobii.FreezeGazeData"), 0, TEXT("0 - Eyetracking data is not frozen. 1 - Eyetracking data is frozen. This is really useful for visualizing gaze and debugging."));
static TAutoConsoleVariable<int32> CVarTobiiFocusTraceChannel(TEXT("tobii.FocusTraceChannel"), (int32)ECC_Visibility, TEXT("This is the trace channel that will be used in all eye tracking queries."));
static TAutoConsoleVariable<float> CVarTobiiFovealConeAngleDegrees(TEXT("tobii.FovealConeAngleDegrees"), 5.0f, TEXT("A larger value here will lead to the GTOM system considering a larger area around the gaze point. Refer to this link to see what values are reasonable: https://en.wikipedia.org/wiki/Fovea_centralis#/media/File:Macula.svg. Eventually, we will hopefully select good values for you so you don't have to care about this."));
static TAutoConsoleVariable<float> CVarTobiiMaximumTraceDistance(TEXT("tobii.MaximumTraceDistance"), 5000.0f, TEXT("This is how far in front of the player we can detect objects. This could impact game play so be careful. Shorter range can also positively impact performance."));
static TAutoConsoleVariable<float> CVarTobiiMaximumAcceptableStableGazeAverageAngularSpeedDegPerMicroSecs(TEXT("tobii.MaximumAcceptableStableGazeAverageAngularSpeedDegPerMicroSecs"), 0.00001f, TEXT("Raising this value will make the system more generous when determining if a gaze point is currently stable or not."));
static TAutoConsoleVariable<float> CVarTobiiStablePointInterpolationBias(TEXT("tobii.StablePointInterpolationBias"), 0.6f, TEXT("Gaze point stability is based on a linear interpolation filter. This scalar determines the bias given to the new point."));

static TAutoConsoleVariable<float> CVarTobiiWatchingToNotWatchingInertiaSecs(TEXT("tobii.desktop.WatchingToNotWatchingInertiaSecs"), 0.6f, TEXT("To make the experience more stable we introduce inertia for transitioning from the UserPresentAndWatchingWindow eyetracking status state. This is the length of that inertia period in seconds. Please note that the eyetracker also has innate inertia, this is just a way to further control it."));
static TAutoConsoleVariable<float> CVarTobiiAdditionalWindowMarginCm(TEXT("tobii.desktop.AdditionalWindowMarginCm"), 1.3f, TEXT("We add an artificial margin around the game window to avoid precision and accuracy problems when the gaze point is close to the edge of the screen. The value is the size of this extra border in centimeters (same as unreal units)."));
static TAutoConsoleVariable<float> CVarTobiiHeadPosePositionInterpolationBias(TEXT("tobii.desktop.HeadPosePositionInterpolationBias"), 0.6f, TEXT("Head pose position filtration is done using a linear interpolation filter. This scalar determines the bias given to the new position data."));
static TAutoConsoleVariable<float> CVarTobiiHeadPoseRotationInterpolationBias(TEXT("tobii.desktop.HeadPoseRotationInterpolationBias"), 0.1f, TEXT("Head pose rotation filtration is done using a spherical linear interpolation filter. This scalar is used as the interpolation constant."));
static TAutoConsoleVariable<int32> CVarInfiniteScreenEnabled(TEXT("tobii.desktop.InfiniteScreenEnabled"), 1, TEXT("0 - Extended View is off.  1 - Extended View is on."));
static TAutoConsoleVariable<float> CVarExtendedViewMaxGazeAngleDeg(TEXT("tobii.desktop.ExtendedViewMaxGazeAngleDeg"), 30, TEXT("Extended View angles will not rotate the camera more than this amount of degrees due to the user's gaze."));
static TAutoConsoleVariable<float> CVarExtendedViewHeadSensitivity(TEXT("tobii.desktop.ExtendedViewHeadSensitivity"), 0.5f, TEXT("This is how sensitive the head component of extended view will be."));
static TAutoConsoleVariable<float> CVarExtendedViewGazeSensitivity(TEXT("tobii.desktop.ExtendedViewGazeSensitivity"), 0.5f, TEXT("This is how sensitive the gaze component of extended view will be."));
static TAutoConsoleVariable<float> CVarExtendedViewGazeOnlyScalar(TEXT("tobii.desktop.ExtendedViewGazeOnlyScalar"), 1.3f, TEXT("This is how we will scale the gaze extended view if only gaze is available."));
static TAutoConsoleVariable<float> CVarExtendedViewHeadOnlyScalar(TEXT("tobii.desktop.ExtendedViewHeadOnlyScalar"), 1.3f, TEXT("This is how we will scale the head extended view if only head is available."));

static TAutoConsoleVariable<float> CVarHMDScreenDistanceToEyeCm(TEXT("tobii.xr.HMDScreenDistanceToEyeCm"), 3.0f, TEXT("Since the foveal cone on the screen depends on the distance of the eye from the screen, we need this. Since UE4 doesn't provide APIs to poll it dynamically, you have to set it manually here instead."));
static TAutoConsoleVariable<int32> CVarTobiiApplyHMDOrientation(TEXT("tobii.xr.ApplyHMDOrientation"), 1, TEXT("Apply the HMD orientation to your gaze direction"));
static TAutoConsoleVariable<int32> CVarTobiiApplyActorRotation(TEXT("tobii.xr.ApplyActorRotation"), 1, TEXT("Apply your actor rotation to your gaze direction"));

static TAutoConsoleVariable<int32> CVarEnableEyetrackingEmulation(TEXT("tobii.emulation.EnableEyetrackingEmulation"), 0, TEXT("0 - Don't emulate eye tracking. 1 - Emulate eye tracking."));
static TAutoConsoleVariable<float> CVarEyetrackingEmulationGazeSpeed(TEXT("tobii.emulation.DesktopEyetrackingEmulationGazeSpeed"), 0.4f, TEXT("When emulating gaze on desktop, this is the speed scalar used for the gaze point."));

static TAutoConsoleVariable<int32> CVarEnableEyetrackingDebug(TEXT("tobii.debug"), 0, TEXT("0 - Eyetracking debug visualizations are disabled. 1 - Eyetracking debug visualizations are enabled."));
static TAutoConsoleVariable<int32> CVarEnableGazePointDebug(TEXT("tobii.debug.EnableGazePointDebug"), 1, TEXT("0 - Gaze point debug visualizations are disabled. 1 - Gaze point debug visualizations are enabled."));
static TAutoConsoleVariable<int32> CVarEnableHeadPoseDebug(TEXT("tobii.debug.EnableHeadPoseDebug"), 0, TEXT("0 - Head pose debug visualizations are disabled. 1 - Head pose debug visualizations are enabled."));

#if TOBII_EYETRACKING_ACTIVE

using namespace TobiiGameIntegration;

FTobiiEyeTracker::FTobiiEyeTracker()
	: TgiApi(nullptr)
	, ActivePlayerController(nullptr)
	, bIsXR(false)
{
	TgiApi = GetApi(TCHAR_TO_ANSI(FApp::GetProjectName()));
	StartTime = FDateTime::UtcNow();
	ResetData();
}

FTobiiEyeTracker::~FTobiiEyeTracker()
{
	Shutdown();
}

void FTobiiEyeTracker::Shutdown()
{
	if (TgiApi != nullptr)
	{
		TgiApi->Shutdown();
		TgiApi = nullptr;
	}

	GazeTrackerStatus = ETobiiGazeTrackerStatus::NotConnected;
}

bool FTobiiEyeTracker::IsConnectedToEyeTracker()
{
	if (TgiApi != nullptr)
	{
		IStreamsProvider* StreamsProvider = TgiApi->GetStreamsProvider();
		ITrackerController* TrackerController = TgiApi->GetTrackerController();
		if (TrackerController != nullptr && StreamsProvider != nullptr)
		{
			return true;
		}
	}
	
	return false;
}

void FTobiiEyeTracker::ResetData()
{
	ActivePlayerController.Reset();
	CombinedWorldGazeHitData = FHitResult();
	LeftWorldGazeHitData= FHitResult();
	RightWorldGazeHitData = FHitResult();
	LeftGazeData = FTobiiGazeData();
	RightGazeData = FTobiiGazeData();
	CombinedGazeData = FTobiiGazeData();
	GazeTrackerStatus = ETobiiGazeTrackerStatus::NotConnected;
	HeadPoseData = FTobiiHeadPoseData();
	DisplayInfo = FTobiiDisplayInfo();

	GazePointDeltaTimeMicroSecs = 0;
	CurrentAverageGazeAngularSpeedDegPerMicroSecs = 0.0;
}

bool FTobiiEyeTracker::Tick(float DeltaTime)
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_TobiiEyetracking_Tick);

	if (GEngine->EyeTrackingDevice.Get() != this)
	{
		Shutdown();
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// Update status
	//////////////////////////////////////////////////////////////////////////
	if (!CVarTobiiEnableEyetracking.GetValueOnGameThread())
	{
		if (GazeTrackerStatus != ETobiiGazeTrackerStatus::Disabled)
		{
			ResetData();
			GazeTrackerStatus = ETobiiGazeTrackerStatus::Disabled;
		}

		return true;
	}

	if (GEngine == nullptr
		|| GEngine->GameViewport == nullptr
		|| GEngine->GameViewport->GetGameViewport() == nullptr
		|| TgiApi == nullptr)
	{
		ResetData();
		return true;
	}

	if (!UpdateLowLevelResources())
	{
		ResetData();
		return true;
	}

	//If we have no active PC, set default
	if (!ActivePlayerController.IsValid())
	{
		SetEyeTrackedPlayer(nullptr);
	}

	if (!CVarTobiiFreezingGazeData.GetValueOnGameThread())
	{
		if (bIsXR)
		{
			TickXR(DeltaTime);
		}
		else
		{
			TickDesktop(DeltaTime);
		}
	}

	UpdateWorldSpaceData(DeltaTime);
	UpdateStabilityData(DeltaTime);

	if (ActivePlayerController.IsValid()
		&& ActivePlayerController->GetWorld() != nullptr
		&& CVarEnableEyetrackingDebug.GetValueOnGameThread())
	{
		if (CVarEnableGazePointDebug.GetValueOnGameThread())
		{
			UE_LOG(LogTemp, Warning, TEXT("Gaze Direction: (%.2f, %.2f, %.2f)"), CombinedGazeData.WorldGazeDirection.X, CombinedGazeData.WorldGazeDirection.Y, CombinedGazeData.WorldGazeDirection.Z);
			const float DrawSize = FVector::Dist(CombinedGazeData.WorldGazeOrigin, CombinedWorldGazeHitData.Location) * FMath::Tan(FMath::DegreesToRadians(CombinedGazeData.WorldGazeConeAngleDegrees));
			DrawDebugSphere(ActivePlayerController->GetWorld(), CombinedWorldGazeHitData.Location, DrawSize, 16, CombinedGazeData.bIsStable ? FColor::Green : FColor::Red, false, 0.0f);
		}

		if (CVarEnableHeadPoseDebug.GetValueOnGameThread())
		{
			UE_LOG(LogTemp, Warning, TEXT("Head location: (%.2f, %.2f, %.2f) ~~ Head orientation: (%.2f, %.2f, %.2f)")
				, HeadPoseData.HeadLocation.X, HeadPoseData.HeadLocation.Y, HeadPoseData.HeadLocation.Z
			    , HeadPoseData.HeadOrientation.Yaw, HeadPoseData.HeadOrientation.Pitch, HeadPoseData.HeadOrientation.Roll);
		}
	}

	return true;
}

bool FTobiiEyeTracker::UpdateLowLevelResources()
{
	//Get game window and monitor handles
	if (DisplayInfo.GameMonitorHandle == nullptr || PlatformNotifications.bShouldUpdateGameMonitorHandle)
	{
		PlatformNotifications.bShouldUpdateGameMonitorHandle = false;

		if (!GEngine->GameViewport->GetWindow().IsValid())
		{
			return false;
		}
		SWindow* GameWindow = GEngine->GameViewport->GetWindow().Get();
		if (!GameWindow->GetNativeWindow().IsValid())
		{
			return false;
		}

		DisplayInfo.DpiScale = GameWindow->GetCachedGeometry().Scale;
		DisplayInfo.GameWindowHandle = GameWindow->GetNativeWindow().Get()->GetOSWindowHandle();
		if (DisplayInfo.GameWindowHandle == nullptr)
		{
			return false;
		}

		DisplayInfo.GameMonitorHandle = FTobiiPlatformSpecific::GetMonitorInformation(DisplayInfo.GameWindowHandle, DisplayInfo.MonitorWidthPx, DisplayInfo.MonitorHeightPx);
	}
	
	//Since the viewport size can change every frame, we cannot treat this data as constant.
	IStreamsProvider* StreamsProvider = TgiApi->GetStreamsProvider();
	if (StreamsProvider != nullptr)
	{
		GazePoint MaxGazeUNorm, MaxGazeMm;
		MaxGazeUNorm.X = MaxGazeUNorm.Y = 1.0f;
		StreamsProvider->ConvertGazePoint(MaxGazeUNorm, MaxGazeMm, Normalized, Mm);

		DisplayInfo.MonitorWidthCm = MaxGazeMm.X / 10.0f;
		DisplayInfo.MonitorHeightCm = MaxGazeMm.Y / 10.0f;

		FVector2D ViewportSize;
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		DisplayInfo.MainViewportWidthPx = ViewportSize.X;
		DisplayInfo.MainViewportHeightPx = ViewportSize.Y;
		DisplayInfo.MainViewportWidthCm = (((float)DisplayInfo.MainViewportWidthPx / (float)DisplayInfo.MonitorWidthPx) * DisplayInfo.MonitorWidthCm);
		DisplayInfo.MainViewportHeightCm = (((float)DisplayInfo.MainViewportHeightPx / (float)DisplayInfo.MonitorHeightPx) * DisplayInfo.MonitorHeightCm);
	}

	return true;
}

FVector2D& FTobiiEyeTracker::TickEmulatedGazePointUNorm(float DeltaTime)
{
	static FVector2D EmulatedGazeNorm(0.5f, 0.5f);

	if (ActivePlayerController.IsValid())
	{
		const float EmulatedGazeSpeed = CVarEyetrackingEmulationGazeSpeed.GetValueOnGameThread();
		if (ActivePlayerController->IsInputKeyDown(FKey("Up")))
		{
			EmulatedGazeNorm.Y -= DeltaTime * EmulatedGazeSpeed;
		}
		else if (ActivePlayerController->IsInputKeyDown(FKey("Down")))
		{
			EmulatedGazeNorm.Y += DeltaTime * EmulatedGazeSpeed;
		}

		if (ActivePlayerController->IsInputKeyDown(FKey("Left")))
		{
			EmulatedGazeNorm.X -= DeltaTime * EmulatedGazeSpeed;
		}
		else if (ActivePlayerController->IsInputKeyDown(FKey("Right")))
		{
			EmulatedGazeNorm.X += DeltaTime * EmulatedGazeSpeed;
		}
	}

	EmulatedGazeNorm.X = FMath::Clamp(EmulatedGazeNorm.X, 0.0f, 1.0f);
	EmulatedGazeNorm.Y = FMath::Clamp(EmulatedGazeNorm.Y, 0.0f, 1.0f);

	return EmulatedGazeNorm;
}

void FTobiiEyeTracker::TickDesktop(float DeltaTime)
{
	//Pump API
	ITrackerController* TrackerController = TgiApi->GetTrackerController();
	if (TrackerController != nullptr)
	{
		TrackerController->TrackWindow(DisplayInfo.GameWindowHandle);
	}
	TgiApi->Update();

	FDateTime Now = FDateTime::UtcNow();
	IStreamsProvider* StreamsProvider = nullptr;
	bool bIsEmulating = false;
	if (CVarEnableEyetrackingEmulation.GetValueOnGameThread())
	{
		bIsEmulating = true;
		GazeTrackerStatus = ETobiiGazeTrackerStatus::UserPresent;
	}
	else
	{
		//Test for status
		StreamsProvider = TgiApi->GetStreamsProvider();
		if (StreamsProvider != nullptr && TrackerController != nullptr && TrackerController->IsConnected())
		{
			if (GazeTrackerStatus < ETobiiGazeTrackerStatus::UserNotPresent)
			{
				GazeTrackerStatus = ETobiiGazeTrackerStatus::UserNotPresent;
			}
		}
		else
		{
			GazeTrackerStatus = ETobiiGazeTrackerStatus::NotConnected;
		}
	}

	if (GazeTrackerStatus >= ETobiiGazeTrackerStatus::UserNotPresent)
	{
		if (bIsEmulating)
		{
			RawGazePoint.TimeStamp = Now;
			RawGazePoint.GazePointNormalized = TickEmulatedGazePointUNorm(DeltaTime);
			HeadPoseData.HeadLocation = FVector::ZeroVector;
			HeadPoseData.HeadOrientation = FRotator::ZeroRotator;
			InfiniteScreenAngles = FRotator::ZeroRotator;
		}
		else
		{
			GazeTrackerStatus = StreamsProvider->IsPresent() ? ETobiiGazeTrackerStatus::UserPresent : ETobiiGazeTrackerStatus::UserNotPresent;

			//Get new gaze data
			const GazePoint* GazePointsSinceLastUpdateSNorm;
			int NumGazePointsSinceLastUpdate = StreamsProvider->GetGazePoints(GazePointsSinceLastUpdateSNorm);
			if (NumGazePointsSinceLastUpdate > 0)
			{
				RawGazePoint.GazePointNormalized.Set(0.0f, 0.0f);
				for (int32 GazeIdx = 0; GazeIdx < NumGazePointsSinceLastUpdate; GazeIdx++)
				{
					RawGazePoint.GazePointNormalized += FVector2D(GazePointsSinceLastUpdateSNorm[GazeIdx].X, GazePointsSinceLastUpdateSNorm[GazeIdx].Y);
				}

				RawGazePoint.TimeStamp = Now;
				RawGazePoint.GazePointNormalized /= (float)NumGazePointsSinceLastUpdate;

				//Convert to UNorm
				RawGazePoint.GazePointNormalized.Set((RawGazePoint.GazePointNormalized.X + 1.0f) / 2.0f
					, (-RawGazePoint.GazePointNormalized.Y + 1.0f) / 2.0f);
			}

			//Get new head pose data
			const HeadPose* HeadPosesSinceLastUpdate;
			int NumHeadPosesSinceLastUpdate = StreamsProvider->GetHeadPoses(HeadPosesSinceLastUpdate);
			if (NumHeadPosesSinceLastUpdate > 0)
			{
				RawHeadPose.HeadPositionCm.Set(0.0f, 0.0f, 0.0f);
				float RawPitchRad = 0.0f;
				float RawYawRad = 0.0f;
				float RawRollRad = 0.0f;

				for (int32 HeadPoseIdx = 0; HeadPoseIdx < NumHeadPosesSinceLastUpdate; HeadPoseIdx++)
				{
					RawHeadPose.HeadPositionCm += FVector(HeadPosesSinceLastUpdate[HeadPoseIdx].Position.X, HeadPosesSinceLastUpdate[HeadPoseIdx].Position.Y, HeadPosesSinceLastUpdate[HeadPoseIdx].Position.Z);
					RawPitchRad += HeadPosesSinceLastUpdate[HeadPoseIdx].Rotation.Pitch;
					RawYawRad += HeadPosesSinceLastUpdate[HeadPoseIdx].Rotation.Yaw;
					RawRollRad += HeadPosesSinceLastUpdate[HeadPoseIdx].Rotation.Roll;
				}

				RawHeadPose.HeadPositionCm /= (float)NumHeadPosesSinceLastUpdate;
				RawPitchRad /= (float)NumHeadPosesSinceLastUpdate;
				RawYawRad /= (float)NumHeadPosesSinceLastUpdate;
				RawRollRad /= (float)NumHeadPosesSinceLastUpdate;

				RawHeadPose.TimeStamp = Now;
				RawHeadPose.HeadPositionCm /= 10.0f; //Unit conversion
				RawHeadPose.HeadOrientation = FRotator(FMath::RadiansToDegrees(RawPitchRad), FMath::RadiansToDegrees(RawYawRad), FMath::RadiansToDegrees(RawRollRad));
			}

			//Calculate derived data
			const float HeadPosePositionInterpolationBias = FMath::Clamp(CVarTobiiHeadPosePositionInterpolationBias.GetValueOnGameThread(), 0.3f, 1.0f);
			const float HeadPoseRotationInterpolationBias = FMath::Clamp(CVarTobiiHeadPoseRotationInterpolationBias.GetValueOnGameThread(), 0.01f, 1.0f);

			HeadPoseData.HeadLocation = FMath::LerpStable(HeadPoseData.HeadLocation, RawHeadPose.HeadPositionCm, HeadPosePositionInterpolationBias);
			HeadPoseData.HeadOrientation = FQuat::Slerp(HeadPoseData.HeadOrientation.Quaternion(), RawHeadPose.HeadOrientation.Quaternion(), HeadPoseRotationInterpolationBias).Rotator();

			//Infinite screen
			bool bExtendedViewUpdateSuccessful = false;
			IFeatures* Features = TgiApi->GetFeatures();
			if (CVarInfiniteScreenEnabled.GetValueOnGameThread() && !bIsXR && Features != nullptr)
			{
				IExtendedView* ExtendedView = Features->GetExtendedView();
				if (ExtendedView != nullptr)
				{
					const float BaseHeadViewResponsiveness = FMath::Clamp(CVarExtendedViewHeadSensitivity.GetValueOnGameThread(), 0.0f, 1.0f);
					const float BaseGazeViewResponsiveness = FMath::Clamp(CVarExtendedViewGazeSensitivity.GetValueOnGameThread(), 0.0f, 1.0f);

					ExtendedViewSettings Settings;
					Settings.NormalizedGazeViewMinimumExtensionAngle = Settings.NormalizedGazeViewExtensionAngle = CVarExtendedViewMaxGazeAngleDeg.GetValueOnGameThread() / 360.0f;
					Settings.HeadViewResponsiveness = BaseHeadViewResponsiveness;
					Settings.GazeViewResponsiveness = BaseGazeViewResponsiveness;
					Settings.HeadViewAutoCenter.IsEnabled = false;

					//Update default settings
					ExtendedView->UpdateSettings(Settings);

					//Update specific settings
					Settings.GazeViewResponsiveness = BaseGazeViewResponsiveness * CVarExtendedViewGazeOnlyScalar.GetValueOnGameThread();
					ExtendedView->UpdateGazeOnlySettings(Settings);
					Settings.HeadViewResponsiveness = BaseHeadViewResponsiveness * CVarExtendedViewHeadOnlyScalar.GetValueOnGameThread();
					ExtendedView->UpdateHeadOnlySettings(Settings);

					Transformation ExtendedViewData = ExtendedView->GetTransformation();
					InfiniteScreenAngles.Roll = 0.0f;
					InfiniteScreenAngles.Yaw = ExtendedViewData.Rotation.Yaw;
					InfiniteScreenAngles.Pitch = ExtendedViewData.Rotation.Pitch;
					bExtendedViewUpdateSuccessful = true;
				}
			}
			if (!bExtendedViewUpdateSuccessful)
			{
				InfiniteScreenAngles = FRotator::ZeroRotator;
			}
		}

		//Since these are not time based, only run when necessary.
		static FDateTime PreviousRawGazePointTime;
		if (PreviousRawGazePointTime != RawGazePoint.TimeStamp)
		{
			GazePointDeltaTimeMicroSecs = (uint64)FMath::Max((RawGazePoint.TimeStamp - PreviousRawGazePointTime).GetTotalMicroseconds(), 0.0);
			CombinedGazeData.TimeStamp = RawGazePoint.TimeStamp;
			FVector2D ScreenSpaceGazePointUNorm = ConvertRawGazePointUNormToGameViewportCoordinateUNorm(GEngine->GameViewport->GetGameViewport(), RawGazePoint.GazePointNormalized);
			CombinedGazeData.ScreenGazePointPx.Set(ScreenSpaceGazePointUNorm.X * DisplayInfo.MainViewportWidthPx, ScreenSpaceGazePointUNorm.Y * DisplayInfo.MainViewportHeightPx);

			PreviousRawGazePointTime = RawGazePoint.TimeStamp;
		}
		GazeDataTimeStampMicroSecs = (PreviousRawGazePointTime - StartTime).GetTotalMicroseconds();

		const float AspectRatio = DisplayInfo.MainViewportWidthPx / (float)DisplayInfo.MainViewportHeightPx;
		const float FovealRegionSizeDeg = FMath::Max(CVarTobiiFovealConeAngleDegrees.GetValueOnGameThread(), 0.0f);
		CombinedGazeData.ScreenGazeCircleRadiiPx.Y = FEyetrackingUtils::CalculateFovealRegionHeightPx(DisplayInfo.MainViewportHeightCm, DisplayInfo.MainViewportHeightPx, HeadPoseData.HeadLocation.Z, FovealRegionSizeDeg);
		CombinedGazeData.ScreenGazeCircleRadiiPx.X = CombinedGazeData.ScreenGazeCircleRadiiPx.Y * AspectRatio;
		CombinedGazeData.WorldGazeConeAngleDegrees = FovealRegionSizeDeg;
		CombinedGazeData.EyeOpenness = 1.0f;
	}
}

void FTobiiEyeTracker::TickXR(float DeltaTime)
{
	//Pump API
	ITrackerController* TrackerController = TgiApi->GetTrackerController();
	if (TrackerController != nullptr)
	{
		TrackerController->TrackHMD();
	}
	TgiApi->Update();

	//Test for status
	FDateTime Now = FDateTime::UtcNow();
	IStreamsProvider* StreamsProvider = nullptr;
	bool bIsEmulating = false;
	if (CVarEnableEyetrackingEmulation.GetValueOnGameThread())
	{
		bIsEmulating = true;
		GazeTrackerStatus = ETobiiGazeTrackerStatus::UserPresent;
	}
	else
	{
		StreamsProvider = TgiApi->GetStreamsProvider();
		GazeTrackerStatus = StreamsProvider != nullptr && TrackerController != nullptr && TrackerController->IsConnected()
			? ETobiiGazeTrackerStatus::UserPresent
			: ETobiiGazeTrackerStatus::NotConnected;
	}

	if (GazeTrackerStatus == ETobiiGazeTrackerStatus::UserPresent && ActivePlayerController.IsValid())
	{
		bool bNewGazeData = false;

		if (bIsEmulating)
		{
			FQuat HMDOrientation;
			FVector HMDPosition;
			GEngine->XRSystem->GetCurrentPose(IXRTrackingSystem::HMDDeviceId, HMDOrientation, HMDPosition);

			FVector2D& GazeModifierUNorm = TickEmulatedGazePointUNorm(DeltaTime);
			FVector2D GazeModifierSNorm((GazeModifierUNorm.X - 0.5f) * 2.0f, (GazeModifierUNorm.Y - 0.5f) * 2.0f);
			HMDOrientation *= FQuat::MakeFromEuler(FVector(0.0f, -GazeModifierSNorm.Y * 90.0f, GazeModifierSNorm.X * 90.0f));
			LeftGazeData.WorldGazeDirection = RightGazeData.WorldGazeDirection = HMDOrientation.GetForwardVector();

			LeftGazeData.bIsGazeDataValid = RightGazeData.bIsGazeDataValid = true;
			LeftGazeData.EyeOpenness = RightGazeData.EyeOpenness = 1.0f;
			bNewGazeData = true;
		}
		else
		{
			//Find direction and openness
			//const HMDGaze* RawTobiiGazeData;
			HMDGaze RawTobiiGazeData;
			//int NumDataSinceLastUpdate = StreamsProvider->GetHMDGaze(RawTobiiGazeData);
			int NumDataSinceLastUpdate = StreamsProvider->GetLatestHMDGaze(RawTobiiGazeData) ? 1 : 0;
			if (NumDataSinceLastUpdate > 0)
			{
				const HMDGaze& LatestGazeData = RawTobiiGazeData;//[NumDataSinceLastUpdate - 1];
				LeftGazeData.bIsGazeDataValid = (LatestGazeData.Validity & HMDValidityFlags::LeftEyeIsValid) == HMDValidityFlags::LeftEyeIsValid;
				RightGazeData.bIsGazeDataValid = (LatestGazeData.Validity & HMDValidityFlags::RightEyeIsValid) == HMDValidityFlags::RightEyeIsValid;

				LeftGazeData.WorldGazeDirection.Set(0.0f, 0.0f, 0.0f);
				RightGazeData.WorldGazeDirection.Set(0.0f, 0.0f, 0.0f);
				for (int32 GazeIdx = 0; GazeIdx < NumDataSinceLastUpdate; GazeIdx++)
				{
					const HMDGaze& GazeData = RawTobiiGazeData;// [GazeIdx];
					LeftGazeData.WorldGazeDirection += FVector(GazeData.LeftEyeInfo.GazeDirection.Z, -GazeData.LeftEyeInfo.GazeDirection.X, GazeData.LeftEyeInfo.GazeDirection.Y);
					RightGazeData.WorldGazeDirection += FVector(GazeData.RightEyeInfo.GazeDirection.Z, -GazeData.RightEyeInfo.GazeDirection.X, GazeData.RightEyeInfo.GazeDirection.Y);
				}
				LeftGazeData.bIsGazeDataValid = LeftGazeData.bIsGazeDataValid && LeftGazeData.WorldGazeDirection.Normalize();
				RightGazeData.bIsGazeDataValid = RightGazeData.bIsGazeDataValid && RightGazeData.WorldGazeDirection.Normalize();

				LeftGazeData.EyeOpenness = LatestGazeData.LeftEyeInfo.EyeOpenness;
				RightGazeData.EyeOpenness = LatestGazeData.RightEyeInfo.EyeOpenness;
				GazePointDeltaTimeMicroSecs = (int64)LatestGazeData.Timestamp - GazeDataTimeStampMicroSecs;
				GazeDataTimeStampMicroSecs = (int64)LatestGazeData.Timestamp;
				bNewGazeData = true;
			}

			//Optionally correct orientation depending on project settings
			{
				if (CVarTobiiApplyHMDOrientation.GetValueOnGameThread())
				{
					FQuat HMDOrientation;
					FVector HMDPosition;
					GEngine->XRSystem->GetCurrentPose(IXRTrackingSystem::HMDDeviceId, HMDOrientation, HMDPosition);
					LeftGazeData.WorldGazeDirection = HMDOrientation.RotateVector(LeftGazeData.WorldGazeDirection);
					RightGazeData.WorldGazeDirection = HMDOrientation.RotateVector(RightGazeData.WorldGazeDirection);
				}
			}
		}

		if (CVarTobiiApplyActorRotation.GetValueOnGameThread()
			&& ActivePlayerController.IsValid()
			&& ActivePlayerController->GetPawn() != nullptr)
		{
			FQuat ActorRotation = ActivePlayerController->GetPawn()->GetActorRotation().Quaternion();

			LeftGazeData.WorldGazeDirection = ActorRotation.RotateVector(LeftGazeData.WorldGazeDirection);
			RightGazeData.WorldGazeDirection = ActorRotation.RotateVector(RightGazeData.WorldGazeDirection);
		}

		//Calculate combined data
		CombinedGazeData.bIsGazeDataValid = LeftGazeData.bIsGazeDataValid || RightGazeData.bIsGazeDataValid;
		if (LeftGazeData.bIsGazeDataValid && RightGazeData.bIsGazeDataValid)
		{
			CombinedGazeData.WorldGazeDirection = (LeftGazeData.WorldGazeDirection + RightGazeData.WorldGazeDirection);
			CombinedGazeData.bIsGazeDataValid = CombinedGazeData.bIsGazeDataValid && CombinedGazeData.WorldGazeDirection.Normalize();
		}
		else if (LeftGazeData.bIsGazeDataValid)
		{
			CombinedGazeData.WorldGazeDirection = LeftGazeData.WorldGazeDirection;
		}
		else if (RightGazeData.bIsGazeDataValid)
		{
			CombinedGazeData.WorldGazeDirection = RightGazeData.WorldGazeDirection;
		}

		if (bNewGazeData)
		{
			CombinedGazeData.TimeStamp = Now;
		}

		const float AspectRatio = DisplayInfo.MainViewportWidthPx / (float)DisplayInfo.MainViewportHeightPx;
		const float FovealRegionSizeDeg = FMath::Max(CVarTobiiFovealConeAngleDegrees.GetValueOnGameThread(), 0.0f);
		CombinedGazeData.ScreenGazeCircleRadiiPx.Y = FEyetrackingUtils::CalculateFovealRegionHeightPx(DisplayInfo.MainViewportHeightCm, DisplayInfo.MainViewportHeightPx, CVarHMDScreenDistanceToEyeCm.GetValueOnGameThread(), FovealRegionSizeDeg);
		CombinedGazeData.ScreenGazeCircleRadiiPx.X = CombinedGazeData.ScreenGazeCircleRadiiPx.Y * AspectRatio;
		LeftGazeData.WorldGazeConeAngleDegrees = RightGazeData.WorldGazeConeAngleDegrees = CombinedGazeData.WorldGazeConeAngleDegrees = FovealRegionSizeDeg;
		LeftGazeData.ScreenGazeCircleRadiiPx = RightGazeData.ScreenGazeCircleRadiiPx = CombinedGazeData.ScreenGazeCircleRadiiPx;
	}
}


void FTobiiEyeTracker::UpdateWorldSpaceData(float DeltaTime)
{
	if (ActivePlayerController.IsValid() && ActivePlayerController->GetWorld() != nullptr)
	{
		if (bIsXR)
		{
			ULocalPlayer* const LocalPlayer = ActivePlayerController->GetLocalPlayer();
			if (LocalPlayer != nullptr)
			{
				//Find origin and screen point. This also calculates screen data for VR as it is expressed in world space data.
				//@TODO: We need to make it obvious that if you want to line trace here, you must move the origin forward to the near plane.
				const float ProjectionDistanceCm = 1000.0f;
				FSceneViewProjectionData LeftProjectionData, RightProjectionData;
				if (LocalPlayer->GetProjectionData(LocalPlayer->ViewportClient->Viewport, eSSP_LEFT_EYE, /*out*/ LeftProjectionData))
				{
					LeftGazeData.WorldGazeOrigin = LeftProjectionData.ViewOrigin;
					const FVector ProjectionPoint = LeftGazeData.WorldGazeOrigin + (LeftGazeData.WorldGazeDirection * ProjectionDistanceCm);
					ActivePlayerController->ProjectWorldLocationToScreen(ProjectionPoint, LeftGazeData.ScreenGazePointPx);
				}
				else if (ActivePlayerController->PlayerCameraManager != nullptr)
				{
					LeftGazeData.WorldGazeOrigin = ActivePlayerController->PlayerCameraManager->GetCameraLocation();
				}
				if (LocalPlayer->GetProjectionData(LocalPlayer->ViewportClient->Viewport, eSSP_RIGHT_EYE, /*out*/ RightProjectionData))
				{
					RightGazeData.WorldGazeOrigin = RightProjectionData.ViewOrigin;
					const FVector ProjectionPoint = RightGazeData.WorldGazeOrigin + (RightGazeData.WorldGazeDirection * ProjectionDistanceCm);
					ActivePlayerController->ProjectWorldLocationToScreen(ProjectionPoint, RightGazeData.ScreenGazePointPx);
				}
				else if (ActivePlayerController->PlayerCameraManager != nullptr)
				{
					RightGazeData.WorldGazeOrigin = ActivePlayerController->PlayerCameraManager->GetCameraLocation();
				}

				CombinedGazeData.WorldGazeOrigin = (LeftGazeData.WorldGazeOrigin + RightGazeData.WorldGazeOrigin) / 2.0f;
				CombinedGazeData.ScreenGazePointPx = (LeftGazeData.ScreenGazePointPx + RightGazeData.ScreenGazePointPx) / 2.0f;
			}
		}
		else
		{
			CombinedGazeData.bIsGazeDataValid = ActivePlayerController->DeprojectScreenPositionToWorld(CombinedGazeData.ScreenGazePointPx.X, CombinedGazeData.ScreenGazePointPx.Y, CombinedGazeData.WorldGazeOrigin, CombinedGazeData.WorldGazeDirection);
			CombinedGazeData.WorldGazeOrigin = ActivePlayerController->PlayerCameraManager->GetCameraLocation();
			LeftGazeData = RightGazeData = CombinedGazeData;
		}

		//Verify direction validity
		if (!CombinedGazeData.WorldGazeDirection.IsNormalized())
		{
			CombinedGazeData.bIsGazeDataValid = false;
		}
		if (!LeftGazeData.WorldGazeDirection.IsNormalized())
		{
			LeftGazeData.bIsGazeDataValid = false;
		}
		if (!RightGazeData.WorldGazeDirection.IsNormalized())
		{
			RightGazeData.bIsGazeDataValid = false;
		}

		FCollisionQueryParams CollisionQueryParams;
		CollisionQueryParams.AddIgnoredActor(ActivePlayerController.Get());
		CollisionQueryParams.AddIgnoredActor(ActivePlayerController->GetPawn());
		const float MaximumTraceDistance = FMath::Max(CVarTobiiMaximumTraceDistance.GetValueOnGameThread(), 0.0f);
		const FVector CombinedGazeFarLocation = CombinedGazeData.WorldGazeOrigin + (CombinedGazeData.WorldGazeDirection * MaximumTraceDistance);
		if (!CombinedGazeData.bIsGazeDataValid ||
			!ActivePlayerController->GetWorld()->LineTraceSingleByChannel(CombinedWorldGazeHitData, CombinedGazeData.WorldGazeOrigin
			, CombinedGazeFarLocation, (ECollisionChannel)CVarTobiiFocusTraceChannel.GetValueOnGameThread(), CollisionQueryParams))
		{
			CombinedWorldGazeHitData.Actor = nullptr;
			CombinedWorldGazeHitData.Component = nullptr;
			CombinedWorldGazeHitData.Distance = MaximumTraceDistance;
			CombinedWorldGazeHitData.Location = CombinedGazeFarLocation;
			CombinedWorldGazeHitData.bBlockingHit = false;
		}

		const FVector LeftGazeFarLocation = LeftGazeData.WorldGazeOrigin + (LeftGazeData.WorldGazeDirection * MaximumTraceDistance);
		if (!LeftGazeData.bIsGazeDataValid ||
			!ActivePlayerController->GetWorld()->LineTraceSingleByChannel(LeftWorldGazeHitData, LeftGazeData.WorldGazeOrigin
			, LeftGazeFarLocation, (ECollisionChannel)CVarTobiiFocusTraceChannel.GetValueOnGameThread(), CollisionQueryParams))
		{
			LeftWorldGazeHitData.Actor = nullptr;
			LeftWorldGazeHitData.Component = nullptr;
			LeftWorldGazeHitData.Distance = MaximumTraceDistance;
			LeftWorldGazeHitData.Location = CombinedGazeFarLocation;
			LeftWorldGazeHitData.bBlockingHit = false;
		}

		const FVector RightGazeFarLocation = RightGazeData.WorldGazeOrigin + (RightGazeData.WorldGazeDirection * MaximumTraceDistance);
		if (!RightGazeData.bIsGazeDataValid ||
			!ActivePlayerController->GetWorld()->LineTraceSingleByChannel(RightWorldGazeHitData, RightGazeData.WorldGazeOrigin
			, RightGazeFarLocation, (ECollisionChannel)CVarTobiiFocusTraceChannel.GetValueOnGameThread(), CollisionQueryParams))
		{
			RightWorldGazeHitData.Actor = nullptr;
			RightWorldGazeHitData.Component = nullptr;
			RightWorldGazeHitData.Distance = MaximumTraceDistance;
			RightWorldGazeHitData.Location = CombinedGazeFarLocation;
			RightWorldGazeHitData.bBlockingHit = false;
		}
	}
}

void FTobiiEyeTracker::UpdateStabilityData(float DeltaTime)
{
	if (CVarEnableEyetrackingEmulation.GetValueOnGameThread() && bIsXR)
	{
		CombinedGazeData.bIsStable = true;
	}
	else
	{
		//We will calculate the average gaze point velocity using a very simple moving cumulative average.
		//If the average velocity of these points is under some predetermined threshold, we will consider it stable.
		if (GazePointDeltaTimeMicroSecs > 0)
		{
			const float GazeAngleDiffDeg = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(CombinedGazeData.WorldGazeDirection, PrevCombinedGazeDirection)));
			const double AngularSpeedDegPerMicroSecs = GazeAngleDiffDeg / (double)GazePointDeltaTimeMicroSecs;
			const float StablePointInterpolationBias = FMath::Clamp(CVarTobiiStablePointInterpolationBias.GetValueOnGameThread(), 0.3f, 1.0f);
			const float NewAvgSpeed = FMath::LerpStable(CurrentAverageGazeAngularSpeedDegPerMicroSecs, AngularSpeedDegPerMicroSecs, StablePointInterpolationBias);
			CurrentAverageGazeAngularSpeedDegPerMicroSecs = FMath::IsFinite(NewAvgSpeed) ? NewAvgSpeed : 0.0f; //Protect from overflow

			PrevCombinedGazeDirection = CombinedGazeData.WorldGazeDirection;

			const float MaxAcceptableStablePointAverageSpeed = FMath::Max(CVarTobiiMaximumAcceptableStableGazeAverageAngularSpeedDegPerMicroSecs.GetValueOnGameThread(), 0.0f);
			CombinedGazeData.bIsStable = CurrentAverageGazeAngularSpeedDegPerMicroSecs <= MaxAcceptableStablePointAverageSpeed;
		}
	}

	LeftGazeData.bIsStable = RightGazeData.bIsStable = CombinedGazeData.bIsStable;
}

void FTobiiEyeTracker::SetEyeTrackedPlayer(APlayerController* PlayerController)
{
	if (PlayerController != nullptr)
	{
		ActivePlayerController = PlayerController;
	}
	else if (GEngine != nullptr && GEngine->GameViewport != nullptr)
	{
		ActivePlayerController = GEngine->GetFirstLocalPlayerController(GEngine->GameViewport->GetWorld());
	}

	bIsXR = UTobiiBlueprintLibrary::IsXRPlayerController(ActivePlayerController.Get());
}

/************************************************************************/
/* IGazeTracker                                                         */
/************************************************************************/
const FTobiiGazeData& FTobiiEyeTracker::GetCombinedGazeData() const
{
	return CombinedGazeData;
}

const FTobiiGazeData& FTobiiEyeTracker::GetLeftGazeData() const
{
	return LeftGazeData;
}

const FTobiiGazeData& FTobiiEyeTracker::GetRightGazeData() const
{
	return RightGazeData;
}

ETobiiGazeTrackerStatus FTobiiEyeTracker::GetGazeTrackerStatus() const
{
	return GazeTrackerStatus;
}

bool FTobiiEyeTracker::GetGazeTrackerCapability(ETobiiGazeTrackerCapability Capability) const
{
	//Desktop trackers only support CombinedGazeData
	return bIsXR || Capability == ETobiiGazeTrackerCapability::CombinedGazeData;
}

/************************************************************************/
/* IHeadTracker                                                         */
/************************************************************************/
const FTobiiHeadPoseData& FTobiiEyeTracker::GetHeadPoseData() const
{
	return HeadPoseData;
}

/************************************************************************/
/* ITobiiEyetracker                                                     */
/************************************************************************/
const FHitResult& FTobiiEyeTracker::GetCombinedWorldGazeHitData() const
{
	return CombinedWorldGazeHitData;
}

const FHitResult& FTobiiEyeTracker::GetLeftWorldGazeHitData() const
{
	return LeftWorldGazeHitData;
}

const FHitResult& FTobiiEyeTracker::GetRightWorldGazeHitData() const
{
	return RightWorldGazeHitData;
}

const FTobiiDisplayInfo& FTobiiEyeTracker::GetDisplayInformation() const
{
	return DisplayInfo;
}

const FTobiiDesktopTrackBox& FTobiiEyeTracker::GetDesktopTrackBox() const
{
	return DesktopTrackBox;
}

const FRotator& FTobiiEyeTracker::GetInfiniteScreenAngles() const
{
	return InfiniteScreenAngles;
}

/************************************************************************/
/* IEyetracker                                                          */
/************************************************************************/
bool FTobiiEyeTracker::GetEyeTrackerGazeData(FEyeTrackerGazeData& OutGazeData) const
{
	OutGazeData.ConfidenceValue = CombinedGazeData.bIsGazeDataValid ? 1.0f : 0.0f;
	OutGazeData.FixationPoint = FVector::ZeroVector; //This data is not useful for a game designer as it is not reliable enough, so we don't supply it.
	OutGazeData.GazeOrigin = CombinedGazeData.WorldGazeOrigin;
	OutGazeData.GazeDirection = CombinedGazeData.WorldGazeDirection;

	return true;
}

bool FTobiiEyeTracker::GetEyeTrackerStereoGazeData(FEyeTrackerStereoGazeData& OutGazeData) const
{
	OutGazeData.ConfidenceValue = CombinedGazeData.bIsGazeDataValid ? 1.0f : 0.0f;
	OutGazeData.FixationPoint = FVector::ZeroVector; //This data is not useful for a game designer as it is not reliable enough, so we don't supply it.
	OutGazeData.LeftEyeOrigin = LeftGazeData.WorldGazeOrigin;
	OutGazeData.LeftEyeDirection = LeftGazeData.WorldGazeDirection;
	OutGazeData.RightEyeOrigin = RightGazeData.WorldGazeOrigin;
	OutGazeData.RightEyeOrigin = RightGazeData.WorldGazeDirection;

	return true;
}

EEyeTrackerStatus FTobiiEyeTracker::GetEyeTrackerStatus() const
{
	switch (GazeTrackerStatus)
	{
	case ETobiiGazeTrackerStatus::NotConnected:		return EEyeTrackerStatus::NotConnected;
	case ETobiiGazeTrackerStatus::NotConfigured:		return EEyeTrackerStatus::NotConnected;
	case ETobiiGazeTrackerStatus::Disabled:			return EEyeTrackerStatus::NotConnected;
	case ETobiiGazeTrackerStatus::UserNotPresent:	return EEyeTrackerStatus::NotTracking;
	case ETobiiGazeTrackerStatus::UserPresent:		return EEyeTrackerStatus::Tracking;
	default:									return EEyeTrackerStatus::NotConnected;
	}
}

FVector2D FTobiiEyeTracker::ConvertRawGazePointUNormToGameViewportCoordinateUNorm(FSceneViewport* GameViewport, const FVector2D& InNormalizedPoint)
{
#if WITH_EDITOR
	//We need this to make sure we get a correct gaze point if the editor has tool windows attached to the window itself.
	FIntPoint VirtualDesktopPixelCoord;
	if (FTobiiPlatformSpecific::ConvertGazeCoordinateToVirtualDesktopPixel(DisplayInfo.GameWindowHandle, InNormalizedPoint, VirtualDesktopPixelCoord))
	{
		FVector2D ViewportPixelCoordUNorm;
		if (UTobiiBlueprintLibrary::VirtualDesktopPixelToViewportCoordinateUNorm(VirtualDesktopPixelCoord, ViewportPixelCoordUNorm))
		{
			return ViewportPixelCoordUNorm;
		}
	}
#endif

	return InNormalizedPoint;
}

// void FTobiiEyeTracker::DrawDebug(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplayInfo, float& YL, float& YPos)
// {
// 	if (bIsXR)
// 	{
// 		return;
// 	}
// 
//  	UWorld* World = Canvas->GetWorld();
//  	UFont* RenderFont = GEngine->GetSmallFont();
//  	EGazeTrackerStatus Status = GetGazeTrackerStatus();
//  
//  	//First draw our status string
//  	FString StatusText;
//  	switch (Status)
//  	{
//  	case EGazeTrackerStatus::Disabled:
//  		StatusText = "Eyetracking disabled";
//  		Canvas->DrawColor = FColor::Magenta;
//  		break;
//  	case EGazeTrackerStatus::UserNotPresent:
//  		StatusText = "User not present";
//  		Canvas->DrawColor = FColor::Purple;
//  		break;
//  	case EGazeTrackerStatus::UserPresent:
//  		StatusText = "User is present";
//  		Canvas->DrawColor = FColor::Blue;
//  		break;
//  	default:
//  		StatusText = "Eyetracking not available";
//  		Canvas->DrawColor = FColor::Red;
//  		break;
//  	}
//  
//  	TArray<FStringFormatArg> Args;
//  	Args.Emplace(StatusText);
//  
//  	//Can't use SizeX/Y since it wont work on other DPI settings
//  	float CanvasDrawWidth = Canvas->ClipX - Canvas->OrgX;
//  	float CanvasDrawHeight = Canvas->ClipY - Canvas->OrgY;
//  
//  	Canvas->DrawText(RenderFont, FString::Format(TEXT("Status: {0}"), Args), 0.5f * CanvasDrawWidth - 100, 0.05f * CanvasDrawHeight);
//  
//  	if (CVarEnableHeadPoseDebug.GetValueOnGameThread())
//  	{
//  		const float PoseDebugScale = 1.0f;
//  
//  		FVector WorldPosition, WorldDirection;
//  		HUD->Deproject(CanvasDrawWidth * 0.5f, CanvasDrawHeight * 0.8f, WorldPosition, WorldDirection);
//  
//  		//First we figure our origin data
//  		const FVector PoseDebugLocation = WorldPosition + WorldDirection * 200.0f;
//  		const FQuat PoseDebugRotation = HUD->GetOwner()->GetActorRotation().Quaternion();
//  
//  		FCanvasLineItem LineItem;
//  		const FHeadPoseData& HeadPose = GetHeadPoseData();
//  		const float BasisScale = PoseDebugScale * 7.0f;
//  		LineItem.LineThickness = 3.0f;
//  
//  		FQuat OriginRotation = PoseDebugRotation * HeadPose.HeadOrientation.Quaternion();
//  		FVector ForwardBasis = OriginRotation.RotateVector(FVector::ForwardVector) * BasisScale;
//  		FVector RightBasis = OriginRotation.RotateVector(FVector::RightVector) * BasisScale * 0.5f;
//  		FVector UpBasis = OriginRotation.RotateVector(FVector::UpVector) * BasisScale * 0.5f;
//  
//  		FVector OriginWorldPosition = PoseDebugLocation;// + PoseDebugRotation.RotateVector((HeadPose.HeadPositionCm - TrackBoxCenter) * PoseDebugScale);
//  		FVector ProjectedOrigin = HUD->Project(OriginWorldPosition);
//  		FVector ProjectedForward = HUD->Project(OriginWorldPosition + ForwardBasis);
//  		FVector ProjectedRight = HUD->Project(OriginWorldPosition + RightBasis);
//  		FVector ProjectedUp = HUD->Project(OriginWorldPosition + UpBasis);
//  
//  		LineItem.SetColor(FLinearColor::Red);
//  		LineItem.Origin = ProjectedOrigin;
//  		LineItem.EndPos = ProjectedForward;
//  		Canvas->DrawItem(LineItem);
//  		LineItem.SetColor(FLinearColor::Green);
//  		LineItem.Origin = ProjectedOrigin;
//  		LineItem.EndPos = ProjectedRight;
//  		Canvas->DrawItem(LineItem);
//  		LineItem.SetColor(FLinearColor::Blue);
//  		LineItem.Origin = ProjectedOrigin;
//  		LineItem.EndPos = ProjectedUp;
//  		Canvas->DrawItem(LineItem);
//  	}
// }

#endif //TOBII_EYETRACKING_ACTIVE
