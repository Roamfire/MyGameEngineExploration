/* ========================================================================
$File: $
$Date: $
$Revision: $
$Creator: Roman Markovetski $
$Notice: (C) Copyright 2014 by Farmland Blossoms. All Rights Reserved. $
======================================================================== */

#ifndef __RC_X_ENTITY_H_
#include "RaidCore_Common.h"
#include "RaidCore_Math.h"
#include "RaidCore_WorldCoordinates.h"
#include "RaidCore_MemManager.h"
#include "RaidCore_Physics.h"

//
enum game_entity_type {
    EntityType_None,
    EntityType_Shadow,
    EntityType_Player,
    EntityType_Wpn_Sword,
    EntityType_Wpn_Gun,
    EntityType_Wpn_Projectile,
    EntityType_Wpn_Chain, // extends from controlling entity to a certain distance (flame-thrower flame)
    EntityType_Wpn_Boomerang,
    EntityType_NPC,
    EntityType_Structure,
    EntityType_Environment,
    EntityType_Tree,
    EntityType_Stair,
};
//
// Define the position and state of the entity
enum game_entity_state
{
    EntityState_none = 0, // a visual ghost, no interaction at all (particles?)
    EntityState_Collides = (1 << 1),// tests for collision with others
    EntityState_Active = (1 << 2),// handle collision only if this is available
    EntityState_Grounded = (1 << 3),// this entity must be rendered on a ground!!! ex. shadow
    EntityState_Slope = (1 << 4),//
};
//
struct entity_id {
    uint32 value;
};
// The EntityPiece is a unique entity, per type.
//  it contains the references to its display and behavior resources
struct EntityPiece
{
	GfxVolume
		iEntityBounds; // collision box
	uint32
		iEntityAsset;
    uint32
		iState;
    // NOTE(Roman): animation...
    real32 maxTime;
    real32 animTime;
	//
	vec3 offset; // relative offset from previous entity center
	EntityPiece* next; // next entity piece
};
struct EntityHeader {
    game_entity_type entityType;
};
class Entity {
private:
    EntityHeader info;
    entity_id
		iStoredId;  // index in the game entity list
    //
    //
    uint32 pieceCount;
	EntityPiece* base; // base entity piece
public:
    bool32 operator==(const Entity& e2);
    bool32 operator<(const Entity& e2);
    static bool32 isEntityState(EntityPiece* e, uint32 state);
    void setEntityState(uint32 state);
    void resetEntityState(uint32 state);

    uint32 getPieceCount() const { return pieceCount; }
    EntityPiece* getPieces();
    void setPieces(EntityPiece*, uint32);

    void init(entity_id, game_entity_type);
    
    EntityHeader getInfo() const { return info; }
    entity_id getId() const { return iStoredId; }
};

struct EntityPieceReference {
    Entity *ptr;
    entity_id id;
};

//
// NOTE(Roman): each region hosts a list of all entities in it.
struct WorldAreaEntity {
    Entity*
		gameEntity; // This is the entity list we update and render

    vec3
        iPtSpeed;	// Current velocity of the entity, independant of facing direction
    coord_system::RegionCoord
        iWorldPosition;		// The absolute world map position, this is used to locate the correct map region we need for the rendering and locating
    vec3
        fFacingDirection;  // direction 
};
struct TEntityReference {
    WorldAreaEntity* ref;
    uint32 index;
};

rc_macro_h bool32 operator ==(const EntityPieceReference& A, const EntityPieceReference& B);
rc_macro_h bool32 operator ==(const WorldAreaEntity& A, const WorldAreaEntity& B);
rc_macro_h bool32 operator ==(const TEntityReference& A, const TEntityReference& B);


#define __RC_X_ENTITY_H_
#endif//__RC_X_ENTITY_H_
