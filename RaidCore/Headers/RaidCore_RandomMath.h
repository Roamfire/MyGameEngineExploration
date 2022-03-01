/* ========================================================================
$File: $
$Date: $
$Revision: $
$Creator: Roman Markovetski $
$Notice: (C) Copyright 2014 by Farmland Blossoms. All Rights Reserved. $
======================================================================== */

#ifndef __RC_X_RANDOM_H_

#include "RaidCore_Common.h"
#include "RaidCore_Math.h"

#define MaxRandomNumber 0x05f5c21f
#define MinRandomNumber 0x000025a0

namespace random
{
	extern uint32 RandomNumberTable[];

	struct random_series
	{
		uint32 Index;
	};

	random_series RandomSeed(uint32 Value);
	uint32 RandomNextuint32(random_series *Series);

	inline uint32 RandomChoice(random_series *Series, uint32 ChoiceCount)
	{
		uint32 result = (RandomNextuint32(Series) % ChoiceCount);

		return (result);
	}

	inline real32 RandomUnilateral(random_series *Series)
	{
		real32 Divisor = 1.0f / (real32)MaxRandomNumber;
		real32 result = Divisor*(real32)RandomNextuint32(Series);

		return (result);
	}

	inline real32 RandomBilateral(random_series *Series)
	{
		real32 result = 2.0f*RandomUnilateral(Series) - 1.0f;

		return (result);
	}

	inline real32 RandomBetween(random_series *Series, real32 Min, real32 Max)
	{
		real32 result = game_math::lerp(Min, RandomUnilateral(Series), Max);

		return (result);
	}

	inline int32 RandomBetween(random_series *Series, int32 Min, int32 Max)
	{
		int32 result = Min + (int32)(RandomNextuint32(Series) % ((Max + 1) - Min));

		return (result);
	}
}

#define __RC_X_RANDOM_H_
#endif//__RC_X_RANDOM_H_
