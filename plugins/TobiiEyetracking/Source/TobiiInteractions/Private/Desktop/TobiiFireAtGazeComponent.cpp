/******************************************************************************
* Copyright 2017- Tobii Technology AB. All rights reserved.
*
* @author Temaran | Fredrik Lindh | fredrik.lindh@tobii.com | https://github.com/Temaran
******************************************************************************/

#include "TobiiFireAtGazeComponent.h"
#include "TobiiInteractionsBlueprintLibrary.h"
#include "TobiiGTOMBlueprintLibrary.h"
#include "../TobiiInteractionsInternalTypes.h"

#include "Engine/Engine.h"
#include "IEyeTracker.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"

static TAutoConsoleVariable<int32> CVarFireAtGazeEnabled(TEXT("tobii.interaction.FireAtGazeEnabled"), 1, TEXT("Fire at gaze lets you fire where you look when not in ADS (Aim Down Sights) mode. 0 - Fire at gaze is disabled. 1 - Fire at gaze is enabled."));

UTobiiFireAtGazeComponent::UTobiiFireAtGazeComponent()
	: CameraComponent(nullptr)
	
	, FireAtGazeTargetActor(nullptr)
	, FireAtGazeTargetComponent(nullptr)
	, FireAtGazeTargetLocation(0.0f, 0.0f, 0.0f)

	, MaxDistance(10000.0f)
	, NoTargetBehavior(ETobiiFireAtGazeNoTargetBehavior::PointGunToGaze)
	, TraceChannel(ECC_Visibility)
{
	PrimaryComponentTick.bCanEverTick = true;
}

bool UTobiiFireAtGazeComponent::FireAtGazeAvailable()
{
	return CameraComponent != nullptr 
		&& UTobiiInteractionsBlueprintLibrary::IsFireAtGazeEnabled();
}

bool UTobiiFireAtGazeComponent::WantsCrosshair()
{
	if (!FireAtGazeAvailable())
	{
		return false;
	}

	switch (NoTargetBehavior)
	{
	case ETobiiFireAtGazeNoTargetBehavior::PointGunForward:
		return !FireAtGazeTargetActor.IsValid();
	case ETobiiFireAtGazeNoTargetBehavior::PointGunToGaze:
		return true;
	default:
		return false;
	}
}

void UTobiiFireAtGazeComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!FireAtGazeAvailable() || GEngine == nullptr || !GEngine->EyeTrackingDevice.IsValid())
	{
		return;
	}

	FTobiiGazeFocusData FocusData;
	UTobiiGTOMBlueprintLibrary::GetFilteredGazeFocusData(FocusLayerFilters, bIsWhiteList, true, false, FocusData);
	if (FocusData.FocusedPrimitiveComponent.IsValid())
	{
		FireAtGazeTargetActor = FocusData.FocusedActor;
		FireAtGazeTargetComponent = FocusData.FocusedPrimitiveComponent;
		FireAtGazeTargetLocation = FocusData.LastVisibleWorldLocation;
	}
	else
	{
		FVector GazeRayOrigin, GazeRayDirection;
		switch (NoTargetBehavior)
		{
		case ETobiiFireAtGazeNoTargetBehavior::PointGunForward:
		{
			GazeRayOrigin = CameraComponent->GetComponentLocation();
			GazeRayDirection = CameraComponent->GetForwardVector();
			break;
		}

		case ETobiiFireAtGazeNoTargetBehavior::PointGunToGaze:
		default:
		{
			FEyeTrackerGazeData CombinedGazeData;
			GEngine->EyeTrackingDevice->GetEyeTrackerGazeData(CombinedGazeData);
			GazeRayOrigin = CombinedGazeData.GazeOrigin;
			GazeRayDirection = CombinedGazeData.GazeDirection;
			break;
		}
		}

		FHitResult HitResult;
		UWorld* World = GetWorld();
		if (World != nullptr && World->LineTraceSingleByChannel(HitResult, GazeRayOrigin, GazeRayOrigin + GazeRayDirection * MaxDistance, TraceChannel)
			&& UTobiiGTOMBlueprintLibrary::IsPrimitiveComponentGazeFocusable(HitResult.GetComponent()))
		{
			FireAtGazeTargetActor = HitResult.GetActor();
			FireAtGazeTargetComponent = HitResult.GetComponent();
			FireAtGazeTargetLocation = HitResult.Location;
		}
		else
		{
			FireAtGazeTargetActor = nullptr;
			FireAtGazeTargetComponent = nullptr;
			FireAtGazeTargetLocation = GazeRayOrigin + GazeRayDirection * MaxDistance;
		}
	}
}
