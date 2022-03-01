/* ========================================================================
$File: $
$Date: $
$Revision: $
$Creator: Roman Markovetski $
$Notice: (C) Copyright 2014 by Farmland Blossoms. All Rights Reserved. $
======================================================================== */

// --
#include "Headers/RaidCore_Platform.h"
#include "OSHeaders\RaidCore_Win32_Struct.h"
#include <stdio.h>

////////////////////////////////////////////////////////////////////////////////////////////////
////// TIMER!!!!
//////
namespace game_timer {
    struct RaidTick {
        LARGE_INTEGER lastMark;
        LARGE_INTEGER currentTime;

        LARGE_INTEGER systemTicksPerSecond;
    };

    rc_macro_force real32 getElapsedSeconds(RaidTick * pTick) // tenth of a miliseconds passed
    {
        // Not initializing variables,
        // They are guarenteed to be initialized before use, saveing cpu cycles here!
        LARGE_INTEGER tmp;
        real32 diff;

        if (QueryPerformanceCounter(&tmp)) {
            pTick->currentTime.QuadPart = tmp.QuadPart;
        }
        // diff - elapsed mili-ticks
        diff = (uint32)(pTick->currentTime.QuadPart - pTick->lastMark.QuadPart);

        // reply - elapsed miliseconds
        return (diff / ((real32)pTick->systemTicksPerSecond.QuadPart));
    }
    inline int64 Diff(RaidTick * pTick) // tenth of a miliseconds passed
    {
        // Not initializing variables,
        // They are guarenteed to be initialized before use, saveing cpu cycles here!
        LARGE_INTEGER tmp;
        int64 diff;
        int64 reply;

        if (QueryPerformanceCounter(&tmp)) {
            pTick->currentTime.QuadPart = tmp.QuadPart;
        }
        // diff - elapsed mili-ticks
        diff = (pTick->currentTime.QuadPart - pTick->lastMark.QuadPart);
        diff *= 1000;

        // reply - elapsed miliseconds
        reply = (diff / pTick->systemTicksPerSecond.QuadPart);
        return reply;
    }
    inline void Mark(RaidTick * pTick) // miliseconds passed
    {
        pTick->lastMark.QuadPart = pTick->currentTime.QuadPart;
    }

    void Init(RaidTick* m) {
        m->currentTime.QuadPart = m->lastMark.QuadPart = 0;
        QueryPerformanceFrequency(&m->systemTicksPerSecond);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//
