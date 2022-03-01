/* ========================================================================
$File: $
$Date: $
$Revision: $
$Creator: Roman Markovetski $
$Notice: (C) Copyright 2014 by Farmland Blossoms. All Rights Reserved. $
======================================================================== */

#ifndef __RC_X_IMAGE_UTILS_H_

// --
#include "../RaidCore/Headers/RaidCore_Common.h"
#include "../RaidCore/Headers/RaidCore_MathOperators.h"

#pragma pack(push, 1)
struct tagBitmap
{
	uint16	bfType;
	uint32	bfSize;
	uint16	bfReserved1;
	uint16	bfReserved2;
	uint32	bfOffset;

	uint32	biSize;
	int32	biWidth;
	int32	biHeight;
	uint16	biPlanes;
	uint16	biBitCount;
	uint32	biCompression;
	uint32	biSizeImage;
	int32	biXPelsPerMeter;
	int32	biYPelsPerMeter;
	uint32	biClrUsed;
	uint32	biClrImportant;

	uint32	redMask;
	uint32	greenMask;
	uint32	blueMask;
};
#pragma pack(pop)

game_memory::MemoryBlock getBitmapRaw(uint8 * data, game_memory::arena_p pArena);

#define __RC_X_IMAGE_UTILS_H_
#endif//__RC_X_IMAGE_UTILS_H_
