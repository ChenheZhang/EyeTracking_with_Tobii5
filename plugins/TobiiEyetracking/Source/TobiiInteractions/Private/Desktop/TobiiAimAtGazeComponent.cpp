/******************************************************************************
* Copyright 2017- Tobii Technology AB. All rights reserved.
*
* @author Temaran | Fredrik Lindh | fredrik.lindh@tobii.com | https://github.com/Temaran
******************************************************************************/

#include "TobiiAimAtGazeComponent.h"
#include "TobiiInteractionsBlueprintLibrary.h"
#include "TobiiInteractionsInternalTypes.h"
#include "TobiiGTOMBlueprintLibrary.h"

#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"

static TAutoConsoleVariable<int32> CVarAimAtGazeEnabled(TEXT("tobii.interaction.AimAtGazeEnabled"), 1, TEXT("Aim@gaze will rotate the view to align with the gaze point when the user triggers it, usually by entering ADS (Aim Down Sights). 0 - Aim@gaze is off.  1 - Aim@gaze is on."));
static TAutoConsoleVariable<float> CVarAimAtGazeDoneThreshold(TEXT("tobii.interaction.AimAtGazeDoneThreshold"), 0.0001, TEXT("If the dot product between the current and target direction quats is smaller than this, we are done rotating."));

static TAutoConsoleVariable<int32> CVarAimAtGazeDebug(TEXT("tobii.debug.AimAtGaze"), 1, TEXT("0 - Will not visualize aim@gaze debug data. 1 - Will visualize aim@gaze debug data."));

UTobiiAimAtGazeComponent::UTobiiAimAtGazeComponent()
	: bAllowRetarget(false)
	, AimSpeed(0.2f)

	, CurrentFocusComponent(nullptr)
	, CurrentAimTarget(0.0f, 0.0f, 0.0f)
	, bIsGazeAiming(false)
{
	PrimaryComponentTick.bCanEverTick = true;
}

bool UTobiiAimAtGazeComponent::AimAtGazeAvailable()
{
	return UTobiiInteractionsBlueprintLibrary::IsAimAtGazeEnabled();
}

void UTobiiAimAtGazeComponent::AimAtGaze()
{
	if (!AimAtGazeAvailable())
	{
		return;
	}

	CurrentFocusComponent.Reset();

	FTobiiGazeFocusData FocusData;
	UTobiiGTOMBlueprintLibrary::GetFilteredGazeFocusData(FocusLayerFilters, bIsWhiteList, true, false, FocusData);
	if (FocusData.FocusedActor.IsValid())
	{
		CurrentFocusComponent = FocusData.FocusedPrimitiveComponent; 
		CurrentAimTarget = FocusData.LastVisibleWorldLocation;
		bIsGazeAiming = true;
	}
	else
	{
		const FHitResult& GazeHitData = UTobiiGTOMBlueprintLibrary::GetNaiveGazeHit();
		CurrentAimTarget = GazeHitData.Location;
		bIsGazeAiming = true;
	}
}

void UTobiiAimAtGazeComponent::ContinuousAimAtGaze()
{
	if (!AimAtGazeAvailable())
	{
		CurrentFocusComponent = nullptr;
		return;
	}

	if (bAllowRetarget)
	{
		FTobiiGazeFocusData FocusData;
		UTobiiGTOMBlueprintLibrary::GetFilteredGazeFocusData(FocusLayerFilters, bIsWhiteList, true, false, FocusData);
		if (FocusData.FocusedPrimitiveComponent.IsValid())
		{
			CurrentFocusComponent = FocusData.FocusedPrimitiveComponent;
		}
	}

	if (CurrentFocusComponent.IsValid())
	{
		FVector TargetFocusPosition;
		UTobiiGTOMBlueprintLibrary::GetPrimitiveComponentFocusLocation(CurrentFocusComponent.Get(), TargetFocusPosition);

		CurrentAimTarget = TargetFocusPosition;
		bIsGazeAiming = true;
	}
	else
	{
		CurrentFocusComponent.Reset();
	}
}

void UTobiiAimAtGazeComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	static const auto DrawDebugCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("tobii.debug"));
	if (CurrentFocusComponent != nullptr && DrawDebugCVar->GetInt() && CVarAimAtGazeDebug.GetValueOnGameThread())
	{
		DrawDebugBox(GetWorld(), CurrentAimTarget, FVector(50.0f), FColor::Cyan, false, 0.0f);
	}

	APlayerController* PlayerController = nullptr;
	APawn* OwnerPawn = Cast<APawn>(GetOwner());

	// We make it possible to attach AimAtGaze component to either Pawn or PlayerController
	if (OwnerPawn != nullptr)
	{
		PlayerController = Cast<APlayerController>(OwnerPawn->GetController());
	}
	else
	{
		PlayerController = Cast<APlayerController>(GetOwner());
	}

	if (PlayerController != nullptr)
	{
		if (AimAtGazeAvailable() && bIsGazeAiming && PlayerController->PlayerCameraManager != nullptr)
		{
			FVector CameraLocation = PlayerController->PlayerCameraManager->GetCameraLocation();
			FVector AimTargetDirection = CurrentAimTarget - CameraLocation;

			FQuat TargetQuat = AimTargetDirection.ToOrientationQuat();
			FQuat CurrentControlQuat = FQuat(PlayerController->GetControlRotation());

			CurrentControlQuat = FQuat::Slerp(CurrentControlQuat, TargetQuat, AimSpeed);
			float DotDistance = (CurrentControlQuat | TargetQuat);

			if ((1.0f - DotDistance) < CVarAimAtGazeDoneThreshold.GetValueOnGameThread())
			{
				bIsGazeAiming = false;
				CurrentControlQuat = TargetQuat;
			}

			PlayerController->SetControlRotation(CurrentControlQuat.Rotator());
		}
	}
}
