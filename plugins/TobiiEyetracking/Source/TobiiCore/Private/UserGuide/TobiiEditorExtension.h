/******************************************************************************
* Copyright 2017- Tobii Technology AB. All rights reserved.
*
* @author Temaran | Fredrik Lindh | fredrik.lindh@tobii.com | https://github.com/Temaran
******************************************************************************/

#pragma once

#if WITH_EDITOR

#include "../TobiiCoreModule.h"

#include "CoreMinimal.h"

class FTobiiEditorExtension
{
public:
	FTobiiEditorExtension(FTobiiCoreModule* InCoreModule);
	~FTobiiEditorExtension();
	void Initialize();

private:
	FTobiiCoreModule* CoreModule;
	FDelegateHandle LoadedDelegateHandle;
	bool bIsEditorInitialized;

	void OnEditorLoaded(SWindow& SlateWindow, void* ViewportRHIPtr);
};

#endif //WITH_EDITOR
