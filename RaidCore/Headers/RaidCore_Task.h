/* ========================================================================
$File: $
$Date: $
$Revision: $
$Creator: Roman Markovetski $
$Notice: (C) Copyright 2014 by Farmland Blossoms. All Rights Reserved. $
======================================================================== */

#ifndef __RC_TASK_H_
#include "RaidCore_MemManager.h"

struct TemporaryMemory {
    game_memory::arena *arena;
    memory_int usedMemory;
    uint32 countId;
};

struct TaskWithMemory {
    bool32 beingUsed;
    game_memory::arena arena;

    TemporaryMemory memoryFlush;
};

#define __RC_TASK_H_
#endif//__RC_TASK_H_
