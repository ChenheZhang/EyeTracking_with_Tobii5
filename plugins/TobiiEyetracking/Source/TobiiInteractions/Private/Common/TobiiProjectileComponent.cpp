/******************************************************************************
* Copyright 2017- Tobii Technology AB. All rights reserved.
*
* @author Temaran | Fredrik Lindh | fredrik.lindh@tobii.com | https://github.com/Temaran
******************************************************************************/

#include "TobiiProjectileComponent.h"
#include "TobiiGTOMBlueprintLibrary.h"
#include "TobiiInteractionsBlueprintLibrary.h"

#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

UTobiiProjectileComponent::UTobiiProjectileComponent()
	: InitialVelocity(1.0f, 0.0f, 0.0f)
	, bAffectedByGravity(true)
	, WorldSpaceGravity(0.0f, 0.0f, 0.0f)
	, InitialFuelSecs(0.0f)
	, MaxLifeTimeSecs(0.0f)

	, HomingBehavior(ETobiiProjectileHomingBehavior::Acceleration)
	, SteeringMaxTurnSpeedDegPerSec(10.0f)
	
	, GuidanceSystem(ETobiiProjectileGuidanceSystem::ComplexPrediction)
	, GuidanceSystemUpdateFreq(0.0f)
	, GuidanceSystemMaximumTargetAngleDeg(0.0f)

	, GuidanceSystemTarget()
	, AccelerationVectorTowardsTarget()
	, bTargetOffCourse(false)
{
	bCanThrust = true;
	bUsesFuel = InitialFuelSecs > 0.0f;
	CurrentFuelSecs = InitialFuelSecs;
	CurrentLifeTime = 0.0f;
	GuidanceSystemUpdateTimerSecsLeft = 0.0f;
	LastTargetVelocity = FVector::ZeroVector;
	TargetAcceleration = FVector::ZeroVector;

	HomingAccelerationMagnitude = 10000.0f;
	PrimaryComponentTick.bCanEverTick = true;
}

void UTobiiProjectileComponent::BeginPlay()
{
	Super::BeginPlay();

	bCanThrust = true;
	bUsesFuel = InitialFuelSecs > 0.0f;
	CurrentFuelSecs = InitialFuelSecs;
	CurrentLifeTime = 0.0f;
	GuidanceSystemUpdateTimerSecsLeft = 0.0f;
	LastTargetVelocity = FVector::ZeroVector;
	TargetAcceleration = FVector::ZeroVector;
	if (WorldSpaceGravity.IsZero())
	{
		WorldSpaceGravity.Set(0.0f, 0.0f, GetGravityZ());
	}

	//This is a copy from the base class InitializeComponent function. This should really be done at beginplay to let any instigator configure initial speed etc.
	if (InitialVelocity.SizeSquared() > 0.f)
	{
		FVector NewVelocity;
		if (InitialSpeed > 0.f)
		{
			NewVelocity = InitialVelocity.GetSafeNormal() * InitialSpeed;
		}
		else
		{
			NewVelocity = InitialVelocity;
		}

		if (bInitialVelocityInLocalSpace)
		{
			SetVelocityInLocalSpace(NewVelocity);
		}

		if (bRotationFollowsVelocity)
		{
			if (UpdatedComponent)
			{
				UpdatedComponent->SetWorldRotation(NewVelocity.Rotation());
			}
		}

		UpdateComponentVelocity();

		if (UpdatedPrimitive && UpdatedPrimitive->IsSimulatingPhysics())
		{
			UpdatedPrimitive->SetPhysicsLinearVelocity(NewVelocity);
		}
	}
}

void UTobiiProjectileComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	CurrentLifeTime += DeltaTime;

	if (bUsesFuel && bCanThrust)
	{
		CurrentFuelSecs -= DeltaTime;
		if (CurrentFuelSecs <= 0.0f)
		{
			CurrentFuelSecs = 0.0f;
			bCanThrust = false;
		}
	}

	bTargetOffCourse = false;
	if(GuidanceSystemMaximumTargetAngleDeg > FLT_EPSILON && HomingTargetComponent.IsValid())
	{
		FVector TargetFocusPosition;
		UTobiiGTOMBlueprintLibrary::GetPrimitiveComponentFocusLocation(HomingTargetComponent.Get(), TargetFocusPosition);

		//If our target is too far off course, disable thrust
		FVector DeltaVector = TargetFocusPosition - GetOwner()->GetActorLocation();
		FVector MyDirection = Velocity;

		bool bHasValidVectors = DeltaVector.Normalize();
		bHasValidVectors = bHasValidVectors && MyDirection.Normalize();

		if (bHasValidVectors)
		{
			float SeparationAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(DeltaVector, MyDirection)));
			if (SeparationAngle > GuidanceSystemMaximumTargetAngleDeg)
			{
				bTargetOffCourse = true;
			}
		}
	}

	if (HomingTargetComponent.IsValid() && !bTargetOffCourse)
	{
		TargetAcceleration = HomingTargetComponent->GetComponentVelocity() - LastTargetVelocity;
		LastTargetVelocity = HomingTargetComponent->GetComponentVelocity();

		if (GuidanceSystemUpdateFreq == 0.0f)
		{
			SimulateGuidanceSystem();
		}
		else
		{
			GuidanceSystemUpdateTimerSecsLeft -= DeltaTime;
			if (GuidanceSystemUpdateTimerSecsLeft <= 0.0f)
			{
				GuidanceSystemUpdateTimerSecsLeft = 1.0f / GuidanceSystemUpdateFreq;
				SimulateGuidanceSystem();
			}
		}

		//Steering behavior
		if (bIsHomingProjectile && bCanThrust && HomingBehavior == ETobiiProjectileHomingBehavior::Steering)
		{
			FVector ForwardDir = Velocity;
			FVector TowardsTarget = GuidanceSystemTarget - GetOwner()->GetActorLocation();
			bool bHasValidVectors = ForwardDir.Normalize();
			bHasValidVectors = bHasValidVectors && TowardsTarget.Normalize();

			if (bHasValidVectors)
			{
				float AngularDifferenceRad = FMath::Acos(FVector::DotProduct(ForwardDir, TowardsTarget));
				float AlphaStep = FMath::Clamp(FMath::DegreesToRadians(SteeringMaxTurnSpeedDegPerSec) * DeltaTime / AngularDifferenceRad, 0.0f, 1.0f);
				FVector NewVelocityDirection = FQuat::Slerp(ForwardDir.ToOrientationQuat(), TowardsTarget.ToOrientationQuat(), AlphaStep).GetForwardVector();
				Velocity = NewVelocityDirection * Velocity.Size();
				UpdateComponentVelocity();
			}
		}
	}

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (MaxLifeTimeSecs > 0.0f && CurrentLifeTime > MaxLifeTimeSecs)
	{
		GetOwner()->Destroy();
	}
}

FVector UTobiiProjectileComponent::ComputeAcceleration(const FVector& InVelocity, float DeltaTime) const
{
	FVector Acceleration(FVector::ZeroVector);

	if (bAffectedByGravity)
	{
		Acceleration += WorldSpaceGravity;
	}

	if (bCanThrust && !bTargetOffCourse && bIsHomingProjectile && HomingTargetComponent.IsValid() && HomingBehavior == ETobiiProjectileHomingBehavior::Acceleration)
	{
		Acceleration += AccelerationVectorTowardsTarget;
	}

	return Acceleration;
}

void UTobiiProjectileComponent::SimulateGuidanceSystem()
{
	FVector TargetFocusPosition;
	UTobiiGTOMBlueprintLibrary::GetPrimitiveComponentFocusLocation(HomingTargetComponent.Get(), TargetFocusPosition);

	switch (GuidanceSystem)
	{
	case ETobiiProjectileGuidanceSystem::Prediction:
	case ETobiiProjectileGuidanceSystem::ComplexPrediction:
	{
		FTobiiAccelerationBasedHomingData InterceptData;
		InterceptData.ProjectilePosition = GetOwner()->GetActorLocation();
		InterceptData.ProjectileVelocity = Velocity;
		InterceptData.ProjectileAccelerationMagnitude = HomingAccelerationMagnitude;
		InterceptData.TargetPosition = TargetFocusPosition;
		InterceptData.TargetVelocity = HomingTargetComponent->GetComponentVelocity();
		InterceptData.TargetAcceleration = TargetAcceleration;
		InterceptData.bAttemptClosestApproachSolution = GuidanceSystem == ETobiiProjectileGuidanceSystem::ComplexPrediction;
		
		FTobiiAccelerationBasedHomingResult HomingResult;
		if (UTobiiInteractionsBlueprintLibrary::FindNeededAccelerationForAccelerationBasedHomingProjectile(InterceptData, HomingResult))
		{
			GuidanceSystemTarget = HomingResult.ExpectedInterceptLocation;
			AccelerationVectorTowardsTarget = HomingResult.SuggestedAcceleration;
		}
		else
		{
			GuidanceSystemTarget = TargetFocusPosition;
			AccelerationVectorTowardsTarget = (GuidanceSystemTarget - UpdatedComponent->GetComponentLocation()).GetSafeNormal() * HomingAccelerationMagnitude;
		}
		break;
	}
	case ETobiiProjectileGuidanceSystem::Simple:
	default:
	{
		GuidanceSystemTarget = TargetFocusPosition;
		AccelerationVectorTowardsTarget = (GuidanceSystemTarget - UpdatedComponent->GetComponentLocation()).GetSafeNormal() * HomingAccelerationMagnitude;
	}
	}
}

void UTobiiProjectileComponent::StartHoming(USceneComponent* NewHomingTargetComponent)
{
	HomingTargetComponent = NewHomingTargetComponent;
	if (HomingTargetComponent != nullptr)
	{
		bIsHomingProjectile = true;
	}

	bSimulationEnabled = true;
	if (UpdatedComponent == nullptr)
	{
		UpdatedComponent = GetOwner()->GetRootComponent();
	}
	if (Cast<UPrimitiveComponent>(UpdatedComponent) != nullptr)
	{
		Cast<UPrimitiveComponent>(UpdatedComponent)->SetSimulatePhysics(false);
	}
	PrimaryComponentTick.SetTickFunctionEnable(true);
}

void UTobiiProjectileComponent::StopHoming()
{
	bIsHomingProjectile = false;
	HomingTargetComponent = nullptr;
}
