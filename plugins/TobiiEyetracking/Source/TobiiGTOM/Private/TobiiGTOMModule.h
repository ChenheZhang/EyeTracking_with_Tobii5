/******************************************************************************
* Copyright 2017- Tobii Technology AB. All rights reserved.
*
* @author Temaran | Fredrik Lindh | fredrik.lindh@tobii.com | https://github.com/Temaran
******************************************************************************/

#pragma once

#include "CoreMinimal.h"
#include "Runtime/InputDevice/Public/IInputDeviceModule.h"
#include "Modules/ModuleManager.h"

class TOBIIGTOM_API FTobiiGTOMModule : public IInputDeviceModule
{
public:
	static inline FTobiiGTOMModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FTobiiGTOMModule>("TobiiGTOM");
	}

	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("TobiiGTOM");
	}

public:
	TSharedPtr<class FTobiiGTOMEngine> GTOMInputDevice;

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	virtual TSharedPtr< class IInputDevice > CreateInputDevice(const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler);

private:
	void* G2OMDllHandle;
};
