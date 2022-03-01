/* ========================================================================
$File: $
$Date: $
$Revision: $
$Creator: Roman Markovetski $
$Notice: (C) Copyright 2014 by Farmland Blossoms. All Rights Reserved. $
======================================================================== */

// GameHandler_Management.cpp : Defines the exported functions for the DLL application.
//

#include "../RaidCore/Headers/RaidCore_Platform.h"
// --
#include "GameHandler.h"

#include "../RaidCore/Headers/RaidCore_MemManager.h"
#include "../RaidCore/Headers/RaidCore_GameWorld.h"
#include "../RaidCore/Headers/RaidCore_GameState.h"
#include "../RaidCore/Headers/RaidCore_MathOperators.h"
#include "../RaidCore/Headers/RaidCore_Physics.h"

#include "../RaidCore/Headers/RaidCore_WorldCoordinates.h"

#include "../GameHandler/GameHandler_Management.h"
#include "../GameHandler/RaidCore_GameEntities.h"
#include "../RaidCore/Headers/RaidCore_RandomMath.h"

#ifdef RC_TESTCODE
#include "../GameHandler/Debug.cpp"
#else
#include "../GameHandler/Debug.cpp"
#endif

namespace rc_list {
	block<SimUpdateEntity>* list<SimUpdateEntity>::freeBlock = NULL;
}

void InitAndLoadMapLevel(GameState* gameState, const char * map_name) {
    // set sim region to initial position at the top left corner of the world.
    game_map_structs::GameWorld & world = gameState->gameWorld;
    //
    if (gameState->gameState == control_state_load_level) {

		DEBUG_MAPInit(gameState);

        gameState->gameState = control_state_run;
    }
}

void SetControlledEntity(GameState* gameState, uint32 entity_id) {
    AssertNotImplemented;
}

void SetTargetEntity(GameState* gameState, uint32 entity_id, uint32 target_id) {
    AssertNotImplemented;
}

// NOTE(Roman): camera control...
// 1. follow PC
// 2. follow PC within region, stop at edge, transtion to new region when PC changes location
void UpdateCamera(GameState* gameState, real32 dTinSec) {
    // 
    // Camera centers on the player by default...
    WorldAreaEntity * entity = gameState->controlledEntity;
    if (NULL != gameState->cameraTargetEntity) {
        AssertInvalidCodePath;
    }

    vec3 va = {};
    switch (gameState->cameraEntity.state) {
        case CameraState_EntityFollowScroll: 
            if (entity) {
                coord_system::RegionCoord target = entity->iWorldPosition;
                target.offset.y = 0.0f;
                vec3 toTarget = coord_system::getDistance(target, gameState->cameraEntity.iWorldPosition, gameState->regionUnitDimentions);

                real32 distanceFromTarget = game_math::lengthSq(toTarget);
                if (distanceFromTarget <= game_math::epsilon && game_math::isZero(gameState->cameraEntity.iWorldPosition.offset.y)) {
                    return;
                }

                // NOTE(Roman): Speed camera correction, if distance is greater then a full area teleport camera to the nearest area adjacent to target
                if (distanceFromTarget > game_math::lengthSq(gameState->regionUnitDimentions)) {
                    va = toTarget * (1.0f - (game_math::lengthSq(gameState->regionUnitDimentions) / distanceFromTarget));
                    break;
                }

                if (game_math::isZero(toTarget.x)) {
                    va.x = 0.0f;
                } else {
                    va.x = toTarget.x;
                }
                if (game_math::isZero(toTarget.y)) {
                    va.y = 0.0f;
                } else {
                    va.y = toTarget.y;
                }
                if (game_math::isZero(toTarget.z)) {
                    va.z = 0.0f;
                } else {
                    va.z = toTarget.z;
                }

                // NOTE(Roman): accelerate the camera reletively to the target, and cap max speed to 5.5 at least.
                game_physics::MotionProfile entityMotion = game_physics::DefaultMotionProfile();
                entityMotion.vAccelerate = va * 33.f;
                entityMotion.fMaxSpeed = _Max(distanceFromTarget, gameState->maxSpeed_mps);
                entityMotion.fMagnitude = 1.0f;
                va = calculateMotionProfile(gameState->cameraEntity.iPtSpeed, &entityMotion, dTinSec);

                // NOTE(Roman): clamp the movement to be between 0.15meters/frame to distance/frame
                // this will give us perfect landing, and a liniar slowdown.
                vec3 sign = game_math::sign(toTarget);
                va = game_math::hadamard(game_math::clamp(v3(0.005f), game_math::absolute(va), game_math::absolute(toTarget)), sign);
                break;
            }
        case CameraState_EntityFollowBounded: 
            if (entity) {
                vec3 boundsHalfDim = (gameState->regionUnitDimentions-gameState->viewUnitDimention) * 0.5;
                boundsHalfDim.y = gameState->regionUnitDimentions.y; // not used since we transition on entity from center to center
                GfxBox bounds = game_rect::createGfxRect(v3(0.2f, 0.0f,0.0f), boundsHalfDim);
                game_rect::expand(bounds, 1.2f);
                //
                coord_system::RegionCoord target = entity->iWorldPosition;
                target.offset.y = 0.0f;

                if (!(target.regionCoord == gameState->cameraEntity.iWorldPosition.regionCoord)) {
                    changeState(gameState, control_state_run_mapTransition);
                }

                target.offset = game_math::clamp(bounds.minPoint, target.offset, bounds.maxPoint);
                vec3 toTarget = coord_system::getDistance(target, gameState->cameraEntity.iWorldPosition, gameState->regionUnitDimentions);

                real32 distanceFromTarget = game_math::lengthSq(toTarget);
                if (distanceFromTarget <= game_math::epsilon && game_math::isZero(gameState->cameraEntity.iWorldPosition.offset.y)) {
                    changeState(gameState, control_state_run);
                    return;
                }

                // NOTE(Roman): Speed camera correction, if distance is greater then a full area teleport camera to the nearest area adjacent to target
                if (distanceFromTarget > game_math::lengthSq(gameState->regionUnitDimentions)) {
                    va = toTarget * (1.0f - (game_math::lengthSq(gameState->regionUnitDimentions) / distanceFromTarget));
                    break;
                }

                if (game_math::isZero(toTarget.x)) {
                    va.x = 0.0f;
                } else {
                    va.x = toTarget.x;
                }
                if (game_math::isZero(toTarget.y)) {
                    va.y = 0.0f;
                } else {
                    va.y = toTarget.y;
                }
                if (game_math::isZero(toTarget.z)) {
                    va.z = 0.0f;
                } else {
                    va.z = toTarget.z;
                }

                // NOTE(Roman): accelerate the camera reletively to the target, and cap max speed to 5.5 at least.
                game_physics::MotionProfile entityMotion = game_physics::DefaultMotionProfile();
                entityMotion.vAccelerate = va * 33.f;
                entityMotion.fMaxSpeed = _Max(distanceFromTarget * 1.5f, gameState->maxSpeed_mps);
                entityMotion.fMaxSpeed = _Min(entityMotion.fMaxSpeed, 60.f);
                entityMotion.fMagnitude = 1.0f;
                va = calculateMotionProfile(gameState->cameraEntity.iPtSpeed, &entityMotion, dTinSec);

                // NOTE(Roman): clamp the movement to be between 0.15meters/frame to distance/frame
                // this will give us perfect landing, and a liniar slowdown.
                vec3 sign = game_math::sign(toTarget);
                va = game_math::hadamard(game_math::clamp(v3(0.005f), game_math::absolute(va), game_math::absolute(toTarget)), sign);
                break;
            }
        case CameraState_AreaCenter: {
            break;
        }
        case CameraState_FreeRoam: {
            if (game_math::isNotZero(gameState->inputData.motionMagnitude)) {
                va = gameState->inputData.Delta * gameState->inputData.motionMagnitude * gameState->maxSpeed_mps;
            }
            va *= dTinSec;
            break;
        }
        default:
            break;
    }

    coord_system::offsetRegionCoord(va, gameState->regionUnitDimentions, &gameState->cameraEntity.iWorldPosition);
}

/////////////////////////////////////////////////////////////////////////////////////////
#define MAX_ENTITY_SIZE 3.0f
void
BeginRegionSimulation(GameState * gameState, UpdateEntityList* simEntityList, const GfxBox& basis) {
    // calculate the sim region inclusion cube (GfxBox)
    // simRegionBound - expansion dimentions to include max object size (capped!) and max distance/frame
    real32 maxSize = MAX_ENTITY_SIZE + gameState->maxSpeed_mps / 30.f;
    vec3 simRegionBound = gameState->viewUnitDimention * 0.5f;
    simRegionBound *= maxSize;

    GfxBox inclusion = game_rect::expandTo(basis, simRegionBound);

    const coord_system::RegionCoord& viewCenter = gameState->cameraEntity.iWorldPosition;
    coord_system::RegionCoord minC = gameState->cameraEntity.iWorldPosition, maxC = gameState->cameraEntity.iWorldPosition;
    coord_system::offsetRegionCoord(-simRegionBound, gameState->regionUnitDimentions, &minC);
    coord_system::offsetRegionCoord(simRegionBound, gameState->regionUnitDimentions, &maxC);
    --minC.regionCoord.y;
    maxC.regionCoord.y = game_math::clamp(0, maxC.regionCoord.y, gameState->cameraEntity.iWorldPosition.regionCoord.y + 1);
    minC.regionCoord.y = game_math::clamp(0, minC.regionCoord.y, gameState->cameraEntity.iWorldPosition.regionCoord.y + 1);

    //
    // NOTE(Roman): This is an O(n^5) but seriously in practice, not that much!
    for (int32 region_y = minC.regionCoord.y; region_y <= maxC.regionCoord.y; ++region_y) {
        coord_system::RegionCoord layer = coord_system::defaultRegionCoord(v3i(0, region_y, 0));
        vec3 layerDistance = coord_system::getDistance(layer, viewCenter, gameState->regionUnitDimentions);
        real32 perspectiveScale = _Max(1.0f, game_math::safeRatio1(-layerDistance.y, gameState->cameraEntity.focalLength));

        for (int32 region_z = minC.regionCoord.z; region_z <= maxC.regionCoord.z; ++region_z) {
            for (int32 region_x = minC.regionCoord.x; region_x <= maxC.regionCoord.x; ++region_x) {
                game_map_structs::WorldArea* worldRegion = NULL;
                if (gameState->gameWorld.getRegion(v3i(region_x, region_y, region_z), worldRegion)) {
                    // TODO(Roman): gather all entities in region that clip to inclusion.
                    game_map_structs::WorldArea::entity_reference_iterator it = rc_list::getIterator(worldRegion->m_LocalEntityList);
                    game_map_structs::WorldArea::entity_reference_list::__type reference = {};
                    while (it.has()) {
                        reference = *it.selected();

                        SimUpdateEntity ent = {};
                        ent.entityRef = reference;
                        ent.iPtSpeed = reference.ref->iPtSpeed;
                        ent.renderOffset = coord_system::getDistance(reference.ref->iWorldPosition, viewCenter, gameState->regionUnitDimentions);

                        ent.base = reference.ref->gameEntity->getPieces();
                        ent.entityType = reference.ref->gameEntity->getInfo().entityType;

                        vec3 distance = ent.renderOffset;
                        EntityPiece* e = ent.base;
                        while (e) {
                            //
                            distance += e->offset;
                            coord_system::offsetDistanceByBounds(e->iEntityBounds, distance);
                            if (game_rect::isPointInRect(distance, inclusion)) {
                                rc_list::remove(worldRegion->m_LocalEntityList, it);
                                rc_list::push(*simEntityList, ent, &gameState->scratchUpdateArena);
                                break;
                            }
                            e = e->next;
                        }
                        it.next();
                    }
                }
#ifdef RC_TESTCODE
                worldRegion = NULL;
#endif
            }
        }
    }
}

void
EndRegionSimulation(GameState * gameState, UpdateEntityList* simEntityList) {
    rc_list::iterator<SimUpdateEntity> it = rc_list::getIterator(*simEntityList);
    SimUpdateEntity* ent = NULL;

    // TODO(Roman): gather all entities in region that clip to inclusion.
    while (ent = it++) {
        coord_system::RegionCoord viewCenter = gameState->cameraEntity.iWorldPosition;
        //ent->renderOffset.y += ent->groundOffset;
        coord_system::offsetRegionCoord(ent->renderOffset, gameState->regionUnitDimentions, &viewCenter);
        ent->entityRef.ref->iPtSpeed = ent->iPtSpeed;
        ent->entityRef.ref->iWorldPosition = viewCenter;

        game_map_structs::WorldArea* worldRegion = NULL;
        if (!gameState->gameWorld.getRegion(viewCenter.regionCoord, worldRegion)) {
            gameState->gameWorld.addRegion(viewCenter.regionCoord);
            gameState->gameWorld.getRegion(viewCenter.regionCoord, worldRegion);
        }
        if (worldRegion) {
            rc_list::push(worldRegion->m_LocalEntityList, ent->entityRef, &gameState->entityArena);
        }
    }
}

