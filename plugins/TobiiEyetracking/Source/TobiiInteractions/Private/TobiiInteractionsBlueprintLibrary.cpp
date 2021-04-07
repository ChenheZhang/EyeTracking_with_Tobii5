/******************************************************************************
* Copyright 2017- Tobii Technology AB. All rights reserved.
*
* @author Temaran | Fredrik Lindh | fredrik.lindh@tobii.com | https://github.com/Temaran
* Thanks to Jochen Schwarze (schwarze@isa.de) for functions to solve square, cubic and quartic functions as found here:
* https://github.com/erich666/GraphicsGems/blob/240a34f2ad3fa577ef57be74920db6c4b00605e4/gems/Roots3And4.c
******************************************************************************/

#include "TobiiInteractionsBlueprintLibrary.h"
#include "TobiiRootFinders.h"

#include "Components/WidgetComponent.h"
#include "IEyeTracker.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/Texture2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Framework/Application/SlateApplication.h"
#include "HAL/IConsoleManager.h"
#include "Slate/WidgetRenderer.h"

UTobiiInteractionsBlueprintLibrary::UTobiiInteractionsBlueprintLibrary(const class FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
}

/**
 * Renders a UMG Widget to a texture with the specified size.
 *
 * @param Widget        The widget to be rendered.
 * @param DrawSize    The size to render the Widget to. Also will be the texture size.
 * @return            The texture containing the rendered widget.
 */
UTexture2D* UTobiiInteractionsBlueprintLibrary::TextureFromWidget(UUserWidget* const Widget, const FVector2D& DrawSize)
{
	if (FSlateApplication::IsInitialized()
		&& Widget != nullptr && Widget->IsValidLowLevel()
		&& DrawSize.X >= 1 && DrawSize.Y >= 1)
	{
		TSharedPtr<SWidget> SlateWidget(Widget->TakeWidget());
		if (!SlateWidget.IsValid())
		{
			return nullptr;
		}

		FWidgetRenderer WidgetRenderer = FWidgetRenderer(true);

		UTextureRenderTarget2D* TextureRenderTarget = WidgetRenderer.DrawWidget(SlateWidget.ToSharedRef(), DrawSize);
		// Creates Texture2D to store RenderTexture content
		UTexture2D *Texture = UTexture2D::CreateTransient(DrawSize.X, DrawSize.Y, PF_B8G8R8A8);
#if WITH_EDITORONLY_DATA
		Texture->MipGenSettings = TMGS_NoMipmaps;
#endif

		// Lock and copies the data between the textures
		TArray<FColor> SurfData;
		FRenderTarget* RenderTarget = TextureRenderTarget->GameThread_GetRenderTargetResource();
		RenderTarget->ReadPixels(SurfData);

		void* TextureData = Texture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
		const int32 TextureDataSize = SurfData.Num() * 4;
		FMemory::Memcpy(TextureData, SurfData.GetData(), TextureDataSize);
		Texture->PlatformData->Mips[0].BulkData.Unlock();
		Texture->UpdateResource();

		// Free resources
		SurfData.Empty();
		TextureRenderTarget->ConditionalBeginDestroy();
		SlateWidget.Reset();

		return Texture;
	}

	return nullptr;
}

bool UTobiiInteractionsBlueprintLibrary::IsInfiniteScreenEnabled()
{
	static const auto EyetrackingEnabledCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("tobii.EnableEyetracking"));
	static const auto InfiniteScreenEnabledCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("tobii.desktop.InfiniteScreenEnabled"));

	if (EyetrackingEnabledCVar != nullptr && InfiniteScreenEnabledCVar != nullptr && GEngine != nullptr && GEngine->EyeTrackingDevice.IsValid())
	{
		return GEngine->EyeTrackingDevice->GetEyeTrackerStatus() >= EEyeTrackerStatus::Tracking
			&& EyetrackingEnabledCVar->GetInt()
			&& InfiniteScreenEnabledCVar->GetInt();
	}

	return false;
}

bool UTobiiInteractionsBlueprintLibrary::IsCleanUIEnabled()
{
	static const auto EyetrackingEnabledCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("tobii.EnableEyetracking"));
	static const auto CleanUIEnabledCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("tobii.interaction.EnableCleanUI"));

	if (EyetrackingEnabledCVar != nullptr && CleanUIEnabledCVar != nullptr && GEngine != nullptr && GEngine->EyeTrackingDevice.IsValid())
	{
		return GEngine->EyeTrackingDevice->GetEyeTrackerStatus() >= EEyeTrackerStatus::Tracking
			&& EyetrackingEnabledCVar->GetInt()
			&& CleanUIEnabledCVar->GetInt();
	}

	return false;
}

bool UTobiiInteractionsBlueprintLibrary::IsAimAtGazeEnabled()
{
	static const auto EyetrackingEnabledCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("tobii.EnableEyetracking"));
	static const auto AimAtGazeEnabledCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("tobii.interaction.AimAtGazeEnabled"));

	if (EyetrackingEnabledCVar != nullptr && AimAtGazeEnabledCVar != nullptr && GEngine != nullptr && GEngine->EyeTrackingDevice.IsValid())
	{
		return GEngine->EyeTrackingDevice->GetEyeTrackerStatus() >= EEyeTrackerStatus::Tracking
			&& EyetrackingEnabledCVar->GetInt()
			&& AimAtGazeEnabledCVar->GetInt();
	}

	return false;
}

bool UTobiiInteractionsBlueprintLibrary::IsFireAtGazeEnabled()
{
	static const auto EyetrackingEnabledCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("tobii.EnableEyetracking"));
	static const auto FireAtGazeEnabledCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("tobii.interaction.FireAtGazeEnabled"));

	if (EyetrackingEnabledCVar != nullptr && FireAtGazeEnabledCVar != nullptr && GEngine != nullptr && GEngine->EyeTrackingDevice.IsValid())
	{
		return GEngine->EyeTrackingDevice->GetEyeTrackerStatus() >= EEyeTrackerStatus::Tracking
			&& EyetrackingEnabledCVar->GetInt()
			&& FireAtGazeEnabledCVar->GetInt();
	}

	return false;
}

bool UTobiiInteractionsBlueprintLibrary::IsThrowAtGazeEnabled()
{
	static const auto EyetrackingEnabledCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("tobii.EnableEyetracking"));
	static const auto ThrowAtGazeEnabledCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("tobii.interaction.ThrowAtGazeEnabled"));

	if (EyetrackingEnabledCVar != nullptr && ThrowAtGazeEnabledCVar != nullptr && GEngine != nullptr && GEngine->EyeTrackingDevice.IsValid())
	{
		return GEngine->EyeTrackingDevice->GetEyeTrackerStatus() >= EEyeTrackerStatus::Tracking
			&& EyetrackingEnabledCVar->GetInt()
			&& ThrowAtGazeEnabledCVar->GetInt();
	}

	return false;
}

float UTobiiInteractionsBlueprintLibrary::CalculateSmoothPitchStep(float ViewPitch)
{
	//Limit infinite screen yaw depending on input device pitch		
	float NormalizedPitch = FRotator::NormalizeAxis(ViewPitch);
	float PitchScale = 1.0f - (FMath::Abs(NormalizedPitch) / 90.0f);
	return FMath::SmoothStep(0.0f, 1.0f, PitchScale);
}

FRotator UTobiiInteractionsBlueprintLibrary::MakeInfiniteScreenCameraRotator(FRotator OriginalCameraRotation, FRotator InfiniteScreenAngles)
{
	FQuat WorldSpaceYawRotation = FQuat(FVector::UpVector, InfiniteScreenAngles.Yaw);
	FQuat LocalSpacePitchRotation = FQuat(FVector::RightVector, -InfiniteScreenAngles.Pitch);
	
	FQuat InfiniteScreenViewRotation(OriginalCameraRotation);
	InfiniteScreenViewRotation = InfiniteScreenViewRotation * LocalSpacePitchRotation; //Local space by multiplying on the right
	InfiniteScreenViewRotation = WorldSpaceYawRotation * InfiniteScreenViewRotation; //World space by multiplying on the left
	return InfiniteScreenViewRotation.Rotator();
}

////////////////////////////////////////////////////////////////////////////

void UTobiiInteractionsBlueprintLibrary::FindRealSquareRoots(float A, float B, float C, TArray<float>& OutRealRoots)
{
	double Coefficients[3]{ C, B, A };
	double Solutions[2]{ 0.0, 0.0 };

	int32 NrSolutions = SolveQuadric(Coefficients, Solutions);
	OutRealRoots.Empty(NrSolutions);
	for (int32 SolutionIdx = 0; SolutionIdx < NrSolutions; SolutionIdx++)
	{
		OutRealRoots.Add(Solutions[SolutionIdx]);
	}
}

void UTobiiInteractionsBlueprintLibrary::FindRealCubicRoots(float A, float B, float C, float D, TArray<float>& OutRealRoots)
{
	double Coefficients[4]{ D, C, B, A };
	double Solutions[3]{ 0.0, 0.0, 0.0 };

	int32 NrSolutions = SolveCubic(Coefficients, Solutions);
	OutRealRoots.Empty(NrSolutions);
	for (int32 SolutionIdx = 0; SolutionIdx < NrSolutions; SolutionIdx++)
	{
		OutRealRoots.Add(Solutions[SolutionIdx]);
	}
}

void UTobiiInteractionsBlueprintLibrary::FindRealQuarticRoots(float A, float B, float C, float D, float E, TArray<float>& OutRealRoots)
{
	double Coefficients[5]{ E, D, C, B, A };
	double Solutions[4]{ 0.0, 0.0, 0.0, 0.0 };

	int32 NrSolutions = SolveQuartic(Coefficients, Solutions);
	OutRealRoots.Empty(NrSolutions);
	for (int32 SolutionIdx = 0; SolutionIdx < NrSolutions; SolutionIdx++)
	{
		OutRealRoots.Add(Solutions[SolutionIdx]);
	}
}

/**
  * Try to find the appropriate acceleration to hit a moving target.
  * We do this by setting up an equation system with 4 equations in 4 unknowns.
  * First we solve for time, and then we plug that into the other equations to find the wanted acceleration.
  *
  * VARIABLES:
  * Time:						t				<--- Need to first solve for this
  * Projectile Pos:				PPX, PPY, PPZ
  * Projectile Vel:				PVX, PVY, PVZ
  * Projectile Acc:				pax, pay, paz   <--- Solving for this is our goal
  * Projectile AccMagnitude:	PAM
  * Target Pos:					TPX, TPY, TPZ
  * Target Vel:					TVX, TVY, TVZ
  * Target Acc:					TAX, TAY, TAZ
  * 
  * EQUATIONS:
  * PPX + PVX * t + (1/2) * pax * t^2 = TPX + TVX * t + (1/2) * TAX * t^2
  * PPY + PVY * t + (1/2) * pay * t^2 = TPY + TVY * t + (1/2) * TAY * t^2
  * PPZ + PVZ * t + (1/2) * paz * t^2 = TPZ + TVZ * t + (1/2) * TAZ * t^2
  * PAM^2 = pax^2 + pay^2 + paz^2
  * 
  * SOLVE FOR ACCELERATION:
  * pax = 2.0 * (TPX + TVX * t + (1/2) * TAX * t^2 - PPX - PVX * t) / t^2
  * pay = 2.0 * (TPY + TVY * t + (1/2) * TAY * t^2 - PPY - PVY * t) / t^2
  * paz = 2.0 * (TPZ + TVZ * t + (1/2) * TAZ * t^2 - PPZ - PVZ * t) / t^2
  * 
  * SIMPLIFY:
  * DPX = TPX - PPX
  * DPY = TPY - PPY
  * DPZ = TPZ - PPZ
  * DVX = TVX - PVX
  * DVY = TVY - PVY
  * DVZ = TVZ - PVZ
  * pax = 2.0 * (DPX + DVX * t + (1/2) * TAX * t^2) / t^2
  * pay = 2.0 * (DPY + DVY * t + (1/2) * TAY * t^2) / t^2
  * paz = 2.0 * (DPZ + DVZ * t + (1/2) * TAZ * t^2) / t^2
  * 
  * SQUARE SO WE CAN SUBSTITUTE LATER:
  * pax^2 = 4.0 * (DPX + DVX * t + (1/2) * TAX * t^2)^2 / t^4
  * pay^2 = 4.0 * (DPY + DVY * t + (1/2) * TAY * t^2)^2 / t^4
  * paz^2 = 4.0 * (DPZ + DVZ * t + (1/2) * TAZ * t^2)^2 / t^4
  * 
  * EXPAND THE SQUARES SO WE CAN EXTRACT COEFFICIENTS LATER:
  * pax^2 = 4.0 * (DPX^2 + 2.0*DPX*DVX*t + DPX*TAX*t^2 + DVX^2*t^2 + DVX*TAX*t^3 + (1/4)*TAX^2*t^4) / t^4
  * pay^2 = 4.0 * (DPY^2 + 2.0*DPY*DVY*t + DPY*TAY*t^2 + DVY^2*t^2 + DVY*TAY*t^3 + (1/4)*TAY^2*t^4) / t^4
  * paz^2 = 4.0 * (DPZ^2 + 2.0*DPZ*DVZ*t + DPZ*TAZ*t^2 + DVZ^2*t^2 + DVZ*TAZ*t^3 + (1/4)*TAZ^2*t^4) / t^4
  * 
  * SUBSTITUTE INTO OUR PAM EQUATION:
  * PAM^2 = 4.0 * (DPX^2 + 2.0*DPX*DVX*t + DPX*TAX*t^2 + DVX^2*t^2 + DVX*TAX*t^3 + (1/4)*TAX^2*t^4) / t^4
  *		  + 4.0 * (DPY^2 + 2.0*DPY*DVY*t + DPY*TAY*t^2 + DVY^2*t^2 + DVY*TAY*t^3 + (1/4)*TAY^2*t^4) / t^4
  *		  + 4.0 * (DPZ^2 + 2.0*DPZ*DVZ*t + DPZ*TAZ*t^2 + DVZ^2*t^2 + DVZ*TAZ*t^3 + (1/4)*TAZ^2*t^4) / t^4
  * 
  * MULTIPLY BY t^4:
  * PAM^2 * t^4 = 4.0 * (DPX^2 + 2.0*DPX*DVX*t + DPX*TAX*t^2 + DVX^2*t^2 + DVX*TAX*t^3 + (1/4)*TAX^2*t^4)
  *				+ 4.0 * (DPY^2 + 2.0*DPY*DVY*t + DPY*TAY*t^2 + DVY^2*t^2 + DVY*TAY*t^3 + (1/4)*TAY^2*t^4)
  *				+ 4.0 * (DPZ^2 + 2.0*DPZ*DVZ*t + DPZ*TAZ*t^2 + DVZ^2*t^2 + DVZ*TAZ*t^3 + (1/4)*TAZ^2*t^4)
  * 
  * MAKE 0 EQUATION AND FORM TERMS:
  * 0 = 4.0*DPX^2 + 8.0*DPX*DVX*t + 4.0*DPX*TAX*t^2 + 4.0*DVX^2*t^2 + 4.0*DVX*TAX*t^3 + TAX^2*t^4
  *	  + 4.0*DPY^2 + 8.0*DPY*DVY*t + 4.0*DPY*TAY*t^2 + 4.0*DVY^2*t^2 + 4.0*DVY*TAY*t^3 + TAY^2*t^4
  *	  + 4.0*DPZ^2 + 8.0*DPZ*DVZ*t + 4.0*DPZ*TAZ*t^2 + 4.0*DVZ^2*t^2 + 4.0*DVZ*TAZ*t^3 + TAZ^2*t^4
  *	  - PAM^2 * t^4
  * 
  * ARRANGE IN COEFFICIENT FORM:
  * 0 = (TAX^2 + TAY^2 + TAZ^2 - PAM^2)									* t^4
  *     4.0 * (DVX*TAX + DVY*TAY + DVZ*TAZ)								* t^3
  *     4.0 * (DVX^2 + DVY^2 + DVZ^2 + DPX*TAX + DPY*TAY + DPZ*TAZ)		* t^2
  *     8.0 * (DPX*DVX + DPY*DVY + DPZ*DVZ)								* t^1
  *     4.0 * (DPX^2 + DPY^2 + DPZ^2)									* t^0
  * 
  * Solve the quartic!
  * Then finally insert the smallest root (time) into the (pax, pay, paz) formulas to get the wanted acceleration.
  */
bool UTobiiInteractionsBlueprintLibrary::FindNeededAccelerationForAccelerationBasedHomingProjectile(const FTobiiAccelerationBasedHomingData& InputData, FTobiiAccelerationBasedHomingResult& BestResult)
{
	const FVector DeltaPosition = InputData.TargetPosition - InputData.ProjectilePosition;
	if (DeltaPosition.SizeSquared() < FLT_EPSILON)
	{
		return false;
	}

	const FVector DeltaVelocity = InputData.TargetVelocity - InputData.ProjectileVelocity;
	
	const double T4Coefficient = FVector::DotProduct(InputData.TargetAcceleration, InputData.TargetAcceleration) - InputData.ProjectileAccelerationMagnitude * InputData.ProjectileAccelerationMagnitude;
	const double T3Coefficient = 4.0 * FVector::DotProduct(DeltaVelocity, InputData.TargetAcceleration);
	const double T2Coefficient = 4.0 * FVector::DotProduct(DeltaVelocity, DeltaVelocity) + FVector::DotProduct(DeltaPosition, InputData.TargetAcceleration);
	const double T1Coefficient = 8.0 * FVector::DotProduct(DeltaPosition, DeltaVelocity);
	const double T0Coefficient = 4.0 * FVector::DotProduct(DeltaPosition, DeltaPosition);
	
 	TArray<double> Solutions;
	double DirectHitCoefficients[5]{ T0Coefficient, T1Coefficient, T2Coefficient, T3Coefficient, T4Coefficient };
	double DirectHitSolutions[4]{ 0.0, 0.0, 0.0, 0.0 };

	int32 NrDirectHitSolutions = SolveQuartic(DirectHitCoefficients, DirectHitSolutions);
	for (int32 SolutionIdx = 0; SolutionIdx < NrDirectHitSolutions; SolutionIdx++)
	{
		const double Solution = DirectHitSolutions[SolutionIdx];
		if (FMath::IsFinite(Solution) && !FMath::IsNaN(Solution) && Solution > DBL_EPSILON)
		{
			Solutions.Add(Solution);
		}
	}

	if (Solutions.Num() == 0 && InputData.bAttemptClosestApproachSolution)
	{
		//Since we couldn't find a direct hit, attempt to find a closest approach instead as backup.
		double ClosestApproachCoefficients[4]{ T1Coefficient, 2.0 * T2Coefficient, 3.0 * T3Coefficient, 4.0 * T4Coefficient };
		double ClosestApproachSolutions[3]{ 0.0, 0.0, 0.0 };

		TMap<double, double> ClosestApproachDistancesToInterceptTimes;
		int32 NrClosestApproachSolutions = SolveCubic(ClosestApproachCoefficients, ClosestApproachSolutions);
		for (int32 SolutionIdx = 0; SolutionIdx < NrClosestApproachSolutions; SolutionIdx++)
		{
			const double Solution = ClosestApproachSolutions[SolutionIdx];

			if (FMath::IsFinite(Solution) && !FMath::IsNaN(Solution) && Solution > DBL_EPSILON
				&& Solution >= 0.0 && !ClosestApproachDistancesToInterceptTimes.Contains(Solution))
			{
				const double Real = Solution;
				const double RealSq = Real * Real;
				const double RealCub = RealSq * Real;
				const double RealQuart = RealCub * Real;
				const double Distance = FMath::Abs(T4Coefficient * RealQuart + T3Coefficient * RealCub + T2Coefficient * RealSq + T1Coefficient * Real + T0Coefficient);
				ClosestApproachDistancesToInterceptTimes.Add(Distance, Real);
			}
		}

		ClosestApproachDistancesToInterceptTimes.KeySort(TLess<float>());
		for (auto& Pair : ClosestApproachDistancesToInterceptTimes)
		{
			Solutions.Add(Pair.Value);
			break;
		}
	}

	if (Solutions.Num() == 0)
	{
		return false;
	}

	Solutions.Sort(TLess<double>());
	const double InterceptTime = Solutions[0];
	const double InterceptTimeSquare = InterceptTime * InterceptTime;

	BestResult.Type = ETobiiInterceptType::DirectHit;
	BestResult.ExpectedInterceptTimeSecs = InterceptTime;
	BestResult.ExpectedInterceptLocation = InputData.TargetPosition	+ InputData.TargetVelocity * InterceptTime + (1.0 / 2.0) * InputData.TargetAcceleration * InterceptTimeSquare;
	BestResult.SuggestedAcceleration = 2.0 * (DeltaPosition + DeltaVelocity * InterceptTime + (1.0 / 2.0) * InputData.TargetAcceleration * InterceptTimeSquare) / InterceptTimeSquare;

	return true;
}

/**
  * Try to find the appropriate velocity to hit a moving target given a ballistic projectile.
  * We do this by setting up an equation system with 4 equations in 4 unknowns.
  * First we solve for time, and then we plug that into the other equations to find the wanted acceleration.
  *
  * VARIABLES:
  * Time:						t				<--- Need to first solve for this
  * Projectile Pos:				PPX, PPY, PPZ
  * Projectile Vel:				pvx, pvy, pvz
  * Projectile Gravity:			PAX, PAY, PAZ	<--- Most likely gravity
  * Projectile Apex:			PAPEX			<--- This is at when (1/2)*t
  * Target Pos:					TPX, TPY, TPZ
  * Target Vel:					TVX, TVY, TVZ
  * Target Acc:					TAX, TAY, TAZ
  *
  * END EQUATIONS:
  * PPX + pvx * t + (1/2) * PAX * t^2 = TPX + TVX * t + (1/2) * TAX * t^2
  * PPY + pvy * t + (1/2) * PAY * t^2 = TPY + TVY * t + (1/2) * TAY * t^2
  * PPZ + pvz * t + (1/2) * PAZ * t^2 = TPZ + TVZ * t + (1/2) * TAZ * t^2
  *
  * KNOWN Z EQUATIONS:
  * PAPEX = PPZ + (1/2) * pvz * t + (1/8) * PAZ * t^2
  *
  * SOLVE FOR VELOCITY:
  * pvx = (TPX + TVX * t + (1/2) * TAX * t^2 - PPX - (1/2) * PAX * t^2) / t
  * pvy = (TPY + TVY * t + (1/2) * TAY * t^2 - PPY - (1/2) * PAY * t^2) / t
  * pvz = (TPZ + TVZ * t + (1/2) * TAZ * t^2 - PPZ - (1/2) * PAZ * t^2) / t
  *
  * SIMPLIFY:
  * DPX = TPX - PPX
  * DPY = TPY - PPY
  * DPZ = TPZ - PPZ
  * DAX = (1/2) * (TAX - PAX)
  * DAY = (1/2) * (TAY - PAY)
  * DAZ = (1/2) * (TAZ - PAZ)
  * pvx = (DPX + TVX * t + DAX * t^2) / t
  * pvy = (DPY + TVY * t + DAY * t^2) / t
  * pvz = (DPZ + TVZ * t + DAZ * t^2) / t
  *
  * SUBSTITUTE INTO OUR MIDPOINT EQUATIONS:
  * PAPEX = PPZ + (1/2) * [(DPZ + TVZ * t + DAZ * t^2) / t] * t + (1/8) * PAZ * t^2
  *
  * MAKE 0 EQUATION AND FORM TERMS:
  * 0 = PPZ + (1/2) * DPZ - PAPEX   +   (1/2) * TVZ * t   +   (1/2) * DAZ * t^2 + (1/8) * PAZ * t^2
  *
  * ARRANGE IN COEFFICIENT FORM:
  * 0 = ((1/2) * DAZ + (1/8) * PAZ)		* t^2
  *     (1/2) * TVZ						* t^1
  *     PPZ + (1/2) * DPZ - PAPEX		* t^0
  *
  * Solve the square for time!
  * Then finally insert the roots (time) into the (pvx, pvy, pvz) formulas to get the possible velocities.
  */
bool UTobiiInteractionsBlueprintLibrary::FindNeededInitialVelocityForBallisticProjectile(const FTobiiBallisticData& InputData, TArray<FTobiiBallisticResult>& Results)
{
	const FVector DeltaPosition = InputData.TargetPosition - InputData.ProjectileInitialPosition;
	if (DeltaPosition.SizeSquared() < FLT_EPSILON)
	{
		return false;
	}

	const float ApexZ = FMath::Max(InputData.ProjectileInitialPosition.Z, InputData.TargetPosition.Z) + InputData.ProjectileApexOffsetCm;
	const FVector DeltaAcceleration = 0.5f * (InputData.TargetAcceleration - InputData.ProjectileAcceleration);
	
	const double T2Coefficient = 0.5 * DeltaAcceleration.Z + 0.125 * InputData.ProjectileAcceleration.Z;
	const double T1Coefficient = 0.5 * InputData.TargetVelocity.Z;
	const double T0Coefficient = InputData.ProjectileInitialPosition.Z + 0.5 * DeltaPosition.Z - ApexZ;
	double TimeCoefficients[3] { T0Coefficient, T1Coefficient, T2Coefficient };
	double TimeSolutions[2] { 0.0, 0.0 };

	int32 NrTimeSolutions = SolveQuadric(TimeCoefficients, TimeSolutions);	
	for (int32 SolutionIdx = 0; SolutionIdx < NrTimeSolutions; SolutionIdx++)
	{
		const double Time = TimeSolutions[SolutionIdx];
		if (FMath::IsFinite(Time) && !FMath::IsNaN(Time) && Time > DBL_EPSILON)
		{
			const double HTime = Time / 2.0;
			const double HTimeSq = HTime * HTime;
			const double TimeSq = Time * Time;

			FTobiiBallisticResult NewResult;
			NewResult.ExpectedInterceptTimeSecs = Time;
			NewResult.SuggestedInitialVelocity = (DeltaPosition + InputData.TargetVelocity * Time + DeltaAcceleration * TimeSq) / Time;
			NewResult.ExpectedInterceptLocation = InputData.ProjectileInitialPosition + NewResult.SuggestedInitialVelocity * Time + 0.5f * InputData.ProjectileAcceleration * TimeSq;
			Results.Add(NewResult);
		}
	}

	return Results.Num() > 0;
}

bool UTobiiInteractionsBlueprintLibrary::TraceBallisticProjectilePath(UObject* WorldContextObject, const FTobiiProjectileTraceData& InputData, TArray<FVector>& OutTracedPath, FHitResult& OutHitResult)
{
	if (WorldContextObject == nullptr)
	{
		return false;
	}

	float CurrentTime = InputData.StepSizeSecs;
	FVector StartPoint = InputData.ProjectileInitialPosition;
	FVector EndPoint = StartPoint + InputData.ProjectileVelocity * CurrentTime + 0.5f * InputData.ProjectileAcceleration * CurrentTime * CurrentTime;

	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActors(InputData.IgnoredActors);

	OutTracedPath.Empty();
	OutTracedPath.Add(StartPoint);

	for (int32 StepCount = 0; StepCount < InputData.MaxNrSteps; StepCount++)
	{
		if (WorldContextObject->GetWorld()->SweepSingleByChannel(OutHitResult, StartPoint, EndPoint, FQuat::Identity, InputData.TraceChannel, FCollisionShape::MakeSphere(InputData.TraceRadiusCm), CollisionParams))
		{
			OutTracedPath.Add(OutHitResult.Location);
			return true;
		}
		else
		{
			OutTracedPath.Add(EndPoint);
		}

		StartPoint = EndPoint;
		CurrentTime += InputData.StepSizeSecs;
		EndPoint = InputData.ProjectileInitialPosition + InputData.ProjectileVelocity * CurrentTime + 0.5f * InputData.ProjectileAcceleration * CurrentTime * CurrentTime;
	}

	return false;
}
