/* ========================================================================
$File: $
$Date: $
$Revision: $
$Creator: Roman Markovetski $
$Notice: (C) Copyright 2014 by Farmland Blossoms. All Rights Reserved. $
======================================================================== */

#include "../RaidCore/Headers/RaidCore_Common.h"

#include "../RaidCore/Headers/RaidCore_MemManager.h"
#define VM_FLAGS MEM_COMMIT|MEM_RESERVE
#define VM_PAGE_OPS PAGE_READWRITE

//
#include <windows.h>

// ---------------------------------------------------------------------------------------------------------------
// memory
namespace memory_func {
    void* Allocate(memory_int size) {
#ifdef RC_TESTCODE
        LPVOID BaseAddress = 0;
#else
        LPVOID BaseAddress = 0;
#endif
        return VirtualAlloc(BaseAddress, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    }

    void Free(void* ptr) {
        VirtualFree(ptr, 0, MEM_RELEASE);
    }
}

