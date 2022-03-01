/* ========================================================================
$File: $
$Date: $
$Revision: $
$Creator: Roman Markovetski $
$Notice: (C) Copyright 2014 by Farmland Blossoms. All Rights Reserved. $
======================================================================== */

// --
#include "../RaidCore/Headers/RaidCore_GameWorld.h"
#include "../RaidCore/Headers/RaidCore_GameState.h"
#include "../RaidCore/Headers/RaidCore_Math.h"

namespace game_map_structs {
    void GameWorld::addRegion(vec3i region) {
        WorldArea mapRegion = {};
        mapRegion.region = region;
        hash_map::addAsc(&regionHash, mapRegion, regionHashKey(&mapRegion));
    }
    void GameWorld::removeRegion(vec3i region) {
        WorldArea mapRegion = {};
        mapRegion.region = region;
        hash_map::remove(&regionHash, regionHashKey(&mapRegion));
    }
    bool32 GameWorld::getRegion(vec3i region, WorldArea*& result) {
        WorldArea key = {};
        key.region = region;
        if (hash_map::get(&regionHash, regionHashKey(&key), result)) {
            return (true32);
        }
        return (false32);
    }

    void GameWorld::addEntity(vec3i region, Entity* entity) {
        hash_map::addAsc(&entityHash, EntityPieceReference{ entity }, entity->getId().value);
    }
    void GameWorld::removeEntity(uint32 stored_id) {
        hash_map::remove(&entityHash, stored_id);
    }
    bool32 GameWorld::getEntity(uint32 stored_id, Entity *& result) {
        EntityPieceReference *ref = NULL;
        if (hash_map::get(&entityHash, stored_id, ref)) {
            result = ref->ptr;
            return (true32);
        }
        return (false32);
    }
    bool32 GameWorld::getEntitiesInArea(const GfxBox& area, rc_list::list<Entity>& result, game_memory::arena_p memory) {
        for (uint32 bi = 0; bi < entityHash._blockCount; ++bi) {
            hash_map::block<EntityPieceReference>* block = entityHash._hashBlocks[bi];
            while (NULL != block) {

                // TODO(Roman):add only if entity is in area.

                rc_list::push(result, *block->_data.ptr, memory);
                block = block->_next;
            }
        }
        return (rc_list::hasValues(result));
    }
}
