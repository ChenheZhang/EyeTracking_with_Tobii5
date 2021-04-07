/******************************************************************************
* Copyright 2017- Tobii Technology AB. All rights reserved.
*
* @author Temaran | Fredrik Lindh | fredrik.lindh@tobii.com | https://github.com/Temaran
******************************************************************************/

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "TobiiAimAtGazeComponent.generated.h"

UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class TOBIIINTERACTIONS_API UTobiiAimAtGazeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTobiiAimAtGazeComponent();

public:
	//This controls how the filters in FocusFilterList are used. If this is true, only focusables with a layer that is in the FocusFilterList will be considered. If this is false, the FocusFilterList will act like a black list, excluding any focusables whos layer can be found in the array.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gaze Focus Management")
	bool bIsWhiteList;

	//These are the filters that will determine what focus data you will receive. The behavior of the filters are controlled by bIsWhiteList.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gaze Focus Management")
	TArray<FName> FocusLayerFilters;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aim@gaze")
	bool bAllowRetarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aim@gaze")
	float AimSpeed;

public:
	//Use this to test if the user has turned aim@gaze on and it is available.
	UFUNCTION(BlueprintPure, Category = "Aim@gaze")
	bool AimAtGazeAvailable();

	//Trigger an aim@gaze
	UFUNCTION(BlueprintCallable, Category = "Aim@gaze")
	void AimAtGaze();

	//Call this every frame you want to continually track the aim@gaze target
	UFUNCTION(BlueprintCallable, Category = "Aim@gaze")
	void ContinuousAimAtGaze();

	/************************************************************************/
	/* UActorComponent                                                      */
	/************************************************************************/
public:
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

private:
	TWeakObjectPtr<class UPrimitiveComponent> CurrentFocusComponent;
	FVector CurrentAimTarget;
	bool bIsGazeAiming;
};
