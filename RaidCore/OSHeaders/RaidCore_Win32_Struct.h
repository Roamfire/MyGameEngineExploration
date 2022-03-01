/* ========================================================================
$File: $
$Date: $
$Revision: $
$Creator: Roman Markovetski $
$Notice: (C) Copyright 2014 by Farmland Blossoms. All Rights Reserved. $
======================================================================== */

/*
DO NOT INCLUDE THIS DIRECTLY!!!
*/

#ifndef __RAID_W32_STRUCT_H_

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
// NOTE(Roman): already included
// #include <windef.h>

#ifndef RC_OPENGL_CLASS_NAME
#define RC_OPENGL_CLASS_NAME "RaidWindowClass"
#endif
#ifndef RC_OPENGL_APP_TITLE
#define RC_OPENGL_APP_TITLE "Raid On"
#endif

namespace game_render_engine {
    struct PlatformWindow {
        HINSTANCE hInst;
        HWND hWnd;
        game_render_engine::GfxOffscreenBuffer offscreen_buffer;
    };

    bool32 createWindow(uint32 width, uint32 height, const char* className, PlatformWindow& window, uint32 flags);
}

#define __RAID_W32_STRUCT_H_
#endif//__RAID_W32_STRUCT_H_
