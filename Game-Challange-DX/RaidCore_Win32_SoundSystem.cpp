/* ========================================================================
$File: $
$Date: $
$Revision: $
$Creator: Roman Markovetski $
$Notice: (C) Copyright 2014 by Farmland Blossoms. All Rights Reserved. $
======================================================================== */

// --
#include "../RaidCore/Headers/RaidCore_SoundSystem.h"

#include <windows.h>
#include <Mmreg.h>
#include <dsound.h>

#define DIRECT_SOUND_CREATE(name) HRESULT name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter);
typedef DIRECT_SOUND_CREATE(direct_sound_create);

namespace game_sound {
    struct SoundSource {
        HWND hWnd;
        HMODULE hSound;
        LPDIRECTSOUND DirectSound;
        LPDIRECTSOUNDBUFFER PrimaryBuffer = NULL;

        WAVEFORMATEX wfmt = {};
    };
    struct SoundBuffer {
        SoundID Id;
        SoundElementState State;
        LPDIRECTSOUNDBUFFER PlayBuffer = NULL;

    };
    struct SoundPlayCollection {
        rc_list::list<SoundBuffer> playing;
        rc_list::list<SoundBuffer> idle;
    };

    bool32 InitSoundSystem(GameSoundHandler* handler) {
        // Load lobrary
        if (NULL == handler->soundSource->hSound) {
            handler->soundSource->hSound = LoadLibraryA("dsound.dll");
        }
        if (NULL != handler->soundSource->hSound) {
            direct_sound_create* drc = (direct_sound_create*)GetProcAddress(handler->soundSource->hSound, "DirectSoundCreate");
            // Create the sound
            if (NULL != handler->soundSource->DirectSound) {
                handler->soundSource->DirectSound->Release();
                handler->soundSource->DirectSound = NULL;
            }
            if (drc && SUCCEEDED(drc(0, &handler->soundSource->DirectSound, NULL))) {
                if (SUCCEEDED(handler->soundSource->DirectSound->SetCooperativeLevel(handler->soundSource->hWnd, DSSCL_PRIORITY))) {
                    DSBUFFERDESC sbDesc = { sizeof(DSBUFFERDESC) };
                    sbDesc.dwFlags = DSBCAPS_PRIMARYBUFFER;
                    sbDesc.dwBufferBytes = 0; // for primary

                    if (handler->BufferSize == 0) {
                        // Note(Roman): set buffer to a 1 sec buffer time
                        handler->BufferSize = maxBufferSize(handler);
                    }

                    handler->soundSource->wfmt = {};
                    WAVEFORMATEX& wfmt = handler->soundSource->wfmt;
                    wfmt.wFormatTag = WAVE_FORMAT_PCM;         /* format type */
                    wfmt.nChannels = handler->Channels;          /* number of channels (i.e. mono, stereo...) */
                    wfmt.wBitsPerSample = handler->BytesPerSample * 8;     /* number of bits per sample of mono data */
                    wfmt.nSamplesPerSec = handler->SamplesPerSecond;     /* sample rate */
                    wfmt.nBlockAlign = handler->Channels * handler->BytesPerSample;        /* block size of data */
                    wfmt.nAvgBytesPerSec = handler->SamplesPerSecond * wfmt.nBlockAlign;    /* for buffer estimation */
                    wfmt.cbSize = 0;             /* the count in bytes of the size of */

                    if (NULL != handler->soundSource->PrimaryBuffer) {
                        handler->soundSource->PrimaryBuffer->Release();
                        handler->soundSource->PrimaryBuffer = NULL;
                    }
                    if (SUCCEEDED(handler->soundSource->DirectSound->CreateSoundBuffer(&sbDesc, &handler->soundSource->PrimaryBuffer, NULL))) {
                        return SUCCEEDED(handler->soundSource->PrimaryBuffer->SetFormat(&wfmt));
                    }
                }
            }
        }
        return false32;
    }
    void ShutdownSoundSystem(GameSoundHandler* handler) {
        if (NULL != handler->soundSource->PrimaryBuffer) {
            handler->soundSource->PrimaryBuffer->Release();
            handler->soundSource->PrimaryBuffer = NULL;
        }
        if (NULL != handler->soundSource->DirectSound) {
            handler->soundSource->DirectSound->Release();
            handler->soundSource->DirectSound = NULL;
        }
        // Load lobrary
        if (NULL != handler->soundSource->hSound) {
            FreeLibrary(handler->soundSource->hSound);
            handler->soundSource->hSound = NULL;
        }
    }
    bool32 InitSound(GameSoundHandler* handler, SoundBuffer* sound) {
        sound->State = error;
        if (handler->BufferSize == 0) {
            // Note(Roman): set buffer to a 1 sec buffer time
            handler->BufferSize = maxBufferSize(handler);
        }

        DSBUFFERDESC sbDesc = { sizeof(DSBUFFERDESC) };
        sbDesc.dwFlags = DSBCAPS_GETCURRENTPOSITION2;
        sbDesc.dwBufferBytes = handler->BufferSize;
        sbDesc.lpwfxFormat = &handler->soundSource->wfmt;
        if (SUCCEEDED(handler->soundSource->DirectSound->CreateSoundBuffer(&sbDesc, &sound->PlayBuffer, NULL))) {
            sound->State = idle;
            return true32;
        }
        return false32;
    }
}
