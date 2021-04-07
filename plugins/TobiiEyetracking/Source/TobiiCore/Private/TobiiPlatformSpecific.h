/******************************************************************************
* Copyright 2017- Tobii Technology AB. All rights reserved.
*
* @author Temaran | Fredrik Lindh | fredrik.lindh@tobii.com | https://github.com/Temaran
******************************************************************************/

#pragma once

#include "CoreMinimal.h"

class FTobiiPlatformSpecific
{
public:
	static void* GetMonitorInformation(const void* GameWindowHandle, int& MonitorWidthPx, int& MonitorHeightPx);
	static void* MonitorHandleFromDeviceName(FString DeviceName);
	
	static bool ConvertGazeCoordinateToVirtualDesktopPixel(void* GameWindowHandle, const FVector2D& ClientCoordsUNorm, FIntPoint& OutVirtualDesktopPixel);
}; 

#if PLATFORM_WINDOWS
#include "Windows/WindowsApplication.h"
#include "HAL/ThreadSafeBool.h"

class FTobiiPlatformNotifications : public IWindowsMessageHandler
{
public:
	FTobiiPlatformNotifications();
	virtual ~FTobiiPlatformNotifications();

	virtual bool ProcessMessage(HWND hwnd, uint32 msg, WPARAM wParam, LPARAM lParam, int32& OutResult) override;

public:
	FThreadSafeBool bShouldForceEyetrackerReconnect;
	FThreadSafeBool bShouldUpdateGameMonitorHandle;
	
};
#elif PLATFORM_MAC
#else
#endif