/******************************************************************************
* Copyright 2017- Tobii Technology AB. All rights reserved.
*
* @author Temaran | Fredrik Lindh | fredrik.lindh@tobii.com | https://github.com/Temaran
******************************************************************************/

#include "TobiiBlueprintLibrary.h"
#include "TobiiCoreModule.h"
#include "TobiiPlatformSpecific.h"
#include "ITobiiCore.h"

#include "IXRTrackingSystem.h"
#include "Engine/LocalPlayer.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/Paths.h"
#include "Slate/SceneViewport.h"

UTobiiBlueprintLibrary::UTobiiBlueprintLibrary(const class FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
}

void UTobiiBlueprintLibrary::SetTobiiEyetrackingEnabled(bool bEyetrackingEnabled)
{
	SetTobiiInt("tobii.EnableEyetracking", bEyetrackingEnabled ? 1 : 0);
}

bool UTobiiBlueprintLibrary::GetTobiiEyetrackingEnabled()
{
	return GetTobiiInt("tobii.EnableEyetracking") != 0;
}

void UTobiiBlueprintLibrary::SetTobiiEyetrackingFrozen(bool bEyetrackingFrozen)
{
	SetTobiiInt("tobii.FreezeGazeData", bEyetrackingFrozen ? 1 : 0);
}

bool UTobiiBlueprintLibrary::GetTobiiEyetrackingFrozen()
{
	return GetTobiiInt("tobii.FreezeGazeData") != 0;
}

FTobiiGazeData UTobiiBlueprintLibrary::GetTobiiCombinedGazeData()
{
	static FTobiiGazeData DummyData;
	return FTobiiCoreModule::IsAvailable() ? FTobiiCoreModule::GetEyeTracker()->GetCombinedGazeData() : DummyData;
}

FTobiiGazeData UTobiiBlueprintLibrary::GetTobiiLeftGazeData()
{
	static FTobiiGazeData DummyData;
	return FTobiiCoreModule::IsAvailable() ? FTobiiCoreModule::GetEyeTracker()->GetLeftGazeData() : DummyData;
}

FTobiiGazeData UTobiiBlueprintLibrary::GetTobiiRightGazeData()
{
	static FTobiiGazeData DummyData;
	return FTobiiCoreModule::IsAvailable() ? FTobiiCoreModule::GetEyeTracker()->GetRightGazeData() : DummyData;
}

FHitResult UTobiiBlueprintLibrary::GetTobiiCombinedWorldGazeHitData()
{
	static FHitResult DummyData;
	DummyData.Actor = nullptr;
	return FTobiiCoreModule::IsAvailable() ? FTobiiCoreModule::GetEyeTracker()->GetCombinedWorldGazeHitData() : DummyData;
}

ETobiiGazeTrackerStatus UTobiiBlueprintLibrary::GetTobiiGazeTrackerStatus()
{
	return FTobiiCoreModule::IsAvailable() ? FTobiiCoreModule::GetEyeTracker()->GetGazeTrackerStatus() : ETobiiGazeTrackerStatus::NotConnected;
}

FTobiiDisplayInfo UTobiiBlueprintLibrary::GetTobiiDisplayInformation()
{
	return FTobiiCoreModule::IsAvailable() ? FTobiiCoreModule::GetEyeTracker()->GetDisplayInformation() : FTobiiDisplayInfo();
}

const FTobiiHeadPoseData& UTobiiBlueprintLibrary::GetTobiiHeadPoseData()
{
	static FTobiiHeadPoseData DummyData;
	return FTobiiCoreModule::IsAvailable() ? FTobiiCoreModule::GetEyeTracker()->GetHeadPoseData() : DummyData;
}

const FTobiiDesktopTrackBox& UTobiiBlueprintLibrary::GetTobiiDesktopTrackBox()
{
 	static FTobiiDesktopTrackBox DummyData;
 	return FTobiiCoreModule::IsAvailable() ? FTobiiCoreModule::GetEyeTracker()->GetDesktopTrackBox() : DummyData;
}

FRotator UTobiiBlueprintLibrary::GetTobiiInfiniteScreenAngles()
{
	if (FTobiiCoreModule::IsAvailable())
	{
		return ITobiiCore::GetEyeTracker()->GetInfiniteScreenAngles();
	}
	else
	{
		return FRotator::ZeroRotator;
	}
}

/************************************************************************/
/* Utils                                                                */
/************************************************************************/
bool UTobiiBlueprintLibrary::VirtualDesktopPixelToViewportCoordinateUNorm(const FVector2D& VirtualDesktopPixel, FVector2D& OutViewportCoordinateUNorm)
{
	if (!FTobiiCoreModule::IsAvailable() || GEngine == nullptr || GEngine->GameViewport == nullptr || GEngine->GameViewport->GetGameViewport() == nullptr)
	{
		return false;
	}

	OutViewportCoordinateUNorm = GEngine->GameViewport->GetGameViewport()->VirtualDesktopPixelToViewport(FIntPoint(VirtualDesktopPixel.X, VirtualDesktopPixel.Y));
	return true;
}

bool UTobiiBlueprintLibrary::ViewportCoordinateUNormToVirtualDesktopPixel(const FVector2D& ViewportCoordinateUNorm, FVector2D& OutVirtualDesktopPixel)
{
	if (!FTobiiCoreModule::IsAvailable() || GEngine == nullptr || GEngine->GameViewport == nullptr || GEngine->GameViewport->GetGameViewport() == nullptr)
	{
		return false;
	}
	
	const FTobiiDisplayInfo& DisplayInfo = FTobiiCoreModule::GetEyeTracker()->GetDisplayInformation();
	FVector2D ScaledInPoint(ViewportCoordinateUNorm.X / DisplayInfo.DpiScale, ViewportCoordinateUNorm.Y / DisplayInfo.DpiScale);
	OutVirtualDesktopPixel = GEngine->GameViewport->GetGameViewport()->ViewportToVirtualDesktopPixel(ScaledInPoint);

	return true;
}

bool UTobiiBlueprintLibrary::ViewportPixelCoordToCmCoord(const FVector2D& InCoordinatePx, FVector2D& OutCoordinateCm)
{
	const FTobiiDisplayInfo& DisplayInfo = FTobiiCoreModule::GetEyeTracker()->GetDisplayInformation();

	if (DisplayInfo.MainViewportWidthPx > 0.0f && DisplayInfo.MainViewportWidthCm > 0.0f && DisplayInfo.MainViewportHeightPx > 0.0f && DisplayInfo.MainViewportHeightCm > 0.0f)
	{
		OutCoordinateCm.Set(InCoordinatePx.X / DisplayInfo.MainViewportWidthPx * DisplayInfo.MainViewportWidthCm, InCoordinatePx.Y / DisplayInfo.MainViewportHeightPx * DisplayInfo.MainViewportHeightCm);
		return true;
	}

	OutCoordinateCm = FVector2D::ZeroVector;
	return false;
}

bool UTobiiBlueprintLibrary::ViewportCmCoordToPixelCoord(const FVector2D& InCoordinateCm, FVector2D& OutCoordinatePx)
{
	const FTobiiDisplayInfo& DisplayInfo = FTobiiCoreModule::GetEyeTracker()->GetDisplayInformation();

	if (DisplayInfo.MainViewportWidthPx > 0.0f && DisplayInfo.MainViewportWidthCm > 0.0f && DisplayInfo.MainViewportHeightPx > 0.0f && DisplayInfo.MainViewportHeightCm > 0.0f)
	{
		OutCoordinatePx.Set(InCoordinateCm.X / DisplayInfo.MainViewportWidthCm * DisplayInfo.MainViewportWidthPx, InCoordinateCm.Y / DisplayInfo.MainViewportHeightCm * DisplayInfo.MainViewportHeightPx);
		return true;
	}

	OutCoordinatePx = FVector2D::ZeroVector;
	return false;
}

bool UTobiiBlueprintLibrary::ViewportPixelCoordToUNormCoord(const FVector2D& InCoordinatePx, FVector2D& OutCoordinateUNorm)
{
	const FTobiiDisplayInfo& DisplayInfo = FTobiiCoreModule::GetEyeTracker()->GetDisplayInformation();

	if (DisplayInfo.MainViewportWidthPx > 0.0f && DisplayInfo.MainViewportHeightPx > 0.0f)
	{
		OutCoordinateUNorm.Set(InCoordinatePx.X / DisplayInfo.MainViewportWidthPx, InCoordinatePx.Y / DisplayInfo.MainViewportHeightPx);
		return true;
	}

	OutCoordinateUNorm = FVector2D::ZeroVector;
	return false;
}

bool UTobiiBlueprintLibrary::ViewportUNormCoordToPixelCoord(const FVector2D& InCoordinateUNorm, FVector2D& OutCoordinatePx)
{
	const FTobiiDisplayInfo& DisplayInfo = FTobiiCoreModule::GetEyeTracker()->GetDisplayInformation();

	if (DisplayInfo.MainViewportWidthPx > 0.0f && DisplayInfo.MainViewportHeightPx > 0.0f)
	{
		OutCoordinatePx.Set(InCoordinateUNorm.X * DisplayInfo.MainViewportWidthPx, InCoordinateUNorm.Y * DisplayInfo.MainViewportHeightPx);
		return true;
	}

	OutCoordinatePx = FVector2D::ZeroVector;
	return false;
}

// internal helper
FQuat GetHMDToWorldRotation(APlayerController* PlayerController)
{
	FQuat ToWorld = FQuat::Identity;
	FVector HMDPosition;
	bool valid = GEngine->XRSystem->GetCurrentPose(IXRTrackingSystem::HMDDeviceId, ToWorld, HMDPosition);
	
	const bool IsEmulated = IConsoleManager::Get().FindConsoleVariable(TEXT("tobii.emulation.EnableEyetrackingEmulation"))->GetInt() != 0;
	const bool IsApplyControlRotation = IConsoleManager::Get().FindConsoleVariable(TEXT("tobii.xr.ApplyControlRotation"))->GetInt() != 0;
	const bool IsApplyActorRotation = IConsoleManager::Get().FindConsoleVariable(TEXT("tobii.xr.ApplyActorRotation"))->GetInt() != 0;

	if (!IsEmulated)
	{
		if (IsApplyControlRotation)
		{
			FVector RealWorldUp = ToWorld.Inverse().RotateVector(FVector::UpVector);
			FQuat YawRotation = FQuat(RealWorldUp, FMath::DegreesToRadians(PlayerController->GetControlRotation().Yaw));
			ToWorld *= YawRotation;
		}
	}

	if (IsApplyActorRotation && PlayerController != nullptr	&& PlayerController->GetPawn() != nullptr)
	{
		ToWorld *= PlayerController->GetPawn()->GetActorRotation().Quaternion();
	}

	return ToWorld;
}

int32 UTobiiBlueprintLibrary::GetTobiiInt(FString CVarName)
{
	const auto CVar = IConsoleManager::Get().FindConsoleVariable(*CVarName);
	return (CVar != nullptr) ? CVar->GetInt() : 0;
}
void UTobiiBlueprintLibrary::SetTobiiInt(FString CVarName, const int32 NewValue)
{
	const auto CVar = IConsoleManager::Get().FindConsoleVariable(*CVarName);
	if (CVar != nullptr)
	{
		CVar->Set(NewValue);
	}
}

float UTobiiBlueprintLibrary::GetTobiiFloat(FString CVarName)
{
	const auto CVar = IConsoleManager::Get().FindConsoleVariable(*CVarName);
	return (CVar != nullptr) ? CVar->GetFloat() : 0.0f;
}
void UTobiiBlueprintLibrary::SetTobiiFloat(FString CVarName, const float NewValue)
{
	const auto CVar = IConsoleManager::Get().FindConsoleVariable(*CVarName);
	if (CVar != nullptr)
	{
		CVar->Set(NewValue);
	}
}

FString GetTobiiSettingsFilePath()
{
	return FPaths::Combine(FPaths::GeneratedConfigDir() + ANSI_TO_TCHAR(FPlatformProperties::PlatformName()), "TobiiEyetracking.ini");
}
void UTobiiBlueprintLibrary::LoadTobiiSettings()
{
	ApplyCVarSettingsFromIni(TEXT("TobiiEyetracking"), *GetTobiiSettingsFilePath(), ECVF_SetByConstructor);
}
void UTobiiBlueprintLibrary::SaveTobiiSetting(FString CVarSettingToSave)
{
	auto CVar = IConsoleManager::Get().FindConsoleVariable(*CVarSettingToSave);
	if (CVar != nullptr)
	{
		GConfig->EmptySection(TEXT("TobiiEyetracking"), *GetTobiiSettingsFilePath());
		GConfig->SetString(TEXT("TobiiEyetracking"), *CVarSettingToSave, *CVar->GetString(), *GetTobiiSettingsFilePath());
		GConfig->Flush(false, *GetTobiiSettingsFilePath());
	}
}

bool UTobiiBlueprintLibrary::IsXRPlayerController(const APlayerController* PlayerController)
{
	if (GEngine != nullptr)
	{
		if (PlayerController != nullptr)
		{
			ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(PlayerController->Player);
			if (LocalPlayer != nullptr && LocalPlayer->ViewportClient != nullptr)
			{
				//@TODO: Bugged in Shipping builds 4.18? Returns false even though running through VIVE.
				return GEngine->IsStereoscopic3D(LocalPlayer->ViewportClient->Viewport);
			}
		}

		return GEngine->IsStereoscopic3D();
	}

	return false;
}

FVector UTobiiBlueprintLibrary::ConvertGazeToUserSpace(APlayerController* PlayerController, const FVector& WorldSpaceGazeDirection)
{
	FVector LocalSpaceDirection = WorldSpaceGazeDirection;
	if (GEngine == nullptr || PlayerController == nullptr)
	{
		return LocalSpaceDirection;
	}

	if (UTobiiBlueprintLibrary::IsXRPlayerController(PlayerController))
	{
		static const auto HMDOrientationCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("tobii.xr.ApplyHMDOrientation"));
		static const auto ActorRotationCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("tobii.xr.ApplyActorRotation"));

		if (ActorRotationCVar->GetInt() != 0
			&& PlayerController->GetPawn() != nullptr)
		{
			FQuat ActorRotation = PlayerController->GetPawn()->GetActorRotation().Quaternion();
			LocalSpaceDirection = ActorRotation.UnrotateVector(LocalSpaceDirection);
		}

		if (HMDOrientationCVar->GetInt() != 0)
		{
			FQuat HMDOrientation;
			FVector HMDPosition;
			GEngine->XRSystem->GetCurrentPose(IXRTrackingSystem::HMDDeviceId, HMDOrientation, HMDPosition);
			LocalSpaceDirection = HMDOrientation.UnrotateVector(LocalSpaceDirection);
		}
	}
	else
	{
		if (PlayerController->PlayerCameraManager != nullptr)
		{
			FRotator CameraRotation = PlayerController->PlayerCameraManager->GetCameraRotation();
			LocalSpaceDirection = CameraRotation.UnrotateVector(LocalSpaceDirection);
		}
	}

	return LocalSpaceDirection;
}
