/******************************************************************************
* Copyright 2017- Tobii Technology AB. All rights reserved.
*
* @author Temaran | Fredrik Lindh | fredrik.lindh@tobii.com | https://github.com/Temaran
******************************************************************************/

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "TobiiFireAtGazeComponent.generated.h"

UENUM(BlueprintType)
enum class ETobiiFireAtGazeNoTargetBehavior : uint8
{
	PointGunForward,
	PointGunToGaze
};

UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class TOBIIINTERACTIONS_API UTobiiFireAtGazeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTobiiFireAtGazeComponent();

public:
	//This controls how the filters in FocusFilterList are used. If this is true, only focusables with a layer that is in the FocusFilterList will be considered. If this is false, the FocusFilterList will act like a black list, excluding any focusables whos layer can be found in the array.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gaze Focus Management")
	bool bIsWhiteList;

	//These are the filters that will determine what focus data you will receive. The behavior of the filters are controlled by bIsWhiteList.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gaze Focus Management")
	TArray<FName> FocusLayerFilters;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire@gaze")
	class UCameraComponent* CameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fire@gaze")
	TWeakObjectPtr<AActor> FireAtGazeTargetActor;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fire@gaze")
	TWeakObjectPtr<UPrimitiveComponent> FireAtGazeTargetComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fire@gaze")
	FVector FireAtGazeTargetLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire@gaze Settings")
	float MaxDistance;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire@gaze Settings")
	ETobiiFireAtGazeNoTargetBehavior NoTargetBehavior;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire@gaze Settings")
	TEnumAsByte<ECollisionChannel> TraceChannel;

public:
	//Use this to test if the user has turned fire@gaze on and it is available.
	UFUNCTION(BlueprintPure, Category = "Fire@gaze")
	bool FireAtGazeAvailable();

	UFUNCTION(BlueprintPure, Category = "Fire@gaze")
	bool WantsCrosshair();

	/************************************************************************/
	/* UActorComponent                                                      */
	/************************************************************************/
public:
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
};
