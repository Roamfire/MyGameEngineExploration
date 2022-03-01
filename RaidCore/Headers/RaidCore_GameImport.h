/* ========================================================================
$File: $
$Date: $
$Revision: $
$Creator: Roman Markovetski $
$Notice: (C) Copyright 2014 by Farmland Blossoms. All Rights Reserved. $
======================================================================== */

#ifndef __RC_GAME_IMPORT_H_
#include "RaidCore_Common.h"
#include "RaidCore_MemManager.h"
#include "RaidCore_UtilsHeader.h"
#include "RaidCore_Platform.h"

struct GameState;
struct GameMemory {
    struct GameState * gameState;
    struct platform_api::PlatformApi platformApi;

    PlatformWorkQueue *HighPriorityQueue;
    PlatformWorkQueue *LowPriorityQueue;

    // Arenas
    game_memory::arena_p
        GamePersistentMemoryArena;
    game_memory::arena_p
        GameTranscientMemoryArena;
    game_memory::arena_p
        GameRunArena;

#ifdef RC_TESTCODE
    struct debug_timer *DEBUGGlobalMemory;
    uint32 DEBUGFrameCounter;
#endif

};

#include "../../RaidCore_ExportDefines.h"

#define __RC_GAME_IMPORT_H_
#endif//__RC_GAME_IMPORT_H_