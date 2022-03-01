/* ========================================================================
$File: $
$Date: $
$Revision: $
$Creator: Roman Markovetski $
$Notice: (C) Copyright 2014 by Farmland Blossoms. All Rights Reserved. $
======================================================================== */

// --
#include "../RaidCore/Headers/RaidCore_MemManager.h"

namespace game_memory {

	arena_p createMemoryArena(arena_p masterArena, memory_int srcLength, ArenaPushParams params) {
		if (0 == srcLength)
			return NULL;

		memory_int need = GetEffectiveSizeFor(masterArena, srcLength, params) + sizeof(arena);
		memory_int freeSpace = (masterArena->_allocLength - masterArena->_usedLength);
		if (0 == freeSpace)
			return NULL;

		arena_p ptr = (arena_p)((masterArena->_basePtr) + (masterArena->_usedLength));
		if ((RAIDCORE_EMPTY_VALUE == srcLength) || (need > freeSpace)) {
			need = freeSpace;
		}
		masterArena->_usedLength += need;

		ptr->_allocLength = need - sizeof(arena);
		ptr->_basePtr = ((uint8*)ptr) + sizeof(arena);
		ptr->_usedLength = 0;
		ptr->_tempCount = 0;
		ptr->_pStateManager = masterArena;
		ptr->_lock = {};

		alloc(ptr, GetAlignmentOffset(ptr, params.Alignment), NoAlign());
		return ptr;
	}

	MemoryBlock alloc(arena_p ptrMemoryState, memory_int request, ArenaPushParams params) {
		MemoryBlock result = { 0, nullptr };
		if (0 == request)
			return (result);

		lockThread(&ptrMemoryState->_lock);
		//
		memory_int freeSpace = (ptrMemoryState->_allocLength - ptrMemoryState->_usedLength);
		memory_int offset = GetAlignmentOffset(ptrMemoryState, params.Alignment);
		memory_int need = offset + GetAlignedSize(request, params.Alignment);
        
        Assert(need < freeSpace)
		if (need < freeSpace) {
			result.m_pMemory = (ptrMemoryState->_basePtr) + (ptrMemoryState->_usedLength + offset);
			ptrMemoryState->_usedLength += need;
			result.m_iMemoryLength = need;
		}
		//
		unlockThread(&ptrMemoryState->_lock);
		//
		if ((need < freeSpace) && (params.Flags & ArenaFlag_ClearToZero)) {
			if (request & ~0xf) {
				setBytes32(result.m_pMemory, 0, request);
			}
			else if (request & ~0x7) {
				setBytes16(result.m_pMemory, 0, request);
			}
			else {
				setBytes8(result.m_pMemory, 0, request);
			}
		}
		//
		return (result);
	}

    namespace paged_memory {
        namespace {
            rc_macro_force memory_block* getPageBlocks(memory_page* page) {
                return (memory_block*)(page + 1);
            }
        }

        paged_memory_arena create(arena_p mem, uint32 size, uint32 maxCount, uint32 align) {
            paged_memory_arena n = { 0 };
            n.alignment = align;
            n.arena = mem;
            n.dataSize = _Max(size, sizeof(memory_block)) + sizeof(memory_page);

            memory_int AlignmentMask = align - 1;
            if (n.dataSize & AlignmentMask) {
                uint32 offset = align - (n.dataSize & AlignmentMask);
                n.dataSize += offset;
            }
            n.pageSize = maxCount * n.dataSize;
            return (n);
        }
        void* allocate(paged_memory_arena* pm) {
            if (!pm->firstBlock) {
                memory_page* page = (memory_page*)PushSize(pm->arena, pm->pageSize).m_pMemory;
                uint32 freeSize = pm->pageSize;

                memory_block* b = getPageBlocks(page);
                while (freeSize >= pm->dataSize) {
                    b->next = (memory_block*)((uint8*)b + pm->dataSize);
                    freeSize -= pm->dataSize;
                    b = b->next;
                }
                b->next = NULL;

                pm->firstBlock = getPageBlocks(page);
                if (pm->firstPage) {
                    page->nextPage = pm->firstPage;
                }
                pm->firstPage = page;
            }

            memory_block* result = pm->firstBlock;
            pm->firstBlock = pm->firstBlock->next;

            return (void*)result;
        }
        rc_macro_cpp void free(paged_memory_arena* pm, void* ptr) {
            memory_block* block = (memory_block*)ptr;
            block->next = pm->firstBlock;
            pm->firstBlock = block;
        }
        rc_macro_cpp void reset(paged_memory_arena* pm) {
            pm->firstBlock = NULL;
            for (memory_page* page = pm->firstPage; page != NULL; page = page->nextPage) {
                uint32 freeSize = pm->pageSize;
                memory_block* b = getPageBlocks(page);
                while (freeSize >= pm->dataSize) {
                    b->next = (memory_block*)((uint8*)b + pm->dataSize);
                    freeSize -= pm->dataSize;
                    b = b->next;
                }
                b->next = pm->firstBlock;
                pm->firstBlock = getPageBlocks(page);
            }
        }
    }
}

//
//--------------------------------------------------------------------------------------------------------------------------------------------------
// Task memory
#include "../RaidCore/Headers/RaidCore_Task.h"

// BeginTemporaryMemory records the used memory position of the arena,
// later it is used to rewind the 
inline TemporaryMemory BeginTemporaryMemory(game_memory::arena *arena) {
    TemporaryMemory Result;

    lockThread(&arena->_lock);
    Result.arena = arena;
    Result.usedMemory = arena->_usedLength;
    Result.countId = ++arena->_tempCount;

    unlockThread(&arena->_lock);
    return(Result);
}

inline void EndTemporaryMemory(TemporaryMemory TempMem) {
    game_memory::arena *arena = TempMem.arena;
    Assert(arena->_usedLength >= TempMem.usedMemory);
    lockThread(&arena->_lock);
    if (arena->_usedLength > TempMem.usedMemory) {
        arena->_usedLength = TempMem.usedMemory;
    }
    Assert(arena->_tempCount > 0);
    --arena->_tempCount;
    unlockThread(&arena->_lock);
}

inline void verifyArenaClearState(game_memory::arena *arena) {
    Assert(arena->_tempCount == 0);
}

TaskWithMemory *BeginTaskWithMemory(TaskWithMemory *Tasks, uint32 count) {
    TaskWithMemory *FoundTask = 0;

    for (uint32 TaskIndex = 0;
         TaskIndex < count;
         ++TaskIndex) {
        TaskWithMemory *Task = Tasks + TaskIndex;
        if (!AtomicCompareExchangeUInt32((uint32*)&Task->beingUsed, true32, false32)) {
            FoundTask = Task;
            Task->memoryFlush = BeginTemporaryMemory(&Task->arena);
            break;
        }
    }

    return(FoundTask);
}

void EndTaskWithMemory(TaskWithMemory *Task) {
    EndTemporaryMemory(Task->memoryFlush);

    CompletePreviousWritesBeforeFutureWrites;
    Task->beingUsed = false;
}
