/******************************************************************************
* Copyright 2017- Tobii Technology AB. All rights reserved.
*
* @author Temaran | Fredrik Lindh | fredrik.lindh@tobii.com | https://github.com/Temaran
******************************************************************************/

#pragma once

#if WITH_EDITOR

#include "../TobiiCoreModule.h"

#include "CoreMinimal.h"
#include "SWebBrowser.h"
#include "Widgets/SWindow.h"

class STobiiLicenseWindow : public SWindow
{
public:
	SLATE_BEGIN_ARGS(STobiiLicenseWindow) {	}
	SLATE_END_ARGS()

	STobiiLicenseWindow() {}

	/** Widget constructor */
	void Construct(const FArguments& Args, FTobiiCoreModule* InCoreModule);

private:
	TSharedPtr<SWebBrowser> MainBrowser;
	FTobiiCoreModule* CoreModule;

	FReply AcceptLicense();
	FReply ShutdownEditor();
};

#endif //WITH_EDITOR
