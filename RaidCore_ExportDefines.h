/* ========================================================================
$File: $
$Date: $
$Revision: $
$Creator: Roman Markovetski $
$Notice: (C) Copyright 2014 by Farmland Blossoms. All Rights Reserved. $
======================================================================== */

#ifndef __RAID_EXPORT_DEFINES_H_

#define GAME_INDEPENDANT_PROCESS(name) void name(GameMemory* gameMemory, user_input::GameInput* input, real32 frameTime)
typedef GAME_INDEPENDANT_PROCESS(Game_Independant_Process);

#define GAME_UPDATE_AND_RENDER(name) void name(GameMemory* gameMemory, user_input::GameInput* input, real32 frameTime)
typedef GAME_UPDATE_AND_RENDER(Game_UpdateAndRender);

#define UPDATE_GAME_STATE(name) void name(GameMemory* gameMemory, int32 screenWidth, int32 screenHeight)
typedef UPDATE_GAME_STATE(Update_Game_State);

#define INIT_GAME_STATE(name) void name(GameMemory* gameMemory, user_input::GameInput* input, game_render_engine::PlatformWindow* window)
typedef INIT_GAME_STATE(Init_Game_State);

#define SHUTDOWN_GAME_STATE(name) void name(GameMemory* gameMemory)
typedef SHUTDOWN_GAME_STATE(Shutdown_Game_State);

#define GAME_GET_SOUND_BUFFERS(name) void name(GameMemory* gameMemory, real32, real32, game_sound::SoundPlayBuffer* pBuffer, game_sound::sound_piece* newSound)
typedef GAME_GET_SOUND_BUFFERS(Game_Get_Sound_Buffers);

#define __RAID_EXPORT_DEFINES_H_
#endif

