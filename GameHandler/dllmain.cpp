/* ========================================================================
$File: $
$Date: $
$Revision: $
$Creator: Roman Markovetski $
$Notice: (C) Copyright 2014 by Farmland Blossoms. All Rights Reserved. $
======================================================================== */

// dllmain.cpp : Defines the entry point for the DLL application.
// --

#include <Windows.h>
BOOL APIENTRY DllMain(HMODULE hModule,
                      DWORD  ul_reason_for_call,
                      LPVOID lpReserved
                      ) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

//extern "C" BOOL WINAPI _DllMainCRTStartup2(HANDLE  hDllHandle, DWORD   dwReason, LPVOID  lpreserved) {
//    return DllMain((HMODULE)hDllHandle, dwReason, lpreserved);
//}
//


// Include libraries
//#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "user32.lib")
//#pragma comment(lib, "winmm.lib")

//// No CRT
//extern "C" int _fltused{0x9875};
//#pragma function(memset)
//extern "C" void * __cdecl memset(_Out_writes_bytes_all_(_Size) void * _Dst, _In_ int _Val, _In_ size_t _Size) {
//    unsigned char Val = *(unsigned char *)&_Val;
//    unsigned char *At = (unsigned char *)_Dst;
//    while (_Size--) {
//        *At++ = Val;
//    }
//    return (At);
//}
