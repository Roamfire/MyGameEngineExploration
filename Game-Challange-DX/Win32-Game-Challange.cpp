/* ========================================================================
$File: $
$Date: $
$Revision: $
$Creator: Roman Markovetski $
$Notice: (C) Copyright 2014 by Farmland Blossoms. All Rights Reserved. $
======================================================================== */

// Game-Challange-DX.cpp : Defines the entry point for the application.
//

#include <io.h>
#include <stdio.h>
#include <stdlib.h>

#include <Windows.h>
#include <Windowsx.h>

#include "../RaidCore/Headers/RaidCore_RenderHandler.h"
// User input manager and handler
#include "../RaidCore/Headers/RaidCore_UserInputHdr.h"
#include "../RaidCore/OSHeaders/RaidCore_Win32_Struct.h"
#include "../RaidCore/Headers/RaidCore_Platform.h"
//
#include "../RaidCore/RaidCore_Win32Timer.cpp"
#include "../RaidCore/RaidCore_Win32MemoryAlloc.cpp"
#include "../RaidCore/RaidCore_MemManager.cpp"
#include "../GameHandler/RaidCore_SoundSystem.cpp"
#include "RaidCore_Win32_SoundSystem.cpp"

namespace game_memory {
	arena_p Initialize(MemoryBlock * mem, memory_int gameMemSize) {
		if (NULL == (mem->m_pMemory = memory_func::ViAllocate<uint8>(gameMemSize))) {
			return NULL;
		}

		mem->m_iMemoryLength = gameMemSize;
		arena_p p = (arena_p)mem->m_pMemory;
		p->_basePtr = mem->m_pMemory + sizeof(arena);
		p->_allocLength = mem->m_iMemoryLength - sizeof(arena);
		p->_pStateManager = NULL;
		resetArena(p);
		return (p);
	}

	void Terminate(MemoryBlock * mem) {
		mem->m_iMemoryLength = 0;
		memory_func::Free(mem->m_pMemory);
	}
}

//
#include "Win32-Game-Challange.h"
#include "../GameHandler/debug_s.h"

/*
*******************************************************************/
struct PlatformWorkQueueEntry {
    PlatformWorkQueueCallback *callback;
    void *data;
};

struct PlatformWorkQueue {
    uint32 volatile completionGoal;
    uint32 volatile completionCount;

    uint32 volatile nextEntryToWrite;
    uint32 volatile nextEntryToRead;
    HANDLE semaphoreHandle;

    PlatformWorkQueueEntry entries[256];
};

internal void
Win32AddEntry(PlatformWorkQueue *Queue, PlatformWorkQueueCallback *callback, void *Data) {
    // TODO(Roman): Switch to InterlockedCompareExchange eventually
    // so that any thread can add?
    uint32 NewNextEntryToWrite = (Queue->nextEntryToWrite + 1) % ArrayCount(Queue->entries);
    Assert(NewNextEntryToWrite != Queue->nextEntryToRead);
    PlatformWorkQueueEntry *Entry = Queue->entries + Queue->nextEntryToWrite;
    Entry->callback = callback;
    Entry->data = Data;
    ++Queue->completionGoal;
    _WriteBarrier();
    Queue->nextEntryToWrite = NewNextEntryToWrite;
    ReleaseSemaphore(Queue->semaphoreHandle, 1, 0);
}

internal bool32
Win32DoNextWorkQueueEntry(PlatformWorkQueue *Queue) {
    bool32 WeShouldSleep = false;

    uint32 OriginalNextEntryToRead = Queue->nextEntryToRead;
    uint32 NewNextEntryToRead = (OriginalNextEntryToRead + 1) % ArrayCount(Queue->entries);
    if (OriginalNextEntryToRead != Queue->nextEntryToWrite) {
        uint32 Index = InterlockedCompareExchange((LONG volatile *)&Queue->nextEntryToRead,
                                                  NewNextEntryToRead,
                                                  OriginalNextEntryToRead);
        if (Index == OriginalNextEntryToRead) {
            PlatformWorkQueueEntry Entry = Queue->entries[Index];
            Entry.callback(Queue, Entry.data);
            InterlockedIncrement((LONG volatile *)&Queue->completionCount);
        }
    } else {
        WeShouldSleep = true;
    }

    return(WeShouldSleep);
}

internal void
Win32CompleteAllWork(PlatformWorkQueue *Queue) {
    while (Queue->completionGoal != Queue->completionCount) {
        Win32DoNextWorkQueueEntry(Queue);
    }

    Queue->completionGoal = 0;
    Queue->completionCount = 0;
}

game_memory::MemoryBlock Win32LoadFile(const char * path, game_memory::arena_p pArena) {
    game_memory::MemoryBlock result = {};
    // Open the file
    FILE *filePtr;
    fopen_s(&filePtr, path, "rb");
    if (filePtr == NULL)
        return(result);

    fseek(filePtr, 0, SEEK_END);
    int32 len = ftell(filePtr);
    fseek(filePtr, 0, SEEK_SET);
    result = PushSize(pArena, len);
    fread(result.m_pMemory, len, 1, filePtr);
    fclose(filePtr);

    return(result);
}

DWORD WINAPI
ThreadProc(LPVOID lpParameter) {
    PlatformWorkQueue *Queue = (PlatformWorkQueue *)lpParameter;

    for (;;) {
        if (Win32DoNextWorkQueueEntry(Queue)) {
            WaitForSingleObjectEx(Queue->semaphoreHandle, INFINITE, FALSE);
        }
    }

    //    return(0);
}

internal PLATFORM_WORK_QUEUE_CALLBACK(DoWorkerWork) {
    char Buffer[256];
    wsprintf(Buffer, "Thread %u: %s\n", GetCurrentThreadId(), (char *)Data);
    OutputDebugStringA(Buffer);
}

internal void
Win32MakeQueue(PlatformWorkQueue *Queue, uint32 ThreadCount) {
    Queue->completionGoal = 0;
    Queue->completionCount = 0;

    Queue->nextEntryToWrite = 0;
    Queue->nextEntryToRead = 0;

    uint32 InitialCount = 0;
    Queue->semaphoreHandle = CreateSemaphoreEx(0,
                                               InitialCount,
                                               ThreadCount,
                                               0, 0, SEMAPHORE_ALL_ACCESS);
    for (uint32 ThreadIndex = 0;
         ThreadIndex < ThreadCount;
         ++ThreadIndex) {
        DWORD ThreadID;
        HANDLE ThreadHandle = CreateThread(0, 0, ThreadProc, Queue, 0, &ThreadID);
        CloseHandle(ThreadHandle);
    }
}

/*
*******************************************************************/
void Win32GameGlobalInit(HWND hWnd, Win32GameCodeApi* gameCode) {
    SetLayeredWindowAttributes(hWnd, 0, 255, LWA_ALPHA);
    ::SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)gameCode);
}

FILETIME Win32GetLastWriteTime(const char* name)
{
    WIN32_FILE_ATTRIBUTE_DATA fileData;
    if (!GetFileAttributesExA(name, GetFileExInfoStandard, &fileData)) {
        return{0,0};
    }
    return fileData.ftLastWriteTime;
}

internal Win32GameCodeApi
Win32LoadGameCode(char *SourceDLLName, char *TempDLLName, char *LockFileName) {
    Win32GameCodeApi result = {};

    //WIN32_FILE_ATTRIBUTE_DATA Ignored;
    //if (!GetFileAttributesExA(LockFileName, GetFileExInfoStandard, &Ignored))
    {
        result.DLLLastWriteTime = Win32GetLastWriteTime(SourceDLLName);
        CopyFile(SourceDLLName, TempDLLName, FALSE);

        result.hMdl = LoadLibraryA(TempDLLName);
        if (result.hMdl) {
            result.shutdownGameState = (Shutdown_Game_State *)
                GetProcAddress(result.hMdl, "ShutdownGameState");
            result.initGameState = (Init_Game_State *)
                GetProcAddress(result.hMdl, "InitGameState");
            result.updateGameState = (Update_Game_State *)
                GetProcAddress(result.hMdl, "UpdateGameState");
            result.gameGetSoundBuffers = (Game_Get_Sound_Buffers *)
                GetProcAddress(result.hMdl, "GameGetSoundBuffers");
            result.gameIndependantProcess = (Game_Independant_Process *)
                GetProcAddress(result.hMdl, "GameIndependantProcess");
            result.gameUpdateAndRender = (Game_UpdateAndRender *)
                GetProcAddress(result.hMdl, "GameUpdateAndRender");

            result.IsValid = (result.shutdownGameState &&
                              result.initGameState &&
                              result.updateGameState &&
                              result.gameGetSoundBuffers &&
                              result.gameIndependantProcess &&
                              result.gameUpdateAndRender);
        }
    }

    if (!result.IsValid) {
        result.shutdownGameState = 0;
        result.initGameState = 0;
        result.updateGameState = 0;
        result.gameGetSoundBuffers = 0;
        result.gameIndependantProcess = 0;
        result.gameUpdateAndRender = 0;
    }

    return (result);
}

internal void
Win32UnloadGameCode(Win32GameCodeApi *GameCode) {
    if (GameCode->hMdl) {
        FreeLibrary(GameCode->hMdl);
        GameCode->hMdl = 0;
    }

    GameCode->IsValid = false;
    GameCode->shutdownGameState = 0;
    GameCode->initGameState = 0;
    GameCode->updateGameState = 0;
    GameCode->gameGetSoundBuffers = 0;
    GameCode->gameIndependantProcess = 0;
    GameCode->gameUpdateAndRender = 0;
}

internal bool32 Win32ReloadGameCode(char *SourceDLLName, char *TempDLLName, char *LockFileName, Win32GameCodeApi& gameCode, game_render_engine::PlatformWindow* window) {
    WIN32_FILE_ATTRIBUTE_DATA Ignored;
    if (!GetFileAttributesExA(LockFileName, GetFileExInfoStandard, &Ignored)) {
        FILETIME DLLLastWriteTime = Win32GetLastWriteTime(SourceDLLName);
        if ((DLLLastWriteTime.dwHighDateTime > gameCode.DLLLastWriteTime.dwHighDateTime) ||
            (DLLLastWriteTime.dwHighDateTime == gameCode.DLLLastWriteTime.dwHighDateTime && DLLLastWriteTime.dwLowDateTime > gameCode.DLLLastWriteTime.dwLowDateTime)) {
            Win32UnloadGameCode(&gameCode);
            DeleteFile(TempDLLName);
            CopyFile(SourceDLLName, TempDLLName, FALSE);
            gameCode.DLLLastWriteTime = DLLLastWriteTime;

            gameCode.hMdl = LoadLibraryA(TempDLLName);
            if (gameCode.hMdl) {
                gameCode.shutdownGameState = (Shutdown_Game_State *)
                    GetProcAddress(gameCode.hMdl, "ShutdownGameState");
                gameCode.initGameState = (Init_Game_State *)
                    GetProcAddress(gameCode.hMdl, "InitGameState");
                gameCode.updateGameState = (Update_Game_State *)
                    GetProcAddress(gameCode.hMdl, "UpdateGameState");
                gameCode.gameGetSoundBuffers = (Game_Get_Sound_Buffers *)
                    GetProcAddress(gameCode.hMdl, "GameGetSoundBuffers");
                gameCode.gameIndependantProcess = (Game_Independant_Process *)
                    GetProcAddress(gameCode.hMdl, "GameIndependantProcess");
                gameCode.gameUpdateAndRender = (Game_UpdateAndRender *)
                    GetProcAddress(gameCode.hMdl, "GameUpdateAndRender");

                gameCode.IsValid = (gameCode.shutdownGameState &&
                                    gameCode.initGameState &&
                                    gameCode.updateGameState &&
                                    gameCode.gameGetSoundBuffers &&
                                    gameCode.gameIndependantProcess &&
                                    gameCode.gameUpdateAndRender);
            }
            if (!gameCode.IsValid) {
                gameCode.shutdownGameState = 0;
                gameCode.initGameState = 0;
                gameCode.updateGameState = 0;
                gameCode.gameGetSoundBuffers = 0;
                gameCode.gameIndependantProcess = 0;
                gameCode.gameUpdateAndRender = 0;
            } else {
            }
        }
    }

    return gameCode.IsValid;
}

//
// Global Variables:
internal char szTitle[] = "_"; // The title bar text
internal char szWindowClass[] = "raid_core_win32"; // the main window class name
//
static WINDOWPLACEMENT GlobalWindowPosition = { sizeof(GlobalWindowPosition) };

void ToggleWindowFullScreen(HWND hWnd, GameMemory * gameMemory) {
    DWORD style = GetWindowLong(hWnd, GWL_STYLE);
    if (style & WS_OVERLAPPEDWINDOW) {
        MONITORINFO MonitorInfo = { sizeof(MonitorInfo) };
        if (GetWindowPlacement(hWnd, &GlobalWindowPosition) &&
            GetMonitorInfo(MonitorFromWindow(hWnd, MONITOR_DEFAULTTOPRIMARY), &MonitorInfo)) {
            SetWindowLong(hWnd, GWL_STYLE, style & ~WS_OVERLAPPEDWINDOW);
            SetWindowPos(hWnd, HWND_TOP,
                         MonitorInfo.rcMonitor.left, MonitorInfo.rcMonitor.top,
                         MonitorInfo.rcMonitor.right - MonitorInfo.rcMonitor.left,
                         MonitorInfo.rcMonitor.bottom - MonitorInfo.rcMonitor.top,
                         SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    } else {
        SetWindowLong(hWnd, GWL_STYLE, style | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(hWnd, &GlobalWindowPosition);
        SetWindowPos(hWnd, 0, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                     SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
}

#define TICK_TIME 50
#define MAX_LOOPS 25

#include "../RaidCore/Headers/RaidCore_RandomMath.h"

void Win32IndependentTickRun(real32 frameTime, GameMemory * gameMemory) {
    //
    // house keeping, no need to time...
    //   but... keep it short
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    int wmId, wmEvent;
    PAINTSTRUCT ps;

    if ((message >= WM_MOUSEFIRST) && (message <= WM_MOUSELAST)) {
        Assert(!"Mouse events in the wrong place")
    }
    if ((message >= WM_KEYFIRST) && (message <= WM_KEYLAST)) {
        Assert(!"Keyboard events in the wrong place")
    }

    Win32GameCodeApi* GameCode = (Win32GameCodeApi*) ::GetWindowLongPtr(hWnd, GWLP_USERDATA);

    switch (message) {
    case WM_CLOSE:
    case WM_DESTROY:
    case WM_NCDESTROY:
    case WM_QUERYENDSESSION:
    case WM_QUIT:
        if (NULL != GameCode) {
            GameCode->bRunning = false;
        } 
        break;

    case WM_SIZE:
        if (NULL != GameCode) {
            RECT rClient;
            RECT rWindow;
            ::GetClientRect(hWnd, &rClient);
            ::GetWindowRect(hWnd, &rWindow);

            DWORD w = LOWORD(lParam);
            DWORD h = HIWORD(lParam);

            DWORD wDiff = (rWindow.right - rWindow.left) - (rClient.right - rClient.left);
            DWORD hDiff = (rWindow.bottom - rWindow.top) - (rClient.bottom - rClient.top);

            HDC dcClient = GetWindowDC(hWnd);
            ::SetBkColor(dcClient, 0);
            ReleaseDC(hWnd, dcClient);

            if (wParam == SIZE_RESTORED) {

                if (h < 540) {
                    h = 540;
                }
                if (w < 960) {
                    w = 960;
                }
            }
            lParam = MAKELONG(w, h);

            int32 width = w - wDiff;
            width += ((4 - (width & 3)) & 3);
            GameCode->updateGameState(GameCode->gameMemory, width, h - hDiff);
        } 
    case WM_SIZING:
        break;

    case WM_COMMAND:
    {
        wmId = LOWORD(wParam);
        wmEvent = HIWORD(wParam);
        // Parse the menu selections:
        switch (wmId) {
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        }
    } break;
    case WM_PAINT:
    {
        HDC hDC = BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
    } break;
    default:
        break;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

/*
*******************************************************************/
bool32 InitGameInput(Win32GameInputApi * GameInput) {
    GameInput->hInputMdl = NULL;
    if (NULL == GameInput->hInputMdl)
        GameInput->hInputMdl = LoadLibraryA("Xinput1_3.dll");
    if (NULL == GameInput->hInputMdl)
        GameInput->hInputMdl = LoadLibraryA("Xinput1_4.dll");
    if (NULL == GameInput->hInputMdl)
        GameInput->hInputMdl = LoadLibraryA("Xinput9_1_0.dll");
    if (NULL == GameInput->hInputMdl)
        return false32;
    //
    GameInput->inputGetCapabilities = (_func_XInputGetCapabilities)GetProcAddress(GameInput->hInputMdl, "XInputGetCapabilities");
    GameInput->inputGetState = (_func_XInputGetState)GetProcAddress(GameInput->hInputMdl, "XInputGetState");
    GameInput->inputSetState = (_func_XInputSetState)GetProcAddress(GameInput->hInputMdl, "XInputSetState");
    GameInput->inputGetKeystroke = (_func_XInputGetKeystroke)GetProcAddress(GameInput->hInputMdl, "XInputGetKeystroke");
    //
    if (!(GameInput->inputGetCapabilities && GameInput->inputGetState && GameInput->inputSetState && GameInput->inputGetKeystroke))
        return false32;
    return true32;
}

internal void
CalculateThumb(real32& X, real32& Y, const real32 deadZone) {
    real32 xLength,yLength;
    if (X < -deadZone) {
        xLength = (real32)((X + deadZone) / (32768.0f - deadZone));
    } else if (X > deadZone) {
        xLength = (real32)((X - deadZone) / (32768.0f - deadZone));
    } else {
        xLength = 0.f;
    }
    if (Y < -deadZone) {
        yLength = (real32)((Y + deadZone) / (32768.0f - deadZone));
    } else if (Y > deadZone) {
        yLength = (real32)((Y - deadZone) / (32768.0f - deadZone));
    } else {
        yLength = 0.f;
    }
    X = game_math::clamp(-1.f, xLength, 1.f);
    Y = game_math::clamp(-1.f, yLength, 1.f);
}

internal bool32
Win32ProcessController(user_input::ControllerState* ctrl, Win32GameInputApi * input) {
    XINPUT_STATE inputState;
    setBytes(&inputState, 0, sizeof(XINPUT_STATE));

    // Simply get the state of the controller from XInput.
    if (input->inputGetState(ctrl->CtrlId, &inputState) == ERROR_SUCCESS) {

        if (ctrl->lastPacketNumber != inputState.dwPacketNumber) {
            real32 R_XAxis = inputState.Gamepad.sThumbRX;
            real32 R_YAxis = inputState.Gamepad.sThumbRY;
            real32 L_XAxis = inputState.Gamepad.sThumbLX;
            real32 L_YAxis = inputState.Gamepad.sThumbLY;
            CalculateThumb(R_XAxis, R_YAxis, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
            CalculateThumb(L_XAxis, L_YAxis, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);

            ctrl->R_XAxis = R_XAxis;
            ctrl->R_YAxis = R_YAxis;
            ctrl->L_XAxis = L_XAxis;
            ctrl->L_YAxis = L_YAxis;

            ctrl->lastPacketNumber = inputState.dwPacketNumber;

            ctrl->Pad_Up = ((inputState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) != 0) ? 0xff : 0x00;
            ctrl->Pad_Down = ((inputState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) != 0) ? 0xff : 0x00;
            ctrl->Pad_Left = ((inputState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) != 0) ? 0xff : 0x00;
            ctrl->Pad_Right = ((inputState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) != 0) ? 0xff : 0x00;

            ctrl->RB_Trigger = ((inputState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) != 0) ? 0xff : 0x00; // 0-255
            ctrl->RT_Trigger = ((inputState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) != 0) ? 0xff : 0x00; // 0-255
            ctrl->LB_Trigger = inputState.Gamepad.bRightTrigger; // 0-255
            if (XINPUT_GAMEPAD_TRIGGER_THRESHOLD <= (ctrl->LB_Trigger)) {
                ctrl->LB_Trigger = 0;
            }
            ctrl->LT_Trigger = inputState.Gamepad.bLeftTrigger; // 0-255
            if (XINPUT_GAMEPAD_TRIGGER_THRESHOLD <= (ctrl->LT_Trigger)) {
                ctrl->LT_Trigger = 0;
            }
            ctrl->Btn_X = ((inputState.Gamepad.wButtons & XINPUT_GAMEPAD_X) != 0) ? 0xff : 0x00; // SQUARE
            ctrl->Btn_Y = ((inputState.Gamepad.wButtons & XINPUT_GAMEPAD_Y) != 0) ? 0xff : 0x00; // TRIANGLE
            ctrl->Btn_A = ((inputState.Gamepad.wButtons & XINPUT_GAMEPAD_A) != 0) ? 0xff : 0x00; // X
            ctrl->Btn_B = ((inputState.Gamepad.wButtons & XINPUT_GAMEPAD_B) != 0) ? 0xff : 0x00; // CIRCLE
            ctrl->Btn_Start = ((inputState.Gamepad.wButtons & XINPUT_GAMEPAD_START) != 0) ? 0xff : 0x00; // Play
            ctrl->Btn_Select = ((inputState.Gamepad.wButtons & XINPUT_GAMEPAD_BACK) != 0) ? 0xff : 0x00; // Back
            return true32;
        }
    }
    return false32;
}

internal void
Win32HandleControllerInput(user_input::ControllerState *ctrl, user_input::GameInput *input) {
    if (ctrl->Pad_Up) {
        user_input::setKeyState(input, user_input::key_vertical, user_input::e_kbPressed, 1.0f);
    } else if (ctrl->Pad_Down) {
        user_input::setKeyState(input, user_input::key_vertical, user_input::e_kbPressed, -1.0f);
    } else if (game_math::absolute(ctrl->L_YAxis) > game_math::epsilon) {
        user_input::setKeyState(input, user_input::key_vertical, user_input::e_kbPressed, -ctrl->L_YAxis);
    } else if (user_input::isPressed(input, user_input::key_vertical)) {
        user_input::reset(input, user_input::key_vertical);
    }    

    if (ctrl->Pad_Left) {
        user_input::setKeyState(input, user_input::key_horizontal, user_input::e_kbPressed, -1.0f);
    } else if (ctrl->Pad_Right) {
        user_input::setKeyState(input, user_input::key_horizontal, user_input::e_kbPressed, 1.0f);
    } else if (game_math::absolute(ctrl->L_XAxis) > game_math::epsilon) {
        user_input::setKeyState(input, user_input::key_horizontal, user_input::e_kbPressed, ctrl->L_XAxis);
    } else if (user_input::isPressed(input, user_input::key_horizontal)) {
        user_input::reset(input, user_input::key_horizontal);
    }

    if (ctrl->Btn_A) {
        user_input::setKeyState(input, user_input::key_btn_A, user_input::e_kbPressed, 1.0f);
    } else {
        user_input::setKeyState(input, user_input::key_btn_A, user_input::e_kbReset, 1.0f);
    }
}

internal void 
Win32ProcessKeyboardInput(user_input::GameInput* gameInput, Win32GameInputApi * input) {
#if 0
    XINPUT_KEYSTROKE xiKeys = {};
    int32 testCount = 12;
    while (ERROR_SUCCESS == input->inputGetKeystroke(0, 0, &xiKeys)) {
        bool32 IsDown = (xiKeys.Flags & (XINPUT_KEYSTROKE_KEYDOWN | XINPUT_KEYSTROKE_REPEAT));
        user_input::eKbState state = (IsDown) ? user_input::e_kbPressed : user_input::e_kbReset;
        switch (xiKeys.VirtualKey) {
            case VK_SHIFT:
                user_input::setKeyState(gameInput, user_input::key_btn_special3, state); // shift
                break;
            case VK_OEM_MINUS:
            case VK_SUBTRACT:
                user_input::setKeyState(gameInput, user_input::key_btn_special1, state); // zoom out
                break;
            case VK_OEM_PLUS:
            case VK_ADD:
                user_input::setKeyState(gameInput, user_input::key_btn_special2, state); // zoom in
                break;

            case VK_SPACE:
                user_input::setKeyState(gameInput, user_input::key_btn_A, state, 1.0f);
                break;
            case 'W':
            case VK_UP:
                user_input::setKeyState(gameInput, user_input::key_vertical, state, -1.0f);
                break;
            case 'S':
            case VK_DOWN:
                user_input::setKeyState(gameInput, user_input::key_vertical, state, 1.0f);
                break;
            case 'A':
            case VK_LEFT:
                user_input::setKeyState(gameInput, user_input::key_horizontal, state, -1.0f);
                break;
            case 'D':
            case VK_RIGHT:
                user_input::setKeyState(gameInput, user_input::key_horizontal, state, 1.0f);
                break;
            case 'T':
                user_input::setKeyState(gameInput, user_input::key_btn_specialF, state);
                break;
            case 'O':
                user_input::setKeyState(gameInput, user_input::key_btn_specialE, state);
                break;
            default:
                break;
        }
        if (--testCount) {
            break;
        }
    }
#endif
}

internal void
Win32ProcessPendingMessages(Win32GameCodeApi * gameCode, user_input::GameInput* gameInput) {
    MSG Message;
    for (;;) {
        BOOL GotMessage = FALSE;

        {
            //			TIMED_BLOCK("PeekMessage");

            // NOTE(Roman): We avoid asking for WM_PAINT and WM_MOUSEMOVE messages
            // here so that Windows will not generate them (they are generated on-demand)
            // since we don't actually care about either.
            GotMessage = PeekMessage(&Message, gameCode->hWnd, 0, WM_PAINT - 1, PM_REMOVE);
            if (!GotMessage) {
                GotMessage = PeekMessage(&Message, gameCode->hWnd, WM_PAINT + 1, WM_MOUSEMOVE - 1, PM_REMOVE);
                if (!GotMessage) {
                    GotMessage = PeekMessage(&Message, gameCode->hWnd, WM_MOUSEMOVE + 1, 0xFFFFFFFF, PM_REMOVE);
                }
            }
        }
        if (!GotMessage) {
            break;
        }
        if ((Message.message >= WM_MOUSEFIRST) && (Message.message <= WM_MOUSELAST)) {
            int xPos = GET_X_LPARAM(Message.lParam);
            int yPos = GET_Y_LPARAM(Message.lParam);
            int fwKeys = 0;
            int zDelta = GET_WHEEL_DELTA_WPARAM(Message.wParam) / WHEEL_DELTA;

            switch (Message.message) {
                case WM_MOUSEMOVE:
                    break;
                case WM_MOUSEHWHEEL:
                    if (zDelta < 0) {
                        user_input::addKeyStateSwitch(gameInput, user_input::key_btn_special1, -zDelta); // zoom out
                    } else {
                        user_input::addKeyStateSwitch(gameInput, user_input::key_btn_special2, zDelta); // zoom in
                    }
                    break;
                case WM_LBUTTONDOWN:
                    user_input::setKeyState(gameInput, user_input::key_btn_LT, user_input::e_kbPressed, 1.0f);
                    break;
                case WM_LBUTTONUP:
                    user_input::setKeyState(gameInput, user_input::key_btn_LT, user_input::e_kbReset);
                    break;
                case WM_RBUTTONDOWN:
                    user_input::setKeyState(gameInput, user_input::key_btn_RT, user_input::e_kbPressed, 1.0f);
                    break;
                case WM_RBUTTONUP:
                    user_input::setKeyState(gameInput, user_input::key_btn_RT, user_input::e_kbReset);
                    break;
            }
            continue;
        }
        if ((Message.message >= WM_KEYFIRST) && (Message.message <= WM_KEYLAST)) {
            switch (Message.message) {
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYDOWN:
            case WM_KEYUP:
            {
                uint32 VKCode = (uint32)Message.wParam;
                bool32 AltKeyWasDown = (Message.lParam & (1 << 29));

                if (AltKeyWasDown) {
                    user_input::setKeyState(gameInput, user_input::key_btn_special0, user_input::e_kbPressed, 1.0f);
                } else {
                    user_input::setKeyState(gameInput, user_input::key_btn_special0, user_input::e_kbReset);
                }

                // NOTE(Roman): Since we are comparing WasDown to IsDown,
                // we MUST use == and != to convert these bit tests to actual
                // 0 or 1 values.
                bool32 WasDown = ((Message.lParam & (1 << 30)) != 0);
                bool32 IsDown = ((Message.lParam & (1 << 31)) == 0);
                if (WasDown != IsDown) {
                    if ((VKCode == VK_ESCAPE) && !IsDown) {
                        gameCode->bRunning = false;
                    }
                    user_input::eKbState state = (IsDown) ? user_input::e_kbPressed : user_input::e_kbReset;

#if 1
                    switch (VKCode) {
                        case VK_SHIFT:
                            user_input::setKeyState(gameInput, user_input::key_btn_special3, state); // shift
                            break;
                        case VK_OEM_MINUS:
                        case VK_SUBTRACT:
                            user_input::setKeyState(gameInput, user_input::key_btn_special1, state); // zoom out
                            break;
                        case VK_OEM_PLUS:
                        case VK_ADD:
                            user_input::setKeyState(gameInput, user_input::key_btn_special2, state); // zoom in
                            break;

                        case VK_SPACE:
                            user_input::setKeyState(gameInput, user_input::key_btn_A, state, 1.0f);
                            break;
                        case 'W':
                        case VK_UP:
                            //user_input::setKeyState(gameInput, user_input::key_vertical, state, -1.0f);
							user_input::addKeyForce(gameInput, user_input::key_vertical, state, -1.0f);
                            break;
                        case 'S':
                        case VK_DOWN:
                            //user_input::setKeyState(gameInput, user_input::key_vertical, state, 1.0f);
							user_input::addKeyForce(gameInput, user_input::key_vertical, state, 1.0f);
                            break;
                        case 'A':
                        case VK_LEFT:
                            //user_input::setKeyState(gameInput, user_input::key_horizontal, state, -1.0f);
							user_input::addKeyForce(gameInput, user_input::key_horizontal, state, -1.0f);
                            break;
                        case 'D':
                        case VK_RIGHT:
                            //user_input::setKeyState(gameInput, user_input::key_horizontal, state, 1.0f);
							user_input::addKeyForce(gameInput, user_input::key_horizontal, state, 1.0f);
                            break;
                        case 'T':
                            user_input::setKeyState(gameInput, user_input::key_btn_specialF, state);
                            break;
                        case 'O':
                            user_input::setKeyState(gameInput, user_input::key_btn_specialE, state);
                            break;
                        default:
                            Win32HandleKeyException(VKCode, IsDown, gameInput);
                            break;
                    }
#endif
                    if (IsDown) {
                        if ((VKCode == VK_F4) && AltKeyWasDown) {
                            gameCode->bRunning = false;
                        }
                        if ((VKCode == VK_RETURN) && AltKeyWasDown) {
                            if (Message.hwnd) {
                                ToggleWindowFullScreen(Message.hwnd, gameCode->gameMemory);
                            }
                        }
                    }
                }

            } break;

            }
            continue;
        }

        TranslateMessage(&Message);
        DispatchMessageA(&Message);
    }
}

#ifdef RC_TESTCODE
#include "Win32Debug.cpp"
#else
#endif

void Register(Win32GameCodeApi* GameCode) {
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_DBLCLKS;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = GameCode->hInst;
    wcex.hIcon = LoadIcon(GameCode->hInst, MAKEINTRESOURCE(IDI_GAMECHALLANGEDX));
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = RC_OPENGL_CLASS_NAME;
    wcex.hIconSm = LoadIcon(GameCode->hInst, MAKEINTRESOURCE(IDI_SMALL));
    RegisterClassEx(&wcex);
}
void Unregister(Win32GameCodeApi* GameCode) {
    UnregisterClass(RC_OPENGL_CLASS_NAME, GameCode->hInst);
}

// Sound
void Win32InitSound(Win32GameCodeApi* GameCode, game_sound::GameSoundHandler* sound, uint16 SamplesPerSecond, uint32 BufferSize) {
    *sound = game_sound::DefaultHandler();
    sound->SamplesPerSecond = SamplesPerSecond;
    sound->BufferSize = BufferSize;

    sound->soundSource = PushStruct(GameCode->gameMemory->GamePersistentMemoryArena, game_sound::SoundSource);
    sound->soundSource->hWnd = GameCode->hWnd;

    if (!game_sound::InitSoundSystem(sound)) {
        // TODO(Roman)
        Assert(!"Sound was not initiated");
    }
}
void
Win32ShutdownSound(Win32GameCodeApi* GameCode, game_sound::GameSoundHandler* Handler) {

    game_sound::ShutdownSoundSystem(Handler);
}
void
Win32FillSoundPlayBuffer(game_sound::GameSoundHandler & Sound, game_sound::SoundPlayBuffer & s, game_sound::sound_piece & soundBuffer, uint32 lastPlayPos, game_sound::SoundPlayBuffer & playBuffer) {
    
}

//
void
#if 1
Win32DrawOffscreenBuffer(HDC hDC, game_render_engine::GfxOffscreenBuffer* buffer) {
	game_render_engine::GfxOffscreenBuffer setup = *buffer;
	uint8 stackData[sizeof(BITMAPINFO) + sizeof(RGBQUAD) * 3];
	BITMAPINFO* info = (BITMAPINFO*)&stackData[0];
	*info = { 0 };
	info->bmiHeader.biSize = sizeof(BITMAPINFO) + sizeof(RGBQUAD) * 3;
	info->bmiHeader.biWidth = setup.imageStats.width;//hit
	info->bmiHeader.biHeight = setup.imageStats.height;//hit
	info->bmiHeader.biPlanes = 1;
	info->bmiHeader.biBitCount = 32;
	info->bmiHeader.biCompression = BI_RGB;
	info->bmiHeader.biSizeImage = getBitmapSize(buffer->imageStats);

	info->bmiColors[0] = { 0xff, 0, 0, 0 };
	info->bmiColors[1] = { 0, 0xff, 0, 0 };
	info->bmiColors[2] = { 0, 0, 0xff, 0 };

	StretchDIBits(hDC, 0, 0, setup.imageStats.width, setup.imageStats.height, 0, 0, setup.imageStats.width, setup.imageStats.height, setup.buffer, info, DIB_RGB_COLORS, SRCCOPY);
}
#else
Win32DrawOffscreenBuffer(HDC hDC, game_render_engine::GfxOffscreenBuffer* buffer) {
    BITMAPINFO* info = (BITMAPINFO*)_alloca(sizeof(BITMAPINFO) + sizeof(RGBQUAD) * 3);
    *info = {};
    info->bmiHeader.biSize = sizeof(BITMAPINFO) + sizeof(RGBQUAD) * 3;
    info->bmiHeader.biWidth = buffer->imageStats.width;//hit
    info->bmiHeader.biHeight = buffer->imageStats.height;//hit
    info->bmiHeader.biPlanes = 1;
    info->bmiHeader.biBitCount = 32;
    info->bmiHeader.biCompression = BI_RGB;
    info->bmiHeader.biSizeImage = getBitmapSize(buffer->imageStats);

    info->bmiColors[0] = { 0xff, 0, 0, 0 };
    info->bmiColors[1] = { 0, 0xff, 0, 0 };
    info->bmiColors[2] = { 0, 0, 0xff, 0 };

    StretchDIBits(hDC, 0, 0, buffer->imageStats.width, buffer->imageStats.height, 0, 0, buffer->imageStats.width, buffer->imageStats.height, buffer->buffer, info, DIB_RGB_COLORS, SRCCOPY);
}
#endif

struct ThreadData {
    Win32GameCodeApi* gc;
    user_input::GameInput* input;
    Win32GameInputApi* inputApi;
};

DWORD WINAPI Win32InputThread(void* api) {
    
    ThreadData* Api = (ThreadData*)api;

    while (Api->gc->bRunning) {
        for (uint32 i = 0; i < ArrayCount(Api->input->m_controllers); ++i) {
            if (Win32ProcessController(&Api->input->m_controllers[i], Api->inputApi)) {
                Win32HandleControllerInput(&Api->input->m_controllers[i], Api->input);
            }
        }
        Win32ProcessKeyboardInput(Api->input, Api->inputApi);
        Win32ProcessPendingMessages(Api->gc, Api->input);

        _mm_pause();
    }
    return 0;
}

internal void Win32ProcessInput(Win32GameCodeApi* gameCode, user_input::GameInput* input, Win32GameInputApi* gameInput) {
    for (uint32 i = 0; i < ArrayCount(input->m_controllers); ++i) {
        if (Win32ProcessController(&input->m_controllers[i], gameInput)) {
            Win32HandleControllerInput(&input->m_controllers[i], input);
        }
    }
    Win32ProcessKeyboardInput(input, gameInput);
    Win32ProcessPendingMessages(gameCode, input);
}

//
int APIENTRY WinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPTSTR    lpCmdLine,
	_In_ int       nCmdShow) {
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	int reply = FALSE;

	Win32GameCodeApi GameCode = Win32LoadGameCode("GameHandler.dll", "TmpGame.dll", "lock.lll");
	if (!GameCode.IsValid) {
		return FALSE;
	}

	PlatformWorkQueue HighPriorityQueue = {};
	Win32MakeQueue(&HighPriorityQueue, 6);

	PlatformWorkQueue LowPriorityQueue = {};
	Win32MakeQueue(&LowPriorityQueue, 2);

	GameCode.bRunning = true32;
	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_GAMECHALLANGEDX));

	game_memory::MemoryBlock gameMemoryBlock;
	game_memory::MemoryBlock scratchMemoryBlock;
	game_memory::MemoryBlock gameRunBlock;
	//
	//
	// Note(Roman):
	// m_pGameMemoryRoot - The full memory block we got
	game_memory::arena_p pPersist = game_memory::Initialize(&gameMemoryBlock, MByte(640));
	if (pPersist == NULL) {
		return FALSE;
	}
	game_memory::arena_p pTemp = game_memory::Initialize(&scratchMemoryBlock, MByte(640));
	if (pTemp == NULL) {
		return FALSE;
	}
	game_memory::arena_p pRun = game_memory::Initialize(&gameRunBlock, MByte(48));
	if (pRun == NULL) {
		return FALSE;
	}

	GameMemory gameMemory = {};
	gameMemory.GameTranscientMemoryArena = pTemp;
	gameMemory.GamePersistentMemoryArena = pPersist;
	gameMemory.GameRunArena = pRun;
	gameMemory.LowPriorityQueue = &LowPriorityQueue;
	gameMemory.HighPriorityQueue = &HighPriorityQueue;
	gameMemory.platformApi.addEntry = Win32AddEntry;
	gameMemory.platformApi.completeAllWork = Win32CompleteAllWork;
	gameMemory.platformApi.loadFile = Win32LoadFile;
	gameMemory.platformApi.allocate = memory_func::Allocate;
	gameMemory.platformApi.free = memory_func::Free;

	GameCode.gameMemory = &gameMemory;

#ifdef RC_TESTCODE
	debug_timer debugTimers[timer_id_count] = {};
	gameMemory.DEBUGGlobalMemory = debugTimers;
	debug_timer* DEBUGGlobalMemory = debugTimers;

	game_timer::RaidTick dTicker;
	game_timer::Init(&dTicker);
	//
	game_timer::Diff(&dTicker);
	game_timer::Mark(&dTicker);
#endif

    // NOTE(Roman): using new operator to construct an object over a memory block.
	// new (ptr*) ptr::type::constructor(); // will execute the constructor on the memory area...
	// new (m_pGameState) GameState();
	Win32GameInputApi GameInput = {};
	if (!InitGameInput(&GameInput)) {
		return false32;
	}

	uint32 NumberOfTileSpacesLoaded = 0;

	game_render_engine::PlatformWindow window = {};
    window.offscreen_buffer = {};
	GameCode.hInst = window.hInst = hInstance;
	Register(&GameCode);

	user_input::GameInput input = {};
	for (uint32 i = 0; i < ArrayCount(input.m_controllers); ++i) {
		input.m_controllers[i].CtrlId = i;
	}
	
	GameCode.initGameState(&gameMemory, &input, &window);
	GameCode.bRunning = (NULL != window.hWnd);

	GameCode.hWnd = window.hWnd;
	Win32GameGlobalInit(window.hWnd, &GameCode);

	// .
	//
	RECT rc;
	::GetClientRect(window.hWnd, &rc);
	GameCode.updateGameState(&gameMemory, rc.right - rc.left, rc.bottom - rc.top);

	GameCode.bCanRender = true32;
	//
	// We want to impose a 60Hz render frequency,
	// So to handle delays, we will break rendering and display into two separate functions
	//
	// First, the Compose() /* Implemented later */ 
	//  - Collects the game state, and performes the drawing into the back buffer
	//    Can be called multiple times, but will not draw unless flagged to update, or buffer was displayed.
	//    On successful composition, flags render.
	// Second, the Render()
	//  - Whem render is flagged, swaps the buffers and prepares the new back buffer for drawing (cleanup)

	game_timer::RaidTick Ticker;
	game_timer::Init(&Ticker);
	//

	game_sound::GameSoundHandler Sound = {};

	Win32InitSound(&GameCode, &Sound, 48000, 0);
	game_sound::SoundBuffer s = {};
	if (game_sound::InitSound(&Sound, &s)) {
		s.State = game_sound::idle;
	}

	// NOTE(Roman): BytesPerSample here include all channels!
    int BytesPerSample = Sound.BytesPerSample * Sound.Channels;
	int sBufferSize = game_sound::maxBufferSize(&Sound);

	//
	// NOTE(Roman): static data we'll use to prepare the audio data, per frame
	game_sound::sound_piece soundBuffer = {};
    //
    // NOTE(Roman): soundBuffer.source must be cleared to zero (done by default PushArray)
    soundBuffer.source = PushArray(pRun, sBufferSize, uint8);
    soundBuffer.length = sBufferSize;
    soundBuffer.bytesPerSample = BytesPerSample;
    //
	game_sound::SoundPlayBuffer playBuffer = {};
	uint32 lastPlayPos = 0;

	// NOTE(Roman): set silence in sound buffer and start playing it
	if (SUCCEEDED(s.PlayBuffer->Lock(0, sBufferSize, &(playBuffer.firstBuffer), (LPDWORD)&(playBuffer.firstLength), &(playBuffer.secondBuffer), (LPDWORD)&(playBuffer.secondLength), 0))) {

		int16* sampleOut = (int16*)playBuffer.firstBuffer;
		DWORD s1 = playBuffer.firstLength / BytesPerSample;
		DWORD s2 = playBuffer.secondLength / BytesPerSample;

		for (DWORD si = 0; si < s1; ++si) {
			*sampleOut++ = 0;
			*sampleOut++ = 0;
		}
		sampleOut = (int16*)playBuffer.secondBuffer;
		for (DWORD si = 0; si < s2; ++si) {
			*sampleOut++ = 0;
			*sampleOut++ = 0;
		}
		lastPlayPos += playBuffer.firstLength + playBuffer.secondLength;
		s.PlayBuffer->Unlock((playBuffer.firstBuffer), (DWORD)(playBuffer.firstLength), (void*)(playBuffer.secondBuffer), (DWORD)(playBuffer.secondLength));
	}
	s.PlayBuffer->Play(0, 0, DSBPLAY_LOOPING);
	s.State = game_sound::playing;

    // NOTE(Roman):
    // Setup the framerate target (30fps)
    real32 secPerFrame = 1.f / 30.f;
    Win32DebugSetup(30);

    //
    timeBeginPeriod(1);

	game_timer::Diff(&Ticker);
	game_timer::Mark(&Ticker);

	//
	//
	// Main message loop:
	while (GameCode.bRunning) {
		if (!Win32ReloadGameCode("GameHandler.dll", "TmpGame.dll", "lock.lll", GameCode, &window)) {
			GameCode.bRunning = false;
			continue;
		}

		BEGIN_TIMED_BLOCK(GameLoop);
		//
		//-----------------------------------------------------------------------------------------------
        Win32ProcessInput(&GameCode, &input, &GameInput);

		/////////////////////////////////////////////////
		//  FULL GAME LOOP                             //
		/////////////////////////////////////////////////

		// NOTE(Roman): fill the sound buffer for the next frame + a tail in case we are late
        real32 frameTimeDelta = game_timer::getElapsedSeconds(&Ticker);
		GameCode.gameGetSoundBuffers(&gameMemory, frameTimeDelta, 0.0f, &playBuffer, &soundBuffer);

		// NOTE(Roman): process input to set states
        frameTimeDelta = game_timer::getElapsedSeconds(&Ticker);
		GameCode.gameIndependantProcess(&gameMemory, &input, frameTimeDelta);

		// NOTE(Roman): handle input, state changes
		//   process collisions and movement
		//   prepare the render queue
		//   draw and flip the buffer
        GameCode.gameUpdateAndRender(&gameMemory, &input, secPerFrame);

		// NOTE(Roman): fill the DirectSound secondary buffer
        DWORD playPtr=0;
        DWORD writePtr;
        if (game_sound::playing == s.State) {
            if (lastPlayPos >= Sound.BufferSize) {
                lastPlayPos -= Sound.BufferSize;
            }
            if (SUCCEEDED(s.PlayBuffer->GetCurrentPosition(&playPtr, &writePtr))) {

                DWORD toLock = soundBuffer.length & Sound.BufferSize;
                DWORD toWrite = (toLock > playPtr) ?
                    ((Sound.BufferSize - toLock) + playPtr) :
                    (playPtr - toLock);

                if (SUCCEEDED(s.PlayBuffer->Lock(toLock, toWrite, &(playBuffer.firstBuffer), (LPDWORD)&(playBuffer.firstLength), &(playBuffer.secondBuffer), (LPDWORD)&(playBuffer.secondLength), 0))) {
                    int16* sampleIn = (int16*)soundBuffer.source;
                    int16* sampleOut = (int16*)playBuffer.firstBuffer;
                    DWORD s1 = playBuffer.firstLength / BytesPerSample;
                    DWORD s2 = playBuffer.secondLength / BytesPerSample;

                    for (DWORD si = 0; si < s1; ++si) {
                        *sampleOut++ = *sampleIn++;
                        *sampleOut++ = *sampleIn++;
                    }
                    sampleOut = (int16*)playBuffer.secondBuffer;
                    for (DWORD si = 0; si < s2; ++si) {
                        *sampleOut++ = *sampleIn++;
                        *sampleOut++ = *sampleIn++;
                    }
                    lastPlayPos += playBuffer.firstLength + playBuffer.secondLength;
                    s.PlayBuffer->Unlock((playBuffer.firstBuffer), (DWORD)(playBuffer.firstLength), (void*)(playBuffer.secondBuffer), (DWORD)(playBuffer.secondLength));
                }
            }
        }

        frameTimeDelta = game_timer::getElapsedSeconds(&Ticker);
        if (frameTimeDelta < secPerFrame) {
            // now CPU melt until we are good!
            while ((frameTimeDelta - secPerFrame) < game_math::epsilon) {
                Sleep(1);
                Win32ProcessInput(&GameCode, &input, &GameInput);
                frameTimeDelta = game_timer::getElapsedSeconds(&Ticker);
            }
        }
        game_timer::Mark(&Ticker);

        Win32DebugSyncSet(playPtr, Sound.BufferSize);
        Win32DebugFrameSet(frameTimeDelta);
        Win32DebugSyncDisplay(&window.offscreen_buffer);

        BEGIN_TIMED_BLOCK(RenderBlt);
        HDC hDC = GetDC(window.hWnd);
        Win32DrawOffscreenBuffer(hDC, &window.offscreen_buffer);
        ReleaseDC(window.hWnd, hDC);
        END_TIMED_BLOCK(RenderBlt);

		END_TIMED_BLOCK(GameLoop);

        ++gameMemory.DEBUGFrameCounter;
	}

#ifdef RC_TESTCODE
	HandleDebugCycleCounters(DEBUGGlobalMemory, static_cast<real32>(gameMemory.DEBUGFrameCounter) / static_cast<real32>(game_timer::Diff(&dTicker)/1000));
#endif

	//
	// NOTE(Roman): Cleanup!!!
	// Terminating the program will get rid of all memeoy allocated and usually of all resources.
	// However, it is always a good practice to handle the cleanup properly, for completeness and usually a good way to see problems occur.
	Win32ShutdownSound(&GameCode, &Sound);
	if (GameCode.IsValid) {
		GameCode.shutdownGameState(&gameMemory);
	}
	Win32UnloadGameCode(&GameCode);
	Unregister(&GameCode);
	//
	//
	game_memory::Terminate(&scratchMemoryBlock);
	game_memory::Terminate(&gameMemoryBlock);
	game_memory::Terminate(&gameRunBlock);

	return reply;
}

