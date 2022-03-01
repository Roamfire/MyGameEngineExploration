/* ========================================================================
$File: $
$Date: $
$Revision: $
$Creator: Roman Markovetski $
$Notice: (C) Copyright 2014 by Farmland Blossoms. All Rights Reserved. $
======================================================================== */

#ifndef __GC_WIN32_GAME_CODE_H_

#include "resource.h"
#include "../RaidCore/Headers/RaidCore_GameImport.h"

#include <Xinput.h>

typedef DWORD(WINAPI *_func_XInputGetKeystroke)(DWORD, DWORD, PXINPUT_KEYSTROKE);
typedef DWORD(WINAPI *_func_XInputGetState)(DWORD, XINPUT_STATE*);
typedef DWORD(WINAPI *_func_XInputSetState)(DWORD, XINPUT_VIBRATION*);
typedef DWORD(WINAPI *_func_XInputGetCapabilities)(DWORD, DWORD, XINPUT_CAPABILITIES*);

typedef struct Win32GameInputApi {
    HMODULE
        hInputMdl;
    _func_XInputGetKeystroke inputGetKeystroke;
    _func_XInputGetState inputGetState;
    _func_XInputSetState inputSetState;
    _func_XInputGetCapabilities inputGetCapabilities;
} Win32GameInputApi;

typedef struct Win32GameCodeApi {
    // game win32 instance
    HINSTANCE
        hInst;
    HWND
        hWnd;
    HMODULE
        hMdl;
    FILETIME
        DLLLastWriteTime;
    // game handler api
    Game_Independant_Process *gameIndependantProcess;
    Game_UpdateAndRender *gameUpdateAndRender;
    Game_Get_Sound_Buffers *gameGetSoundBuffers;
    Update_Game_State *updateGameState;
    Init_Game_State *initGameState;
    Shutdown_Game_State *shutdownGameState;
    // game run status
    bool32
        IsValid;
    bool32
        bRunning;
    bool32
        bNetworkGame;
    bool32
        bCanRender;
    // game misc
    GameMemory* gameMemory;
} Win32GameCodeApi;

#define __GC_WIN32_GAME_CODE_H_
#endif
