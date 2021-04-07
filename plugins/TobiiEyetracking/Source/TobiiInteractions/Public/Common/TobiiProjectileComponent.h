/******************************************************************************
* Copyright 2017- Tobii Technology AB. All rights reserved.
*
* @author Temaran | Fredrik Lindh | fredrik.lindh@tobii.com | https://github.com/Temaran
******************************************************************************/

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

#include "TobiiProjectileComponent.generated.h"

UENUM()
enum class ETobiiProjectileHomingBehavior
{
	//This is the default behavior. The projectile will accelerate towards the target. This also means the projectile will speed up. If this is not what you want, consider using another behavior.
	Acceleration,

	//In this mode, imagine the projectile has side thrusters to enable it to turn gradually towards the target. This behavior preserves projectile speed.
	Steering
};

UENUM()
enum class ETobiiProjectileGuidanceSystem
{
	//This is the default behavior. The projectile will home towards the target's last known location.
	Simple,

	//This mode will predict possible impact times and locations to avoid orbiting. Considers position, velocity and acceleration. Is quite expensive.
	Prediction,

	//In addition to normal prediction, this guidance system mode will also calculate the closest approach if no direct hit is possible. More expensive than standard prediction
	ComplexPrediction
};

UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent), ClassGroup=(Tobii))
class UTobiiProjectileComponent : public UProjectileMovementComponent
{
	GENERATED_BODY()

public:	
	//Use this if you need to set starting values from the outside, don't use Velocity.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	FVector InitialVelocity;

	//If this is true, gravity acceleration will be added to the projectile.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	bool bAffectedByGravity;

	//If bAffectedByGravity is true, this is the gravity acceleration vector that will be applied to the projectile. If this is zero, the default GetGravityZ will be used.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	FVector WorldSpaceGravity;

	//If fuel runs out, homing behavior stops for the projectile. 0 means infinite fuel.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float InitialFuelSecs;

	//The projectile is destroyed if it manages to stay alive for this amount of time. 0 means infinite lifetime.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float MaxLifeTimeSecs;

public:
	//Allows you to customize the homing behavior if you have homing projectiles on. The homing target will always be set to the user's gaze target.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Homing")
	ETobiiProjectileHomingBehavior HomingBehavior;

	//This is the fastest the projectile is able to turn when using the steering homing behavior.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Homing")
	float SteeringMaxTurnSpeedDegPerSec;

public:
	//Allows you to customize the targeting part of the homing behavior. The different options generally provide different trade offs between performance and accuracy. Good accuracy also helps to avoid orbiting projectiles.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guidance System")
	ETobiiProjectileGuidanceSystem GuidanceSystem;

	//This is how often the guidance system will update it's target location. A higher frequency will be more expensive but more accurate and vice versa. The guidance system will never update more than once per frame. 0 means every frame.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guidance System")
	float GuidanceSystemUpdateFreq;

	//Sometimes you might not want the homing behavior to track targets that are too far off course. This is the maximium angle allowed between the projectile forward vector and the vector towards the target. If this threshold is breached, the homing will stop. 0 means there is no threshold.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guidance System")
	float GuidanceSystemMaximumTargetAngleDeg;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Guidance System")
	FVector GuidanceSystemTarget;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Guidance System")
	FVector AccelerationVectorTowardsTarget;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Guidance System")
	bool bTargetOffCourse;

public:
	//This function will attempt to set up the actor to start homing. This entails disabling physics simulation etc. Please note that if the actor is intersecting the ground or other geometry, the simulation might stop immediately since it will "hit" it. To avoid this, set the appropriate collision masks or turn off collision during launch.
	UFUNCTION(BlueprintCallable, Category = "Homing")
	void StartHoming(USceneComponent* NewHomingTargetComponent);

	UFUNCTION(BlueprintCallable, Category = "Homing")
	void StopHoming();

public:	
	UTobiiProjectileComponent();
	
public:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual FVector ComputeAcceleration(const FVector& InVelocity, float DeltaTime) const override;
	virtual void SimulateGuidanceSystem();

private:
	bool bCanThrust;
	bool bUsesFuel;
	float CurrentFuelSecs;
	float CurrentLifeTime;
	float GuidanceSystemUpdateTimerSecsLeft;	
	FVector LastTargetVelocity;
	FVector TargetAcceleration;
};
