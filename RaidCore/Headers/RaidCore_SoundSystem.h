/* ========================================================================
$File: $
$Date: $
$Revision: $
$Creator: Roman Markovetski $
$Notice: (C) Copyright 2014 by Farmland Blossoms. All Rights Reserved. $
======================================================================== */

#ifndef __RC_X_SOUND_SYSTEM_H_
#include "RaidCore_Common.h"
#include "RaidCore_UtilsHeader.h"
#include "RaidCore_MemManager.h"

#if COMPILER_MSVC
#pragma warning(disable:4200)
#else
#endif

//
namespace game_sound
{
	struct SoundSource;
    enum SoundElementState {
        idle,
        playing,
        unload,
        error
    };
	struct SoundPlayBuffer;
	struct SoundPlayCollection;

    struct sound_piece {
        uint32 bytesPerSample;
        //
        uint32 lastReadPosition;
        uint32 length;
        uint8* source;
        //
        // next in chain, or next in hash...
//        sound_piece*
//            next;
    };

    struct SoundPlayBuffer {
        // Lock buffer
        void * firstBuffer;
        uint32 firstLength;
        // lock circular overflow
        void * secondBuffer;
        uint32 secondLength;
    };

    struct GameSoundHandler {
        uint16 SamplesPerSecond;
        uint32 BufferSize;
        uint16 Channels;
        uint16 BytesPerSample;

        SoundSource * soundSource;
        SoundPlayCollection* playingElements;

        hash_map::map<sound_piece> playingSounds;
    };

    GameSoundHandler
        DefaultHandler() {
        GameSoundHandler result = {};
        result.BytesPerSample = 2; // 16bit
        result.Channels = 2;
        return (result);
    }
}

#define __RC_X_SOUND_SYSTEM_H_
#endif//__RC_X_SOUND_SYSTEM_H_
