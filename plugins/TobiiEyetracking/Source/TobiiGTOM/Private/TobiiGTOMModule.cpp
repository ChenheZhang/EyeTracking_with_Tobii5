/******************************************************************************
* Copyright 2017- Tobii Technology AB. All rights reserved.
*
* @author Temaran | Fredrik Lindh | fredrik.lindh@tobii.com | https://github.com/Temaran
******************************************************************************/

#include "TobiiGTOMModule.h"
#include "TobiiGazeFocusableComponent.h"
#include "TobiiGazeFocusableWidget.h"
#include "TobiiGTOMBlueprintLibrary.h"
#include "TobiiGTOMInternalTypes.h"
#include "TobiiGTOMEngine.h"

#include "IEyeTracker.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/PlayerController.h"
#include "Misc/Paths.h"
#include "Runtime/Engine/Public/Slate/SceneViewport.h"

IMPLEMENT_MODULE(FTobiiGTOMModule, TobiiGTOM)

void FTobiiGTOMModule::StartupModule()
{
	G2OMDllHandle = nullptr;

#if TOBII_EYETRACKING_ACTIVE
	FString RelativeG2OMDllPath = FString(TEXT(TOBII_G2OM_RELATIVE_DLL_PATH));

#if TOBII_COMPILE_AS_ENGINE_PLUGIN
	FString FullG2OMDllPath = FPaths::Combine(FPaths::EngineDir(), RelativeG2OMDllPath);
#else
	FString FullG2OMDllPath = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectPluginsDir(), RelativeG2OMDllPath));
#endif //TOBII_COMPILE_AS_ENGINE_PLUGIN

	G2OMDllHandle = FPlatformProcess::GetDllHandle(*FullG2OMDllPath);

	if (G2OMDllHandle != nullptr)
	{
		IModularFeatures::Get().RegisterModularFeature(IInputDeviceModule::GetModularFeatureName(), this);
	}
#endif //TOBII_EYETRACKING_ACTIVE
}

void FTobiiGTOMModule::ShutdownModule()
{
#if TOBII_EYETRACKING_ACTIVE
	if (G2OMDllHandle != nullptr)
	{
		IModularFeatures::Get().UnregisterModularFeature(IInputDeviceModule::GetModularFeatureName(), this);
	}
	G2OMDllHandle = nullptr;
#endif //TOBII_EYETRACKING_ACTIVE
}

TSharedPtr<class IInputDevice> FTobiiGTOMModule::CreateInputDevice(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler)
{
	GTOMInputDevice = MakeShareable(new FTobiiGTOMEngine());
	return TSharedPtr<class IInputDevice>(GTOMInputDevice);
}
