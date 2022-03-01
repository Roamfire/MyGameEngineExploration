/* ========================================================================
$File: $
$Date: $
$Revision: $
$Creator: Roman Markovetski $
$Notice: (C) Copyright 2014 by Farmland Blossoms. All Rights Reserved. $
======================================================================== */

// --
#include "../RaidCore/Headers/RaidCore_SoundSystem.h"

namespace game_sound {
	inline uint32 maxBufferSize(GameSoundHandler* Sound) {
        return (Sound->SamplesPerSecond * Sound->BytesPerSample * Sound->Channels);
	}
}
