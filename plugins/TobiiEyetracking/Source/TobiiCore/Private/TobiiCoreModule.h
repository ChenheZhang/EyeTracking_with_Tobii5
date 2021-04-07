/******************************************************************************
* Copyright 2017- Tobii Technology AB. All rights reserved.
*
* @author Temaran | Fredrik Lindh | fredrik.lindh@tobii.com | https://github.com/Temaran
******************************************************************************/

#pragma once

#include "IInputDevice.h"
#include "ITobiiCore.h"
#include "TobiiEyetracker.h"

#include "CoreMinimal.h"
#include "DisplayDebugHelpers.h"

class AHUD;
class UCanvas;

class FTobiiCoreModule : public ITobiiCore
{
	/************************************************************************/
	/* ITobiiCore                                                           */
	/************************************************************************/
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	virtual TSharedPtr<class IEyeTracker, ESPMode::ThreadSafe> CreateEyeTracker() override;
	virtual bool IsEyeTrackerConnected() const override;

protected:
	virtual TSharedPtr<ITobiiEyeTracker, ESPMode::ThreadSafe> GetEyeTrackerInternal() override;

private:
	TSharedPtr<ITobiiEyeTracker, ESPMode::ThreadSafe> EyeTracker;
	void* GICDllHandle;

#if WITH_EDITOR
	class FTobiiEditorExtension* EditorExtensions;
#endif
};
