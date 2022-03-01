/* ========================================================================
$File: $
$Date: $
$Revision: $
$Creator: Roman Markovetski $
$Notice: (C) Copyright 2014 by Farmland Blossoms. All Rights Reserved. $
======================================================================== */

#include "../RaidCore/Headers/RaidCore_UtilsHeader.h"
#include "../RaidCore/Headers/RaidCore_GameState.h"
#include "../RaidCore/Headers/RaidCore_Task.h"

internal
inline intptr entityHashKey(intptr k) {
    return (k & 0x01fff);
}
void setupGameState(GameState* gameState) {
    gameState->screenPixelResolution = v2i(0, 0);

    hash_map::initialize(&gameState->gameWorld.regionHash, &gameState->entityArena, 256);
    gameState->gameWorld.regionHash.encodeKey = game_map_structs::regionHashKey;
    hash_map::initialize(&gameState->gameWorld.entityHash, &gameState->entityArena, 8192);
    gameState->gameWorld.entityHash.encodeKey = entityHashKey;
    gameState->gameWorld.lastEntiyId = 0;

    gameState->regionUnitDimentions = v3(25.f, 3.0f, 18.f);
}

void changeState(GameState* gameState, game_control_state state) {
	if (gameState->gameState != state) {
		switch (state) {
		case control_state_load_level: {
			break;
		}
		case control_state_load_level_stream: {
			break;
		}
		case control_state_pause: {
			break;
		}
		case control_state_run_mapTransition:
			gameState->inputData.Delta = v3(0.f);
			gameState->inputData.motionMagnitude = 0.f;
			gameState->inputData.motionDirection = v3(0.f);
			break;

		case control_state_run:
			if (gameState->gameState == control_state_run_mapTransition) {
				user_input::reset(gameState->gameInput, user_input::key_btn_A);
			}
			break;

		case control_state_shutdown: {
			break;
		}
		}
		gameState->gameState = state;
	}
}
