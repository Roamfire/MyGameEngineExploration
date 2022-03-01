/* ========================================================================
$File: $
$Date: $
$Revision: $
$Creator: Roman Markovetski $
$Notice: (C) Copyright 2014 by Farmland Blossoms. All Rights Reserved. $
======================================================================== */

// --
#include "../GameHandler/GameHandler.h"

#include "../RaidCore/Headers/RaidCore_MemManager.h"
#include "../RaidCore/Headers/RaidCore_GameWorld.h"
#include "../RaidCore/Headers/RaidCore_GameState.h"
#include "../RaidCore/Headers/RaidCore_MathOperators.h"
#include "../RaidCore/Headers/RaidCore_Physics.h"
#include "../RaidCore/Headers/RaidCore_WorldCoordinates.h"

#include "../GameHandler/RaidCore_GameEntities.h"
#include "../RaidCore/RaidCore_Actor.h"

namespace game_play {
    namespace {
        void addHitPoints(Monster* mon, uint32 hp) {
            mon->hitPoints = game_math::clamp(0, mon->hitPoints + hp, mon->maxHitPoints);
        }
        void subtractHitPoints(Monster* mon, uint32 hp) {
            mon->hitPoints = game_math::clamp(0, mon->hitPoints - hp, mon->maxHitPoints);
            if (mon->hitPoints) {
                mon->setEntityState(NpcState_Hurt);
            } else {
                mon->resetEntityState(~NpcState_Dead);
            }
        }

        bool32 canCollide(SimUpdateEntity* simEntity, SimUpdateEntity* collideWith) {
            bool32 result = false32;
            if (collideWith != NULL) {
                collideWith->updateId.value = simEntity->entityRef.index;
                EntityPiece* e = collideWith->entityRef.ref->gameEntity->getPieces();
                while (e && !result) {
                    result = Entity::isEntityState(e, EntityState_Collides | EntityState_Active);
                    e = e->next;
                }
            }
            return (collideWith != NULL && result);
        }

        // NOTE(Roman): calculate the max move distance allowed per collision
        // must handle the collision result
        // segmentMove - how much of the move is used to reach the collision point
        // speedBump - the collision normal vector that nullifies or reflects the movement in the collision direction
        // minDistanceMultiplier - precentage of the move that created the segmentMove
        // move - input the original move offset, output the remainder of the move after collision and reflection.
        void handleCollisionVector(SimUpdateEntity* simEntity, vec3& segmentMove, vec3& speedBump, const real32 minDistanceMultiplier) {
            if (game_math::isNotZero(game_math::length(segmentMove))) {
                // don't move closer than the threshold, on the collision axis
                segmentMove -= game_math::hadamard(game_math::maxOf(game_math::sign(segmentMove) * game_math::minOffset, segmentMove), speedBump);
            }
            // moves that are too small are eliminated
            game_math::nullify(segmentMove);
        }
        real32 handleCollision(SimUpdateEntity* simEntity, SimUpdateEntity* collideWith, EntityPiece* collidePiece, vec3& segmentMove, vec3& speedBump, real32 minDistanceMultiplier, vec3& move) {
            vec3 origin = simEntity->renderOffset + segmentMove;
            // current speed on the colliding axis is nullified
            simEntity->iPtSpeed -= game_math::hadamard(simEntity->iPtSpeed, speedBump);
            speedBump.x = 1.0f - speedBump.x;
            speedBump.y = 1.0f - speedBump.y;
            speedBump.z = 1.0f - speedBump.z;
            move = game_math::hadamard((move * (1.0f - minDistanceMultiplier)), speedBump);

            return minDistanceMultiplier;
        }
        //------------------------------------------------------------------------------------------------------
        // NOTE(Roman): find the closest entity in the collision path, process collision and update the movement vector
        // return how much entity was moved in this collision iteration
        // movement vector is updated to any remaining movement, for next collision iteration
        void moveEntity(GameState* gameState, SimUpdateEntity* simEntity, vec3 moveDelta, UpdateEntityList& simList) {
            TemporaryMemory tMem = BeginTemporaryMemory(&gameState->scratchUpdateArena);
            UpdateEntityList collisionList = rc_list::initList<SimUpdateEntity>();

            // Where is the entity now.
            vec3 origin = simEntity->renderOffset;
            // collect all entities in collision path
            UpdateEntityListIterator itsCopy = rc_list::getIterator(simList);// 
            while (itsCopy.has()) {
                SimUpdateEntity* testSimEntity = itsCopy.selected();
                if (*simEntity != *testSimEntity && testSimEntity->updateId.value != simEntity->entityRef.index) {
                    if (game_collision::verifyEntityCollision(simEntity, testSimEntity, origin, moveDelta)) {
                        rc_list::push(collisionList, *testSimEntity, tMem.arena);
                    }
                }
                itsCopy.next();
            }
            // process all possible collisions
            vec3 speedBump;
            vec3 entityMove = {};
            real32 minDistanceMultiplier;
            // follow the vector 'moveDelta' from 'origin' and process collisions
            do {
                EntityPiece* collidePiece = NULL;
                SimUpdateEntity* collideWith = NULL;
                UpdateEntityListIterator itSelected;
                minDistanceMultiplier = 1.0f;
                UpdateEntityListIterator itSearch = rc_list::getIterator(collisionList);// 
                while (itSearch.has()) {
                    SimUpdateEntity* testSimEntity = itSearch.selected();
                    // TODO(Roman): handle multiple collision volumes
                    if (testSimEntity->updateId.value != simEntity->entityRef.index) {
                        EntityPiece* collided = game_collision::checkEntityCollision(simEntity, testSimEntity, origin, moveDelta, minDistanceMultiplier, speedBump);
                        if (collided) {
                            collidePiece = collided;
                            collideWith = testSimEntity;
                            itSelected = itSearch;
                        }
                    }
                    itSearch.next();
                }
                vec3 segmentMove = moveDelta * minDistanceMultiplier;
                game_math::nullify(segmentMove, game_math::minOffset); // keep an offset threshold
                if (canCollide(simEntity, collideWith)) {
                    //rc_list::remove(collisionList, itSelected);
                    // moveDelta the entity to collide with 'collideWith'
                    handleCollisionVector(simEntity, segmentMove, speedBump, minDistanceMultiplier);
                    // handle collision logic results and update the distance used (minDistanceMultiplier)
                    minDistanceMultiplier = handleCollision(simEntity, collideWith, collidePiece, segmentMove, speedBump, minDistanceMultiplier, moveDelta);
                    entityMove += segmentMove;
                    origin += segmentMove;
                } else {
                    entityMove += moveDelta;
                    origin += moveDelta;
                }
            } while (1.0f > minDistanceMultiplier);
            if (game_math::isNotZero(game_math::length(entityMove))) {
                simEntity->renderOffset += entityMove;
            }
            EndTemporaryMemory(tMem);
        }
    }

    //------------------------------------------------------------------------------------------------------
    void UpdateEntity(GameState* gameState, SimUpdateEntity* simEntity, vec3& iPtSpeed, UpdateEntityList& simList, real32 frameTime) {
        Assert(simEntity);
        Assert(simEntity->entityRef.ref);

        if (!simEntity->didUpdate) {
            // NOTE(Roman): decide what the entity is doing and its curent state
            // calculate its motion, and handle any collisions
            switch (((Entity*)simEntity->entityRef.ref->gameEntity)->getInfo().entityType) {
                case EntityType_Player: {
                    Hero* hero = (Hero*)simEntity->entityRef.ref->gameEntity;
                    // NOTE(Roman): get facing direction as a unit vector,
                    //              facing is in the movement direction (by design)
                    if (game_math::isNotZero(gameState->inputData.motionMagnitude)) {
                        simEntity->entityRef.ref->fFacingDirection = gameState->inputData.Delta * gameState->inputData.motionMagnitude;
                    }
                    vec3 controlSpeed = v3(iPtSpeed.x, 0.0f, iPtSpeed.z);;
                    vec3 gravitySpeed = v3(0.0f, iPtSpeed.y, 0.0f);
                    vec3 moveDelta = getInputMoveOffset(gameState, controlSpeed, frameTime);
                    moveDelta += calculateAcceleratingForce(gameState, gravitySpeed, v3(0.0f, game_physics::EarthGravityAcceleration, 0.0f), frameTime);
                    iPtSpeed = controlSpeed + gravitySpeed;
                    if (game_math::isNotZero(game_math::lengthSq(moveDelta))) {
                        moveEntity(gameState, simEntity, moveDelta, simList);
                    }
                    break;
                }
                case EntityType_Wpn_Sword:
                    break;
                case EntityType_Wpn_Gun:
                    // spawn projectiles, duh!
                    break;
                case EntityType_Wpn_Projectile:
                    break;
                case EntityType_Wpn_Chain:
                    // extends from controlling entity to a certain distance (flame-thrower's flame)
                    break;
                case EntityType_Wpn_Boomerang:
                    break;
                case EntityType_NPC:
                    break;
                case EntityType_Tree:
                    // NOTE(Roman): do we add animation???
                    // tree can easily be a shrub, a patch of grass, etc.
                    break;
                case EntityType_Structure:
                    // NOTE(Roman): structures are things that can be destroyed
                    // they can be fences, houses, boulders, and any other static element that does not animate, but can react to collision
                case EntityType_Environment:
                    // NOTE(Roman): environment are things that act as a visual elemnt that can overlap entities, with no collision
                    // they can also animate.
                case EntityType_None:
                    // NOTE(Roman): these are completely static
                    // NOTE(Roman): Do we need this? what is the difference with EntityType_Environment???
                default:
                    break;
            }
            simEntity->didUpdate = true32;
        }
    }
}