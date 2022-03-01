/* ========================================================================
$File: $
$Date: $
$Revision: $
$Creator: Roman Markovetski $
$Notice: (C) Copyright 2014 by Farmland Blossoms. All Rights Reserved. $
======================================================================== */

#ifndef __RC_X_W32_MEMORY_ALLOC_H_
#include "RaidCore_Common.h"

// ---------------------------------------------------------------------------------------------------------------
// memory
namespace memory_func
{
	void* Allocate(memory_int size);
	void Free(void* ptr);

	template<class T> inline T* ViAllocate(memory_int count)
	{
		return (T*)Allocate(sizeof(T) * count);
	}
}

#define __RC_X_W32_MEMORY_ALLOC_H_
#endif//__RC_X_W32_MEMORY_ALLOC_H_