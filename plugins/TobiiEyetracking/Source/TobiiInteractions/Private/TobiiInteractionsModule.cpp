/******************************************************************************
* Copyright 2017- Tobii Technology AB. All rights reserved.
*
* @author Temaran | Fredrik Lindh | fredrik.lindh@tobii.com | https://github.com/Temaran
******************************************************************************/

#include "TobiiInteractionsModule.h"
#include "TobiiInteractionsStyle.h"

#include "GameFramework/HUD.h"

IMPLEMENT_MODULE(FTobiiInteractionsModule, TobiiInteractions);

void FTobiiInteractionsModule::StartupModule()
{
#if WITH_EDITOR	
	if (GIsEditor)
	{
		FTobiiInteractionsStyle::Initialize();
	}
#endif
}

void FTobiiInteractionsModule::ShutdownModule()
{
#if WITH_EDITOR
	if (GIsEditor)
	{
		FTobiiInteractionsStyle::Shutdown();
	}
#endif
}
