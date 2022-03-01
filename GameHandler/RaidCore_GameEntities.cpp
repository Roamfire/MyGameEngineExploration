/* ========================================================================
$File: $
$Date: $
$Revision: $
$Creator: Roman Markovetski $
$Notice: (C) Copyright 2014 by Farmland Blossoms. All Rights Reserved. $
======================================================================== */

// GameHandler.cpp : Defines the exported functions for the DLL application.
//

// --
#include "../RaidCore/Headers/RaidCore_MemManager.h"
#include "../RaidCore/Headers/RaidCore_GameWorld.h"
#include "../RaidCore/Headers/RaidCore_GameState.h"

#include "../RaidCore/Headers/RaidCore_Platform.h"
#include "../RaidCore/Headers/RaidCore_MathOperators.h"
#include "../RaidCore/Headers/RaidCore_UserInputHdr.h"
#include "../RaidCore/Headers/RaidCore_Physics.h"
#include "../RaidCore/Headers/RaidCore_WorldCoordinates.h"
#include "../RaidCore/Headers/RaidCore_UserInputHdr.h"
#include "../GameHandler/GameHandler_Management.h"

#include "../GameHandler/RaidCore_GameEntities.h"
#include "../RaidCore/Headers/RaidCore_RandomMath.h"

bool32 Entity::operator==(const Entity& e2) {
    return (iStoredId.value == e2.iStoredId.value);
}
bool32 Entity::operator<(const Entity& e2) {
    return (iStoredId.value < e2.iStoredId.value);
}
bool32 Entity::isEntityState(EntityPiece* e, uint32 state) {
    return ((e->iState & state) == state);
}
//void Entity::setEntityState(uint32* entityState, uint32 state) {
//    *entityState |= state;
//}
//void Entity::resetEntityState(uint32* entityState, uint32 state) {
//    *entityState &= (~state);
//}
void Entity::setEntityState(uint32 state) {
    EntityPiece* e = base;
    while (e) {
        e->iState |= state;
        e = e->next;
    }
}
 void Entity::resetEntityState(uint32 state) {
    EntityPiece* e = base;
    while (e) {
        e->iState &= (~state);
        e = e->next;
    }
}

 EntityPiece* Entity::getPieces() {
     return base;
 }
 void Entity::setPieces(EntityPiece* e, uint32 count) {
     pieceCount = count;
     base = e;
 }

 void Entity::init(entity_id id, game_entity_type type) {
     iStoredId = id;
     info.entityType = type;
 }


rc_macro_cpp bool32 operator ==(const EntityPieceReference& A, const EntityPieceReference& B) {
    bool32 result = ((A.ptr == B.ptr) &&
                     (A.id.value == B.id.value));
    return (result);
}
rc_macro_cpp bool32 operator ==(const WorldAreaEntity& A, const WorldAreaEntity& B) {
    bool32 result = (((Entity*)A.gameEntity)->getId().value == ((Entity*)B.gameEntity)->getId().value);
    return (result);
}
rc_macro_cpp bool32 operator !=(const SimUpdateEntity& A, const SimUpdateEntity& B) {
    bool32 result = (((Entity*)A.entityRef.ref->gameEntity)->getId().value != ((Entity*)B.entityRef.ref->gameEntity)->getId().value);
    return (result);
}
rc_macro_cpp bool32 operator ==(const TEntityReference& A, const TEntityReference& B) {
    bool32 result = ((A.ref == B.ref) &&
                     (A.index == B.index));
    return (result);
}

//------------------------------------------------------------------------------------------------------
vec3 calculateMotionProfile(vec3& iPtSpeed, game_physics::MotionProfile * motion, real32 dTinSec) {
    vec3 result = game_physics::getGenericMotionOffset(iPtSpeed, motion, game_physics::ResistanceStopDrag, dTinSec);
    return (result);
}

// a.k.a Gravity (if pull == v3(0,9.81,0))
rc_macro_cpp vec3 calculateAcceleratingForce(GameState* gameState, vec3& iPtSpeed, const vec3& pull, real32 dTinSec) {
    game_physics::MotionProfile m = game_physics::DefaultMotionProfile();
    // apply gravity
    m.vAccelerate = pull;
    m.fMagnitude = 1.0f;
    m.fMaxSpeed = 30.f;
    vec3 result = calculateMotionProfile(iPtSpeed, &m, dTinSec);
    return (result);
}

rc_macro_cpp vec3 getInputMoveOffset(GameState* gameState, vec3& iPtSpeed, real32 dTinSec) {
    vec3 result = {};
    game_physics::MotionProfile m = game_physics::DefaultMotionProfile();
    m.vAccelerate = gameState->inputData.motionDirection;
    m.fMagnitude = gameState->inputData.motionMagnitude;
    if (game_math::isNotZero(m.fMagnitude) || game_math::isNotZero(game_math::length(iPtSpeed))) {
        result = calculateMotionProfile(iPtSpeed, &m, dTinSec);
    }
    return (result);
}

namespace game_collision {
    internal void updateSlopeMotion(EntityPiece* targetEntity, const vec3& targetEntityOffset, real32 rayPart, const vec3& fFacingDirection, GfxBox collisionTest, const vec3& orgOffset, vec3& move) {

        GfxRect box = game_rect::createGfxRect(v2(targetEntityOffset.x, targetEntityOffset.z), v2(targetEntity->iEntityBounds.radialDimention.x, targetEntity->iEntityBounds.radialDimention.z));
        vec2 offset2d = v2(move.x, move.z) * rayPart; // unhindered move portion
        vec2 step2d = v2(move.x, move.z) * (1.0f - rayPart); // move up or down slope
        vec2 pos2d = v2(orgOffset.x, orgOffset.z) + offset2d;

        if (game_math::isNotZero(rayPart)) {
            // move into stairs
            // use rest of move to update vector
        }

        if (game_rect::isPointInRect(pos2d, box)) {
            vec2 b = game_math::hadamard(
                v2(fFacingDirection.x, fFacingDirection.z),
                game_math::clamp01(game_math::barycentric(box, pos2d))
                );
            real32 k = (fFacingDirection.x < 0.0f || fFacingDirection.z < 0.0f) ? (b.x + b.y) : (1.0f - (b.x + b.y));
            real32 yOffset = k * targetEntity->iEntityBounds.radialDimention.y * 2.f;

            collisionTest.maxPoint.y = collisionTest.minPoint.y + yOffset;
            vec3 normal = {};
            if (game_rect::getCollision(collisionTest, orgOffset, move, rayPart, normal)) {
            }
        }
    }

    bool32 verifyEntityCollision(SimUpdateEntity* simEntity, SimUpdateEntity* testSimEntity, const vec3& origin, const vec3& move) {
        vec3 testPoint = testSimEntity->renderOffset;
        EntityPiece* testE = ((Entity*)testSimEntity->entityRef.ref->gameEntity)->getPieces();
        while (testE) {
            testPoint += testE->offset;
            if (Entity::isEntityState(testE, EntityState_Active | EntityState_Collides)) {
                EntityPiece* e = ((Entity*)simEntity->entityRef.ref->gameEntity)->getPieces();
                vec3 eOffset = origin;
                while (e) {
                    eOffset += e->offset;
                    vec3 testOffset = testPoint + testE->iEntityBounds.offset;
                    if (Entity::isEntityState(e, EntityState_Active | EntityState_Collides)) {
                        GfxBox collisionTest = game_rect::createGfxRect(testOffset, testE->iEntityBounds.radialDimention + e->iEntityBounds.radialDimention);
                        if (game_rect::testCollision(collisionTest, eOffset + e->iEntityBounds.offset, move)) {
                            return true32;
                        }
                    }
                    e = e->next;
                }

            }
            testE = testE->next;
        }
        return (false32);
    }

    EntityPiece* checkEntityCollision(SimUpdateEntity* simEntity, SimUpdateEntity* testSimEntity, const vec3& origin, vec3& move, real32& minDistanceMultiplier, vec3& counterVec) {
        EntityPiece* result = NULL;
        // follow the vector 'move' from 'origin' and process collisions
        vec3 testPoint = testSimEntity->renderOffset;
        EntityPiece* testE = ((Entity*)testSimEntity->entityRef.ref->gameEntity)->getPieces();
        while (testE) {
            testPoint += testE->offset;
            if (Entity::isEntityState(testE, EntityState_Active | EntityState_Collides)) {
                vec3 testOffset = testPoint + testE->iEntityBounds.offset;
                EntityPiece* e = ((Entity*)simEntity->entityRef.ref->gameEntity)->getPieces();
                vec3 eOffset = origin;
                while (e) {
                    eOffset += e->offset;
                    vec3 orgOffset = eOffset + e->iEntityBounds.offset;
                    if (Entity::isEntityState(e, EntityState_Active | EntityState_Collides)) {
                        GfxBox collisionTest = game_rect::createGfxRect(testOffset, testE->iEntityBounds.radialDimention);
                        real32 rayPart = 0.0f;
                        vec3 normal = {};
                        if (Entity::isEntityState(testE, EntityState_Slope)) {
                            if (game_rect::getCollision(collisionTest, orgOffset, move, rayPart, normal)) {
                                if (game_math::isZero(rayPart)) {
                                    // NOTE(Roman): we are inside the object...
                                    // Correct vector and move on, do not let this collision impact next test
                                    updateSlopeMotion(testE, testOffset, 0.0f, testSimEntity->entityRef.ref->fFacingDirection, collisionTest, orgOffset, move);
                                    // NOTE(Roman): after updateSlopeMotion corrected the movement vector, have the move tested against all other entity pieces in the stair entity
                                    break;
                                } else if (rayPart < minDistanceMultiplier) {
                                    // NOTE(Roman): correcting vector for collision with sloped entities,
                                    // inside the slope area, the distance to collision should be modified and the collision recalculated
                                    vec3 subMove = move * (1.0f - rayPart);
                                    updateSlopeMotion(testE, testOffset, rayPart, testSimEntity->entityRef.ref->fFacingDirection, collisionTest, orgOffset + move * rayPart, subMove);
                                    move = move * rayPart + subMove;
                                    break;
                                }
                            }
                        } else {
                            game_rect::expand(collisionTest, e->iEntityBounds.radialDimention);
                            if (game_rect::getCollision(collisionTest, orgOffset, move, rayPart, normal)) {
                                if (rayPart < minDistanceMultiplier) {
                                    counterVec = game_math::set0or1(normal);
                                    minDistanceMultiplier = rayPart;
                                    result = testE;
                                }
                            }
                        }
                    }
                    e = e->next;
                }
            }
            testE = testE->next;
        }
        return (result);
    }
    struct CollisionPlane {
        real32 collisionPlaneD; // where the wall edge is (min/max on any given axis)
        real32 RelX; // distance on X between collision volume centers of entity and tested target
        real32 RelY; // distance on Y between collision volume centers of entity and tested target
        real32 RelZ; // distance on Z between collision volume centers of entity and tested target
        real32 DeltaX; // move distance on X
        real32 DeltaY; // move distance on Y
        real32 DeltaZ; // move distance on Z
        real32 MinY; // collision plane min
        real32 MaxY; // collision plane max
        real32 MinZ; // collision plane min
        real32 MaxZ; // collision plane max
        vec3 Normal; // plane normal
    };
    internal bool32 testCollisionVsPlane(const CollisionPlane* entityMinkowskiPlane, real32 *tMin) {
        bool32 Hit = false;
        if (game_math::isNotZero(entityMinkowskiPlane->DeltaX)) {
            real32 tResult = (entityMinkowskiPlane->collisionPlaneD - entityMinkowskiPlane->RelX) / entityMinkowskiPlane->DeltaX;
            real32 Y = entityMinkowskiPlane->RelY + tResult*entityMinkowskiPlane->DeltaY;
            real32 Z = entityMinkowskiPlane->RelZ + tResult*entityMinkowskiPlane->DeltaZ;
            if ((tResult >= 0.0f) && (*tMin > tResult)) {
                if ((Y >= entityMinkowskiPlane->MinY) && (Y <= entityMinkowskiPlane->MaxY) && (Z >= entityMinkowskiPlane->MinZ) && (Z <= entityMinkowskiPlane->MaxZ)) {
                    *tMin = game_math::maxValue(0.0f, tResult - game_math::minOffset);
                    Hit = true;
                }
            }
        }
        return(Hit);
    }

    EntityPiece* testEntityCollision(SimUpdateEntity* simEntity, SimUpdateEntity* testSimEntity, const vec3& origin, vec3& move, real32& minDistanceMultiplier, vec3& counterVec) {
        EntityPiece* result = NULL;
        bool32 HitThis = false32;
        real32 tMin = minDistanceMultiplier;
        vec3 planeNormal = {};
        // follow the vector 'move' from 'origin' and process collisions
        vec3 testPoint = testSimEntity->renderOffset;
        EntityPiece* testE = ((Entity*)testSimEntity->entityRef.ref->gameEntity)->getPieces();
        while (testE) {
            testPoint += testE->offset;
            if (Entity::isEntityState(testE, EntityState_Active | EntityState_Collides)) {
                vec3 testOffset = testPoint + testE->iEntityBounds.offset;
                EntityPiece* e = ((Entity*)simEntity->entityRef.ref->gameEntity)->getPieces();
                vec3 eOffset = origin;
                while (e) {
                    if (Entity::isEntityState(e, EntityState_Active | EntityState_Collides)) {
                        eOffset += e->offset;
                        GfxBox collisionTest = game_rect::createGfxRect(testOffset, testE->iEntityBounds.radialDimention + e->iEntityBounds.radialDimention);
                        vec3 orgOffset = eOffset + e->iEntityBounds.offset;
                        real32 rayPart = 0.0f;

                        CollisionPlane minkowskiPlanes[] = {
                            { collisionTest.minPoint.x, orgOffset.x, orgOffset.y, orgOffset.z, move.x, move.y, move.z, collisionTest.minPoint.y, collisionTest.maxPoint.y, collisionTest.minPoint.z, collisionTest.maxPoint.z, v3(-1.f, 0.f, 0.f) },
                            { collisionTest.maxPoint.x, orgOffset.x, orgOffset.y, orgOffset.z, move.x, move.y, move.z, collisionTest.minPoint.y, collisionTest.maxPoint.y, collisionTest.minPoint.z, collisionTest.maxPoint.z, v3(1.f, 0.f, 0.f) },
                            { collisionTest.minPoint.y, orgOffset.y, orgOffset.x, orgOffset.z, move.y, move.x, move.z, collisionTest.minPoint.x, collisionTest.maxPoint.x, collisionTest.minPoint.z, collisionTest.maxPoint.z, v3(0.f, -1.f, 0.f) },
                            { collisionTest.maxPoint.y, orgOffset.y, orgOffset.x, orgOffset.z, move.y, move.x, move.z, collisionTest.minPoint.x, collisionTest.maxPoint.x, collisionTest.minPoint.z, collisionTest.maxPoint.z, v3(0.f, 1.f, 0.f) },
                            { collisionTest.minPoint.z, orgOffset.z, orgOffset.x, orgOffset.y, move.z, move.x, move.y, collisionTest.minPoint.x, collisionTest.maxPoint.x, collisionTest.minPoint.y, collisionTest.maxPoint.y, v3(0.f, 0.f, -1.f) },
                            { collisionTest.maxPoint.z, orgOffset.z, orgOffset.x, orgOffset.y, move.z, move.x, move.y, collisionTest.minPoint.x, collisionTest.maxPoint.x, collisionTest.minPoint.y, collisionTest.maxPoint.y, v3(0.f, 0.f, 1.f) },
                        };
                        real32 tMinTest = tMin;
                        for (uint32 planeIndex = 0;
                             planeIndex < ArrayCount(minkowskiPlanes);
                             ++planeIndex) {
                            CollisionPlane *plane = minkowskiPlanes + planeIndex;
                            if (testCollisionVsPlane(plane, &tMin)) {
                                HitThis = true32;
                                planeNormal = plane->Normal;
                                result = testE;
                            }
                        }
                    }
                    e = e->next;
                }
            }
            testE = testE->next;
        }
        if (HitThis) {
            minDistanceMultiplier = tMin;
            counterVec = planeNormal;
        }
        return (result);
    }
}

//------------------------------------------------------------------------------------------------------
internal uint32 MakeStoreId(uint32 type, uint32 * index) {
    return (((type << 24) & 0x7fff0000) | ((*index)++));
}

