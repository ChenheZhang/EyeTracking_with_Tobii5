/******************************************************************************
* Copyright 2017- Tobii Technology AB. All rights reserved.
*
* @author Temaran | Fredrik Lindh | fredrik.lindh@tobii.com | https://github.com/Temaran
******************************************************************************/

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"

#include "TobiiGTOMTypes.generated.h"

class UTobiiGazeFocusableWidget;
typedef uint32 FTobiiFocusableUID;		//Only use this ID type for our focus container types like UTobiiGazeFocusableComponents and UTobiiGazeFocusableWidgets
typedef uint32 FEngineFocusableUID;		//Only use this for base engine things that can be focused, like UPrimitiveComponents and UWidgets

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPrimitiveReceivedGazeFocusSignature, UPrimitiveComponent*, FocusedComponent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPrimitiveLostGazeFocusSignature, UPrimitiveComponent*, FocusedComponent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWidgetReceivedGazeFocusSignature, UTobiiGazeFocusableWidget*, FocusedWidget);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWidgetLostGazeFocusSignature, UTobiiGazeFocusableWidget*, FocusedWidget);

UENUM(BlueprintType)
enum class ETobiiCleanUIMode : uint8
{
	/** CleanUI is disabled for this widget. */
	Disabled,

	/** CleanUI will only consider the gaze point for determining alpha, and will ignore nearby widgets. */
	Normal,

	/** This widget is aware of it's neighbors and only one widget can be faded in at a time. */
	FocusExclusive,

	/** This widget will still calcualte its CleanUI alpha value, but it will not apply it when calculating its own color. */
	Silent
};

/**
  * Information about an object that is being considered when determining user focus.
  * When utilizing gaze, this is almost always what you should be using as your input data.
  */
USTRUCT(BlueprintType)
struct FTobiiGazeFocusData
{
	GENERATED_USTRUCT_BODY()

public:
	FTobiiGazeFocusData()
		: FocusedActor()
		, FocusedPrimitiveComponent()
		, FocusedWidget()
		, LastVisibleWorldLocation(0.0f, 0.0f, 0.0f)
		, FocusConfidence(0.0f)
	{}

	//This is the actor that the Focused Primitive Component belongs to.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Focus Data")
	TWeakObjectPtr<AActor> FocusedActor;

	//This is the primitive component that is most likely to hold the user's focus.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Focus Data")
	TWeakObjectPtr<UPrimitiveComponent> FocusedPrimitiveComponent;

	//This is the primitive component that is most likely to hold the user's focus.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Focus Data")
	TWeakObjectPtr<UTobiiGazeFocusableWidget> FocusedWidget;

	//This is the point on the Focused Primitive Component that was last confirmed visible to the user. This is very useful when aligning objects towards a focused object.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Focus Data")
	FVector LastVisibleWorldLocation;

	//This is how confident the focus system is that this object is in focus. The object with the highest confidence is not necessarily the object with focus however.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Focus Data")
	float FocusConfidence;
};

//This contains the tags that can optionally be used to inform the GTOM system about the primitive component they are attached to. These only exist since we cannot add UPROPERTY's to primitive components without engine modifications. 
//If you want to override behavior or widgets, please change the relevant properties on the TobiiGazeFocusableWidget since we have full access to that type.
class TOBIIGTOM_API FTobiiPrimitiveComponentGazeFocusTags
{
public:
	static FName HasGazeFocusTag;						//A primitive component with this tag is the primitive component that most likely has focus irregardless of layer filters.
	static FName GazeFocusableTag;						//A primitive component with this tag will participate in GTOM even if the actor owning the primitive doesn't have a GazeFocusableComponent, or if the default for the gaze focusable is off.
	static FName NotGazeFocusableTag;					//A primitive component with this tag will not participate in GTOM even if the actor owning the primitive component has a GazeFocusableComponent.
	static FString GazeFocusablePriorityTag;			//A primitive component with this tag will modify it's priority. Priority is used when only a certain number of focusables can participate in an operation. An example of this is ID buffer based GTOM. This is an argument tag, this means you must provide the value after the tag. Usage example: GazeFocusablePriorityTag 100
	static FString GazeFocusableMaximumDistanceTag;		//A primitive component with this tag and has the GazeFocusableTag will only be considered if the distance between the GTOM source and the component is shorter than the argument part of the tag. Please note that this tag cannot be used to force the GTOM line traces to be longer than default, only exclude the primitive if the distance is longer than this argument. This is an argument tag, this means you must provide the value after the tag. Usage example: GazeFocusableMaximumDistanceTag 10000
	static FString GazeFocusableLayerTag;				//A focus manager can opt to only query a subset of focusables by using a focus layer. A primitive will this tag will only be subject to the layer supplied as the argument. This is an argument tag, this means you must provide the value after the tag. Usage example: GazeFocusableLayerTag Enemies

	static FString PrimitiveFocusOffsetXTag;			//If this tag is present, it tells the various systems interacting with gaze focus that the actual focus point is relative to the primitive origin by this amount.
	static FString PrimitiveFocusOffsetYTag;			//If this tag is present, it tells the various systems interacting with gaze focus that the actual focus point is relative to the primitive origin by this amount.
	static FString PrimitiveFocusOffsetZTag;			//If this tag is present, it tells the various systems interacting with gaze focus that the actual focus point is relative to the primitive origin by this amount.
};
