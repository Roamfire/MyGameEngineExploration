/* ========================================================================
$File: $
$Date: $
$Revision: $
$Creator: Roman Markovetski $
$Notice: (C) Copyright 2014 by Farmland Blossoms. All Rights Reserved. $
======================================================================== */

#ifndef __GH_GAMEHANDLER_MGR_H_
#include "../RaidCore/Headers/RaidCore_Common.h"
#include "../RaidCore/Headers/RaidCore_RenderHandler.h"
#include "../RaidCore/Headers/RaidCore_GameState.h"

//------------------------------------------------------------------------------------------
void InitAndLoadMapLevel(GameState* gameState, const char * map_name);
void SetControlledEntity(GameState* gameState, uint32 id);
void SetTargetEntity(GameState* gameState, uint32 follower_id, uint32 followed_id);
//------------------------------------------------------------------------------------------
void UpdateCamera(GameState* gameState, real32 dTinSec);
vec3 calculateMotionProfile(vec3& iPtSpeed, game_physics::MotionProfile * motion, real32 dTinSec);
vec3 Apply_MotionProfile(GameState* gameState, WorldAreaEntity* entity, game_physics::MotionProfile& motion_profile, real32 dTinSec);

struct SimUpdateEntity {
    vec3
        iPtSpeed;	// Current velocity of the entity, used when handling the entity motion/collision update
    vec3
        renderOffset; // distance from simulation center, used to calculate the perspective scaling factor
    //
    game_entity_type entityType;
    EntityPiece* base;

    bool32 didUpdate;
    entity_id updateId;
    TEntityReference entityRef;
};
typedef rc_list::list<SimUpdateEntity> UpdateEntityList;
typedef rc_list::iterator<SimUpdateEntity> UpdateEntityListIterator;

#define __GH_GAMEHANDLER_MGR_H_
#endif 
