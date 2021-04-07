/******************************************************************************
* Copyright 2017- Tobii Technology AB. All rights reserved.
*
* @author Temaran | Fredrik Lindh | fredrik.lindh@tobii.com | https://github.com/Temaran
******************************************************************************/

#include "TobiiPlatformSpecific.h"

#include "Framework/Application/SlateApplication.h"

#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#include "dbt.h"

struct MonitorHandleContext
{
	FString DeviceName;
	HMONITOR hMonitorHandle;
};

BOOL CALLBACK EnumDisplayMonitorsProc(HMONITOR hMonitorHandle, HDC, LPRECT, LPARAM Data)
{
	MonitorHandleContext* Context = (MonitorHandleContext*)Data;
	MONITORINFOEXA MonitorInfo;
	MonitorInfo.cbSize = sizeof(MonitorInfo);
	int res = GetMonitorInfoA(hMonitorHandle, &MonitorInfo);
	if (res && _stricmp(MonitorInfo.szDevice, TCHAR_TO_ANSI(*Context->DeviceName)) == 0)
	{
		Context->hMonitorHandle = hMonitorHandle;
		return FALSE;
	}
	return TRUE;
}

void* FTobiiPlatformSpecific::GetMonitorInformation(const void* GameWindowHandle, int& MonitorWidthPx, int& MonitorHeightPx)
{
	HWND hGameWindowHandle = (HWND)GameWindowHandle;
	HMONITOR hGameMonitor = MonitorFromWindow(hGameWindowHandle, MONITOR_DEFAULTTONEAREST);
	MONITORINFO MonitorInfo;
	MonitorInfo.cbSize = sizeof(MONITORINFO);
	if (GetMonitorInfo(hGameMonitor, &MonitorInfo))
	{
		MonitorWidthPx = MonitorInfo.rcMonitor.right - MonitorInfo.rcMonitor.left;
		MonitorHeightPx = MonitorInfo.rcMonitor.bottom - MonitorInfo.rcMonitor.top;
		return (void*)hGameMonitor;
	}

	MonitorWidthPx = -1;
 	MonitorHeightPx = -1;
	return nullptr;
}

void* FTobiiPlatformSpecific::MonitorHandleFromDeviceName(FString DeviceName)
{
 	MonitorHandleContext Context;
 	Context.DeviceName = DeviceName;
 	Context.hMonitorHandle = nullptr;
	EnumDisplayMonitors(nullptr, nullptr, EnumDisplayMonitorsProc, (LPARAM)&Context);
	return (void*)Context.hMonitorHandle;
}

bool FTobiiPlatformSpecific::ConvertGazeCoordinateToVirtualDesktopPixel(void* GameWindowHandle, const FVector2D& ClientCoordsUNorm, FIntPoint& OutVirtualDesktopPixel)
{
	//Now we can try to convert our point
	RECT ClientRect;
	if (GetClientRect((HWND)GameWindowHandle, &ClientRect))
	{
		POINT ClientPoint;
		ClientPoint.x = (int32)(ClientCoordsUNorm.X * (ClientRect.right - ClientRect.left));
		ClientPoint.y = (int32)(ClientCoordsUNorm.Y * (ClientRect.bottom - ClientRect.top));
		if (ClientToScreen((HWND)GameWindowHandle, &ClientPoint))
		{
			OutVirtualDesktopPixel.X = ClientPoint.x;
			OutVirtualDesktopPixel.Y = ClientPoint.y;
			return true;
		}
	}

	return false;
}

FTobiiPlatformNotifications::FTobiiPlatformNotifications()
	: bShouldForceEyetrackerReconnect(true)
	, bShouldUpdateGameMonitorHandle(true)
{
	if (FSlateApplication::IsInitialized())
	{
		const TSharedPtr<GenericApplication> Application = FSlateApplication::Get().GetPlatformApplication();
		if (Application.IsValid())
		{
			FWindowsApplication* WindowsApplication = (FWindowsApplication*)Application.Get();
			WindowsApplication->AddMessageHandler(*this);
		}
	}
}

FTobiiPlatformNotifications::~FTobiiPlatformNotifications()
{
	if (FSlateApplication::IsInitialized())
	{
		const TSharedPtr<GenericApplication> Application = FSlateApplication::Get().GetPlatformApplication();
		if (Application.IsValid())
		{
			FWindowsApplication* WindowsApplication = (FWindowsApplication*)Application.Get();
			WindowsApplication->RemoveMessageHandler(*this);
		}
	}
}

bool FTobiiPlatformNotifications::ProcessMessage(HWND hwnd, uint32 msg, WPARAM wParam, LPARAM lParam, int32& OutResult)
{
	switch (msg)
	{
	case WM_DEVICECHANGE:
	{
		if (wParam == DBT_DEVICEARRIVAL)
		{
			//This is narrow enough for us. We just want to short-circuit the reconnection timer if someone plugged in an eyetracker so they don't have to wait the full reconnection time.
			bShouldForceEyetrackerReconnect = true;
		}
		break;
	}

	case WM_DISPLAYCHANGE:
	{
		bShouldUpdateGameMonitorHandle = true;
		break;
	}
 	}

	return false;
}

#include "Runtime/Core/Public/Windows/HideWindowsPlatformTypes.h"
#endif
