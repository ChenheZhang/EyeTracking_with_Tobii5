/******************************************************************************
* Copyright 2017- Tobii Technology AB. All rights reserved.
*
* @author Temaran | Fredrik Lindh | fredrik.lindh@tobii.com | https://github.com/Temaran
******************************************************************************/

#pragma once

#if WITH_EDITOR

#include "CoreMinimal.h"
#include "SWebBrowser.h"
#include "Widgets/SWindow.h"

class STobiiWelcomeWindow : public SWindow
{
public:
	SLATE_BEGIN_ARGS(STobiiWelcomeWindow)
	{
	}
	SLATE_END_ARGS()

		STobiiWelcomeWindow() {}

	/** Widget constructor */
	void Construct(const FArguments& Args);

private:
	TSharedPtr<SWebBrowser> MainBrowser;
	bool bDontShowAgain;

	void UpdateFlag();
	FReply ShowDesktopSamples();
	FReply ShowXRSamples();
	FReply AbortTutorial();
};

#endif //WITH_EDITOR
