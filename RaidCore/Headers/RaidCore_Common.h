/* ========================================================================
$File: $
$Date: $
$Revision: $
$Creator: Roman Markovetski $
$Notice: (C) Copyright 2014 by Farmland Blossoms. All Rights Reserved. $
======================================================================== */

#ifndef __RC_X_COMMON_H_

#include <stdint.h>
#include <stddef.h>
#include <limits.h>
#include <float.h>

// FOR DEBUG
#ifdef RC_TESTCODE
#define Assert(exp) {if(!(exp)) {*(int*)0 = 0;} }
#else
#define Assert(exp)
#endif

#define AssertNotImplemented Assert(!"Not implemented")
#define AssertInvalidCodePath Assert(!"Invalid path")

#define SignedMaxLimit(type) ~(1 << (sizeof(type) * 8 - 1))

#define GAMEHANDLER_API extern "C"

#if !defined(COMPILER_MSVC)
#define COMPILER_MSVC 0
#endif

#if !defined(COMPILER_LLVM)
#define COMPILER_LLVM 0
#endif

#if !COMPILER_MSVC && !COMPILER_LLVM
#if _MSC_VER
#undef COMPILER_MSVC
#define COMPILER_MSVC 1
#else
#undef COMPILER_LLVM
#define COMPILER_LLVM 1
#endif
#endif

#if COMPILER_MSVC
#include <intrin.h>
#elif COMPILER_LLVM
#include <x86intrin.h>
#else
#error unknown compiler, not supported!
#endif

//
// Defines to bring unity!!!
typedef intptr_t intptr;
typedef uintptr_t memory_int;

typedef uint8_t uint8, u8;
typedef uint16_t uint16, u16;
typedef uint32_t uint32, u32;
typedef unsigned long ul;
typedef uint64_t uint64, u64;
//
typedef int8_t int8, s8;
typedef int16_t int16, s16;
typedef int32_t int32, s32;
typedef long sl;
typedef int64_t int64, s64;
typedef float real32, r32, f32;
typedef double real64, r64, f64;
//
typedef int32 bool32;
#define true32 1
#define false32 0
#define TestResult(comp) (false32 != (comp))
#define TestFailedResult(comp) (false32 == (comp))
//

//-----------------------------------------------
template<typename T> void Swap(T* p1, T* p2)
{
	T temp = *p1;
	*p1 = *p2;
	*p2 = temp;
}
//-----------------------------------------------

#pragma pack(push, 1)
struct BitmapID {
    uint32 Value;
};

struct FontID {
    uint32 Value;
};

struct SoundID {
    uint32 Value;
};
#pragma pack(pop)

//-----------------------------------------------
// Vector & Matrix math struxtures
//
// Note(Roman): int based vectors
union vec2i {
    int32 E[2];
    struct {
        int32
            x,
            y;
    };
};

union vec3i {
    int32 E[3];
    struct {
        vec2i xy;
        int32
            _ignore;
    };
    struct {
        int32
            x,
            y,
            z;
    };
};

union vec4i {
    int32 E[4];
    struct {
        vec2i _xy;
        int32
            _ignore0,
            _ignore1;
    };
    struct {
        vec3i xyz;
        int32
            _ignore2;
    };
    struct {
        int32
            x,
            y,
            z,
            w;
    };
};

//
// Note(Roman): float based vectors
union vec2 {
    real32 E[2];
    struct {
        real32
            x,
            y;
    };
};

union vec3 {
    real32 E[3];
    struct {
        vec2 xy;
        real32
            _ignore0;
    };
    struct {
        real32
            x,
            y,
            z;
    };
};

union vec4 {
    real32 E[4];
    struct {
        vec2 xy;
        real32
            _ignore0,
            _ignore1;
    };
    struct {
        vec3 xyz;
        real32
            _ignore2;
    };
    struct {
        real32
            x,
            y,
            z,
            w;
    };
};

struct GfxRect {
    vec2
        minPoint,
        maxPoint;
};
struct GfxBox {
    vec3
        minPoint,
        maxPoint;
};

struct GfxVolume {
    vec3
        offset; // offset from an origin point, defined by logical connection to containing entity
    vec3
        radialDimention; // the dimentions on each axis from the center, to the edges.
};

union mat4x4{
    real32 E[4][4];
    struct {
        union {
            vec4 vx;
            real32 x[4];
        };
        union {
            vec4 vy;
            real32 y[4];
        };
        union {
            vec4 vz;
            real32 z[4];
        };
        union {
            vec4 vw;
            real32 w[4];
        };
    };
};
struct inv4x4 {
    mat4x4 set;
    mat4x4 inverse;
};

#define U32FromPointer(Pointer) ((u32)(memory_int)(Pointer))
#define PointerFromU32(type, Value) (type *)((memory_int)Value)

#define OffsetOf(type, Member) (memory_int)&(((type *)0)->Member)
//
#define rc_macro_h inline
#define rc_macro_cpp inline
#define rc_macro_force __forceinline
//

#define KByte(value) ((value) << 10)
#define MByte(value) (KByte(value) << 10)
#define GByte(value) (((uint64)MByte(value)) << 10)
//
#ifndef NULL
#define NULL 0
#endif
//

#define internal static
#define global static

#define Pi32 3.14159265359f
#define Tau32 6.28318530717958647692f

/////////////////////////////////////////////////////////
#define RAIDCORE_EMPTY_VALUE ((memory_int)-1)
#define _Max(a,b) ((a)<(b) ? (b) : (a))
#define _Min(a,b) ((a)<(b) ? (a) : (b))

/////////////////////////////////////////////////////////
template<class _Ty> struct CmpLess
{	// functor for operator<
	bool32 operator()(const _Ty& _Left, const _Ty& _Right) const
	{	// apply operator< to operands
		return (_Left < _Right);
	}
};


#ifdef __cplusplus
GAMEHANDLER_API{
#endif

#ifdef __cplusplus
}
#endif

#define Align4(Value) ((Value + 3) & ~3)
#define Align8(Value) ((Value + 7) & ~7)
#define Align16(Value) ((Value + 15) & ~15)
#define AlignN(Value, N) ((Value + (N)) & ~(N))

#define sizeof4(Value) ((sizeof(Value) + 3) & ~3)
#define sizeof8(Value) ((sizeof(Value) + 7) & ~7)
#define sizeof16(Value) ((sizeof(Value) + 15) & ~15)

#define ArrayCount(x) (sizeof(x)/sizeof(x[0]))

inline uint32
safeCast2U32(uint64 Value)
{
	Assert(Value <= (uint64)UINT_MAX);
	uint32 result = (uint32)Value;
	return (result);
}

inline uint16
safeCast2U16(uint32 Value)
{
	Assert(Value <= (uint32)USHRT_MAX);
	uint16 result = (uint16)Value;
	return (result);
}
inline uint8
safeCast2U8(uint32 Value)
{
	Assert(Value <= (uint32)UCHAR_MAX);
	uint8 result = (uint8)Value;
	return (result);
}
inline uint8
safeCast2U8(uint16 Value)
{
	Assert(Value <= (uint16)UCHAR_MAX);
	uint8 result = (uint8)Value;
	return (result);
}

inline void
setBytes8(void* dst, uint8 value, memory_int length)
{
	uint8* dest = (uint8*)dst;
	for (uint32 i = 0; i < length; ++i)
	{
		*dest++ = value;
	}
}
inline void
setBytes16(void* dst, uint16 value, memory_int length)
{
	uint16* dest = (uint16*)dst;
	for (uint32 i = 0; i < length; i += 2)
	{
		*dest++ = value;
	}
}
inline void
setBytes32(void* dst, uint32 value, memory_int length)
{
	uint32* dest = (uint32*)dst;
	for (uint32 i = 0; i < length; i += 4)
	{
		*dest++ = value;
	}
}
inline void
setBytes(void* dst, uint8 value, memory_int length) {
    if (length & 0xf) {
        uint32* dest32 = (uint32*)dst;
        memory_int len8 = length & ~0xf;
        const uint32 v32 = value << 24 | value << 16 | value << 8 | value;
        for (uint32 i = 0; i < len8; i += 4) {
            *dest32++ = v32;
        }
        length -= len8;
        dst = dest32;
    }
    if (length & 0x7) {
        uint32* dest16 = (uint32*)dst;
        memory_int len8 = length & ~0x7;
        const uint16 v32 = value << 8 | value;
        for (uint32 i = 0; i < len8; i += 2) {
            *dest16++ = v32;
        }
        length -= len8;
        dst = dest16;
    }
    uint8* dest = (uint8*)dst;
    for (uint32 i = 0; i < length; i += 4) {
        *dest++ = value;
    }
}

#include "RaidCore_OS_Common.h"

struct thread_lock {
    uint64 lockNum;
    uint64 lockCount;
};
inline void lockThread(thread_lock* lock) {
    uint64 num = AtomicAddU64(&lock->lockNum, 1);
    while (AtomicCompareU64(&lock->lockCount, num) != lock->lockCount) { _mm_pause(); }
}
inline void unlockThread(thread_lock* lock) {
    AtomicAddU64(&lock->lockCount, 1);
}

//--------------------------------------------------------------------------------------------------
#ifdef RC_TESTCODE

// NOTE(Roman): Show the collision bounding box of an entity
// 0 - disabled
// 1 - enabled
#define _SHOW_AABB_IMAGE 1
#define _SHOW_AABB_PLAYER 0
#define _SHOW_AABB_NPC 0
#define _SHOW_AABB_WALL 0
// NOTE(Roman): Show the tile aligned boundries
// 0 - disabled
// 1 - enabled
#define _SHOW_TILEGRID 0
// NOTE(Roman): Show the render regions, where regions are 
// 0 - disabled
// 1 - enabled
#define _SHOW_RENDER_REGION 1

#else

// NOTE(Roman): always keep 0, in none test code these are always off
#define _SHOW_AABB_IMAGE 0
#define _SHOW_AABB_PLAYER 0
#define _SHOW_AABB_NPC 0
#define _SHOW_AABB_WALL 0
#define _SHOW_TILEGRID 0
#define _SHOW_RENDER_REGION 0

#endif

#define __RC_X_COMMON_H_
#endif//__RC_X_COMMON_H_
