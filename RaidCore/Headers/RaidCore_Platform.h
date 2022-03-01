/* ========================================================================
$File: $
$Date: $
$Revision: $
$Creator: Roman Markovetski $
$Notice: (C) Copyright 2014 by Farmland Blossoms. All Rights Reserved. $
======================================================================== */

#ifndef __RC_X_PLATFORM_H_
#include "RaidCore_Common.h"
#include "RaidCore_MemManager.h"
#include "RaidCore_UtilsHeader.h"

#define PLATFORM_ALLOC(name) void* name(memory_int)
typedef PLATFORM_ALLOC(Allocate_Memory);

#define PLATFORM_FREE(name) void name(void*)
typedef PLATFORM_FREE(Free_Memory);

struct PlatformWorkQueue;
#define PLATFORM_WORK_QUEUE_CALLBACK(name) void name(PlatformWorkQueue *Queue, void *Data)
typedef PLATFORM_WORK_QUEUE_CALLBACK(PlatformWorkQueueCallback);

#define PLATFORM_ADD_ENTRY(name) void name(PlatformWorkQueue *Queue, PlatformWorkQueueCallback *callback, void *Data)
typedef PLATFORM_ADD_ENTRY(platform_add_entry);

#define PLATFORM_COMPLETE_ALL_WORK(name) void name(PlatformWorkQueue *Queue)
typedef PLATFORM_COMPLETE_ALL_WORK(platform_complete_all_work);

#define PLATFORM_LOAD_FILE(name) game_memory::MemoryBlock name(const char * path, game_memory::arena_p pArena)
typedef PLATFORM_LOAD_FILE(platform_load_file);

namespace platform_api {
    
    struct PlatformApi {
        Allocate_Memory *allocate;
        Free_Memory *free;

        platform_add_entry *addEntry;
        platform_complete_all_work *completeAllWork;

        platform_load_file *loadFile;
    };
}

#define __RC_X_PLATFORM_H_
#endif//__RC_X_PLATFORM_H_