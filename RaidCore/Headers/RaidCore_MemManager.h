/* ========================================================================
$File: $
$Date: $
$Revision: $
$Creator: Roman Markovetski $
$Notice: (C) Copyright 2014 by Farmland Blossoms. All Rights Reserved. $
======================================================================== */

#ifndef __RC_X_MEMORY_MGR_H_

#include "RaidCore_Common.h"
#include "RaidCore_MemoryAlloc.h"

namespace game_memory {
    struct arena {
        uint8 *
            _basePtr; // the base pointer to the start of the memory block
        memory_int
            _allocLength; // the max length of the memory block
        memory_int
            _usedLength; // the amount of memory in use
        arena*
            _pStateManager; //  the managinng arena (the master arena)
        //
        thread_lock
            _lock;
        //
        uint32 _tempCount;
    };
    //
    // <type> * const PTR ==> means:
    //  PTR is a const pointer, pointing to a location declared as <type>
    // you can change the values pointed by PTR, but cannot change PTR itself...
    typedef arena * arena_p;

    struct MemoryBlock {
        // length of allocated block
        memory_int
            m_iMemoryLength;
        //
        // The allocated block,
        // This is not used directly, only by internal first time create/destroy
        uint8 *
            m_pMemory;
    };

    enum eArenaPushFlag {
        ArenaFlag_ClearToZero = 0x1,
    };
    struct ArenaPushParams {
        uint32 Flags;
        uint32 Alignment;
    };

    inline ArenaPushParams
        DefaultArenaParams(void) {
        ArenaPushParams Params;
        Params.Flags = ArenaFlag_ClearToZero;
        Params.Alignment = 4;
        return(Params);
    }

    inline ArenaPushParams
        NoAlign(void) {
        ArenaPushParams Params;
        Params.Flags = 0;
        Params.Alignment = 1;
        return(Params);
    }

    inline ArenaPushParams
        AlignNoClear(uint32 Alignment) {
        ArenaPushParams Params = DefaultArenaParams();
        Params.Flags &= ~ArenaFlag_ClearToZero;
        Params.Alignment = Alignment;
        return(Params);
    }

    inline memory_int
        GetAlignedSize(memory_int SizeInit, memory_int Alignment) {
        memory_int AlignmentOffset = SizeInit;
        memory_int AlignmentMask = Alignment - 1;
        if (SizeInit & AlignmentMask) {
            AlignmentOffset += Alignment - (SizeInit & AlignmentMask);
        }

        return(AlignmentOffset);
    }

    inline memory_int
        GetAlignmentOffset(arena *arena, memory_int Alignment) {
        memory_int AlignmentOffset = 0;

        memory_int ResultPointer = (memory_int)arena->_basePtr + arena->_usedLength;
        memory_int AlignmentMask = Alignment - 1;
        if (ResultPointer & AlignmentMask) {
            AlignmentOffset = Alignment - (ResultPointer & AlignmentMask);
        }

        return(AlignmentOffset);
    }
    inline memory_int
        GetEffectiveSizeFor(arena *arena, memory_int SizeInit, ArenaPushParams Params = DefaultArenaParams()) {
        memory_int Size = SizeInit;

        memory_int AlignmentOffset = GetAlignmentOffset(arena, Params.Alignment);
        Size += AlignmentOffset;

        return(Size);
    }

    arena_p Initialize(MemoryBlock *, memory_int);
    void Terminate(MemoryBlock *);

    arena_p createMemoryArena(arena_p, const memory_int srcLength, ArenaPushParams param = DefaultArenaParams());
    //
    MemoryBlock alloc(arena_p, const memory_int blockSize, ArenaPushParams param = DefaultArenaParams());
    void free(arena_p, MemoryBlock*);
    memory_int _Initialize(memory_int, memory_int);
    // reset
    inline void resetArena(arena_p arena) {
        arena->_usedLength = 0;
        arena->_tempCount = 0;
    }

    namespace paged_memory {
        // paged memory lists
        struct memory_block {
            memory_block* next;
        };
        struct memory_page {
            memory_page* nextPage;
        };
        struct paged_memory_arena {
            uint32 dataSize;
            uint32 pageSize;
            uint32 alignment;

            memory_page* firstPage;
            memory_block* firstBlock;

            arena_p arena;
        };
        paged_memory_arena create(arena_p mem, uint32 size, uint32 maxSize, uint32 align);
        void* allocate(paged_memory_arena*);
        rc_macro_h void free(paged_memory_arena*, void* ptr);
        rc_macro_h void reset(paged_memory_arena*);
    }
}


///
/// UTILITIES...

#ifndef PushStructRef
#define PushStructRef(mem,s, ...) *((s*) game_memory::alloc(mem, sizeof(s), ## __VA_ARGS__).m_pMemory)
#endif

#ifndef PushArrayRef
#define PushArrayRef(mem,s,l, ...) *((s*) game_memory::alloc(mem, sizeof(s)*l, ## __VA_ARGS__).m_pMemory)
#endif

#ifndef PushSizeP
#define PushSizeP(mem,l, ...) ((uint8*) game_memory::alloc(mem, l, ## __VA_ARGS__).m_pMemory)
#endif

#ifndef PushStruct
#define PushStruct(arena, type, ...) (type *)game_memory::alloc(arena, sizeof(type), ## __VA_ARGS__).m_pMemory
#endif
#ifndef PushArray
#define PushArray(arena, Count, type, ...) (type *)game_memory::alloc(arena, (Count)*sizeof(type), ## __VA_ARGS__).m_pMemory
#endif
#ifndef PushSize
#define PushSize(arena, Size, ...) game_memory::alloc(arena, Size, ## __VA_ARGS__)
#endif

#define __RC_X_MEMORY_MGR_H_
#endif//__RC_X_MEMORY_MGR_H_
