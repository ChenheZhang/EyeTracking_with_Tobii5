/******************************************************************************
* Copyright 2017- Tobii Technology AB. All rights reserved.
*
* @author Temaran | Fredrik Lindh | fredrik.lindh@tobii.com | https://github.com/Temaran
******************************************************************************/

#pragma once

#include "TobiiGTOMOcclusionTester.h"
#include "TobiiGazeFocusableWidget.h"
#include "TobiiGTOMTypes.h"
#include "tobii_g2om.h"

#include "CoreMinimal.h"
#include "Runtime/InputDevice/Public/IInputDevice.h"

class TOBIIGTOM_API FTobiiGTOMEngine : public IInputDevice
{
public:
	FTobiiGTOMEngine();
	virtual ~FTobiiGTOMEngine();

	FHitResult CombinedWorldGazeHitData;
	TWeakObjectPtr<APlayerController> GTOMPlayerController;

	const TArray<FTobiiGazeFocusData>& GetFocusData() { return G2OMFocusResults; }
	void EmulateGazeFocus(TArray<FTobiiGazeFocusData>& EmulatedFocusData);

// IInputDevice
public:	
	virtual void Tick(float DeltaTime) override;
	virtual void SendControllerEvents() override { }
	virtual void SetMessageHandler(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler) override { }
	virtual bool Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar) override { return true; }
	virtual void SetChannelValue(int32 ControllerId, FForceFeedbackChannelType ChannelType, float Value) override { }
	virtual void SetChannelValues(int32 ControllerId, const FForceFeedbackValues &values) override { }

private:
	g2om_context* G2OMContext;
	g2om_raycast_result G2OMRaycastResults;
	g2om_gaze_data G2OMGazeData;
	FTobiiGTOMOcclusionTester OcclusionTester;
	TArray<FTobiiGazeFocusData> G2OMFocusResults;

	TWeakObjectPtr<UPrimitiveComponent> PreviouslyFocusedPrimitiveComponent;
	TWeakObjectPtr<UTobiiGazeFocusableWidget> PreviouslyFocusedWidget;

	void UpdateWinners(UPrimitiveComponent* NewTopFocusPrimitive, UTobiiGazeFocusableWidget* NewTopFocusWidget);

	void NotifyPrimitiveComponentGazeFocusReceived(UPrimitiveComponent& PrimitiveComponentToReceiveFocus);
	void NotifyPrimitiveComponentGazeFocusLost(UPrimitiveComponent& PrimitiveComponentToLoseFocus);
	void NotifyWidgetGazeFocusReceived(UTobiiGazeFocusableWidget& WidgetToReceiveFocus);
	void NotifyWidgetGazeFocusLost(UTobiiGazeFocusableWidget& WidgetToLoseFocus);
	void FillG2OMRaycast(const FHitResult& HitResult, const FVector2D& ScreenGazePointUNorm, const FVector& GazeDirection, g2om_raycast& RaycastToModify);
};
