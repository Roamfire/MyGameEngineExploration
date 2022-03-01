/* ========================================================================
$File: $
$Date: $
$Revision: $
$Creator: Roman Markovetski $
$Notice: (C) Copyright 2014 by Farmland Blossoms. All Rights Reserved. $
======================================================================== */

#ifndef __RC_X_GAME_WORLD_H_
#include "RaidCore_Common.h"
#include "RaidCore_Math.h"
#include "RaidCore_Entity.h"
#include "RaidCore_UtilsHeader.h"

namespace game_map_structs
{
	struct WorldArea
	{
		//
        vec3i
            region;
        //
        GfxBox
            regionBounds;   // test camera viewport for collision with these (prevent scrolling over empty zones)
                            // TODO(Roman): camera colliding may stop or teleport to the next region is such is available...
                            //   Camera collision happens only if there is no region adjacent in the direction of movement.
		//
		// List of entities in this map space
        typedef rc_list::list<TEntityReference> entity_reference_list;
        typedef rc_list::iterator<entity_reference_list::__type> entity_reference_iterator;
        entity_reference_list
			m_LocalEntityList;
	};
    rc_macro_force intptr regionHashKey(WorldArea* r) {
        intptr result = r->region.z & 0x0fff;
        result <<= 40;
        result += (((r->region.y & 0xff) << 16) + (r->region.x & 0x0fff));
        return (result);
    }
    inline intptr regionHashKey(intptr k)
	{
        return (((k>>40)&0xfff) + ((k>>16)&0xff)+ (k&0x0fff));
	}

	struct GameWorld
	{
        hash_map::map<WorldArea>
            regionHash;
        hash_map::map<EntityPieceReference>
            entityHash;
        uint32
            lastEntiyId;

		///////////////////////////////////////////////////
		// World states
		// 1. max dimentions of plane(s) and tile maps
		// 2. pixel dimention of a tile (not to be confused with the actual bitmap dimentions, those may differ wildly)
		// 3. game world position, reletive to game world center of coordinates
		uint32
			worldMapLayerCount_X,
			worldMapLayerCount_Y,
			worldTileSpaceMaxIndex_X,
			worldTileSpaceMaxIndex_Y;
		GameWorld()
		{
			worldMapLayerCount_X = 0;
			worldMapLayerCount_Y = 0;
			worldTileSpaceMaxIndex_X = 0;
			worldTileSpaceMaxIndex_Y = 0;
            //
            regionHash = {};
            entityHash = {};
            lastEntiyId = 0;
		}
        void addRegion(vec3i region);
        void removeRegion(vec3i region);
        bool32 getRegion(vec3i region, WorldArea*& result);

        void addEntity(vec3i region, Entity* entity);
        void removeEntity(uint32 stored_id);
        bool32 getEntity(uint32 stored_id, Entity*& result);
        bool32 getEntitiesInArea(const GfxBox& area, rc_list::list<Entity>& result, game_memory::arena_p memory);
	};
}

rc_macro_cpp bool32 operator ==(game_map_structs::WorldArea& left, game_map_structs::WorldArea& right) {
    return (left.region == right.region);
}

#define __RC_X_GAME_WORLD_H_
#endif//__RC_X_GAME_WORLD_H_
