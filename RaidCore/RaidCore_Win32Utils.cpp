/* ========================================================================
$File: $
$Date: $
$Revision: $
$Creator: Roman Markovetski $
$Notice: (C) Copyright 2014 by Farmland Blossoms. All Rights Reserved. $
======================================================================== */

// --
#include "Headers/RaidCore_UserInputHdr.h"
#include "Headers/RaidCore_RenderHandler.h"
#include "Headers/RaidCore_Platform.h"
#include "Headers/RaidCore_UtilsHeader.h"
#include "Headers/RaidCore_Task.h"
#include "Headers/RaidCore_ImageUtils.h"
#include "Headers/RaidCore_MathOperators.h"

#include "OSHeaders\RaidCore_Win32_Struct.h"
#include <stdio.h>

BOOL findSecondaryMonitor(HMONITOR Arg1, HDC Arg2, LPRECT Arg3, LPARAM Arg4) {
    if (NULL != Arg4) {
        MONITORINFO mi = { sizeof(MONITORINFO) };
        if (GetMonitorInfo(Arg1, &mi)) {
            RECT* createRc = (RECT*)Arg4;
            if ((MONITORINFOF_PRIMARY & mi.dwFlags) == 0) {
                createRc->left = mi.rcWork.left;
                createRc->top = mi.rcWork.top;
                if ((mi.rcWork.right - mi.rcWork.left) < createRc->right) {
                    createRc->right = (mi.rcWork.right - mi.rcWork.left);
                }
                if ((mi.rcWork.bottom - mi.rcWork.top) < createRc->bottom) {
                    createRc->bottom = (mi.rcWork.bottom - mi.rcWork.top);
                }
                return FALSE;
            }
        }
    }
    return TRUE;
}

bool32 game_render_engine::createWindow(uint32 width, uint32 height, const char* className, game_render_engine::PlatformWindow& window, uint32 flags) {
    if (0 == width) {
        width = CW_USEDEFAULT;
    }
    if (0 == height) {
        height = CW_USEDEFAULT;
    }

    RECT createRc = { 0, 0, width, height };
    HDC hdc = GetDC(NULL);
    EnumDisplayMonitors(hdc, NULL, findSecondaryMonitor, (LPARAM)&createRc);
    ReleaseDC(NULL, hdc);

    if (NULL == window.hWnd || INVALID_HANDLE_VALUE == window.hWnd) {
        window.hWnd = CreateWindowEx(WS_EX_APPWINDOW, className, RC_OPENGL_APP_TITLE, flags,
                                     createRc.left, createRc.top, createRc.right, createRc.bottom, NULL, NULL, window.hInst, NULL);
    }

    return ((NULL == window.hWnd) ? false32 : true32);
}
/////////////////////////////////////////////////////////////////////////////////////////////////
//
