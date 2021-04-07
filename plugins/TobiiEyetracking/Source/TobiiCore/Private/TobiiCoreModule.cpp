/******************************************************************************
* Copyright 2017- Tobii Technology AB. All rights reserved.
*
* @author Temaran | Fredrik Lindh | fredrik.lindh@tobii.com | https://github.com/Temaran
******************************************************************************/

#include "TobiiCoreModule.h"

#include "UserGuide/TobiiEditorExtension.h"

#include "IEyeTrackerModule.h"
#include "GameFramework/HUD.h"
#include "Misc/Paths.h"

IMPLEMENT_MODULE(FTobiiCoreModule, TobiiCore)

/************************************************************************/
/* FTobiiCoreModule                                                     */
/************************************************************************/
void FTobiiCoreModule::StartupModule()
{
	EyeTracker.Reset();

#if TOBII_EYETRACKING_ACTIVE
	FString RelativeGICDllPath = FString(TEXT(TOBII_GIC_RELATIVE_DLL_PATH)); 

#if TOBII_COMPILE_AS_ENGINE_PLUGIN
	FString FullGICDllPath = FPaths::Combine(FPaths::EngineDir(), RelativeGICDllPath);
#else
	FString FullGICDllPath = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectPluginsDir(), RelativeGICDllPath));
#endif //TOBII_COMPILE_AS_ENGINE_PLUGIN

	GICDllHandle = FPlatformProcess::GetDllHandle(*FullGICDllPath);

	if (GICDllHandle != nullptr)
	{
		FTobiiEyeTracker* NewEyeTracker = new FTobiiEyeTracker();
		if (NewEyeTracker != nullptr && NewEyeTracker->IsConnectedToEyeTracker())
		{
			IModularFeatures::Get().RegisterModularFeature(GetModularFeatureName(), this);
			EyeTracker = TSharedPtr<ITobiiEyeTracker, ESPMode::ThreadSafe>(NewEyeTracker);
		}

#if WITH_EDITOR
		EditorExtensions = new FTobiiEditorExtension(this);
#endif //WITH_EDITOR
	}
#endif //TOBII_EYETRACKING_ACTIVE
}

void FTobiiCoreModule::ShutdownModule()
{
#if WITH_EDITOR
	if (EditorExtensions != nullptr)
	{
		delete EditorExtensions;
	}
#endif

	if (EyeTracker.IsValid())
	{
		IModularFeatures::Get().UnregisterModularFeature(GetModularFeatureName(), this);

		FTobiiEyeTracker* EyetrackerPtr = (FTobiiEyeTracker*)EyeTracker.Get();
		EyetrackerPtr->Shutdown();
		EyeTracker.Reset();
	}
}

TSharedPtr<IEyeTracker, ESPMode::ThreadSafe> FTobiiCoreModule::CreateEyeTracker()
{
	return StaticCastSharedPtr<IEyeTracker, ITobiiEyeTracker, ESPMode::ThreadSafe>(EyeTracker);
}

bool FTobiiCoreModule::IsEyeTrackerConnected() const
{
	bool bSRanipalRunning = FModuleManager::Get().IsModuleLoaded("SRanipalCore") && FModuleManager::Get().GetModuleChecked<IEyeTrackerModule>("SRanipalCore").IsEyeTrackerConnected();
	return EyeTracker.IsValid() && !bSRanipalRunning;
}

TSharedPtr<ITobiiEyeTracker, ESPMode::ThreadSafe> FTobiiCoreModule::GetEyeTrackerInternal()
{
	return EyeTracker;
}
