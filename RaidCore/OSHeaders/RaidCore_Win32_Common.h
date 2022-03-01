/* ========================================================================
$File: $
$Date: $
$Revision: $
$Creator: Roman Markovetski $
$Notice: (C) Copyright 2014 by Farmland Blossoms. All Rights Reserved. $
======================================================================== */

/*
DO NOT INCLUDE THIS DIRECTLY!!!
*/

#ifndef __RAID_W32_COMMON_H_

#if COMPILER_MSVC
#define CompletePreviousReadsBeforeFutureReads _ReadBarrier()
#define CompletePreviousWritesBeforeFutureWrites _WriteBarrier()

#define CompleteStore _mm_sfence()
#define CompleteLoad _mm_lfence()
#define PauseWait _mm_pause()

#ifdef __cplusplus
GAMEHANDLER_API {
#endif

	inline uint32 AtomicCompareUInt32(uint32 volatile *Value, uint32 Expected)
	{
		uint32 Result = (uint32)_InterlockedAnd((long volatile *)Value, Expected);

		return(Result);
	}
	inline uint64 AtomicCompareU64(uint64 volatile *Value, uint64 Expected)
	{
		uint64 Result = (uint64)_InterlockedAnd64((int64 volatile*)Value, Expected);

		return(Result);
	}
	inline uint32 AtomicCompareExchangeUInt32(uint32 volatile *Value, uint32 New, uint32 Expected)
	{
		uint32 Result = (uint32)_InterlockedCompareExchange((long volatile *)Value, New, Expected);

		return(Result);
	}
	inline uint64 AtomicCompareExchangeU64(uint64 volatile *Value, uint64 New, uint64 Expected)
	{
		uint64 Result = (uint64)_InterlockedCompareExchange64((int64 volatile*)Value, New, Expected);

		return(Result);
	}
	inline uint32 AtomicExchangeU32(uint32 volatile *Value, uint32 New)
	{
		uint32 Result = (uint32)_InterlockedExchange((long volatile*)Value, New);

		return(Result);
	}
	inline uint64 AtomicExchangeU64(uint64 volatile *Value, uint64 New)
	{
		uint64 Result = (uint64)_InterlockedExchange64((int64 volatile*)Value, New);

		return(Result);
	}
	inline uint64 AtomicAddU64(uint64 volatile *Value, uint64 Addend)
	{
		// NOTE(Roman): Returns the original value _prior_ to adding
		uint64 Result = (uint64)_InterlockedExchangeAdd64((int64 volatile *)Value, Addend);

		return(Result);
	}
	inline uint32 GetThreadID(void)
	{
		uint8 *ThreadLocalStorage = (uint8 *)__readgsqword(0x30);
		uint32 ThreadID = *(uint32 *)(ThreadLocalStorage + 0x48);

		return(ThreadID);
		return (0);
	}
    inline void copyMemory(uint8* dst, uint8* src, uint32 copyLen) {
        uint32 i64 = copyLen & ~0x0f;
        while (i64) {
            *(uint64*)dst = *(uint64*)src;
            dst += sizeof(uint64);
            src += sizeof(uint64);
            i64 -= sizeof(uint64);
        }
        uint32 i32 = copyLen & 0x0f;
        while (i32) {
            *dst = *src;
            ++dst;
            ++src;
            --i32;
        }
    }

#ifdef __cplusplus
}
#endif
#endif//COMPILER_MSVC

#ifndef CODE_ALIGN
#define CODE_ALIGN(x) __declspec(align(x))
#endif

#define __RAID_W32_COMMON_H_
#endif//__RAID_W32_COMMON_H_
