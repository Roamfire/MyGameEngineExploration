/* ========================================================================
$File: $
$Date: $
$Revision: $
$Creator: Roman Markovetski $
$Notice: (C) Copyright 2014 by Farmland Blossoms. All Rights Reserved. $
======================================================================== */

void strset(char* text, uint32 len, const char* format, ...) {
//    va_start()
    _snprintf_s(text, (size_t)len, (size_t)len,
            format);
    //OutputDebugStringA(text);
}


void _AddHalfWall(GameState * gameState, game_map_structs::WorldArea* selectedRgn, const vec3& facing, const vec3& offset) {
    if (!gameState) {
        return;
    }

    WorldAreaEntity* areaEntity = PushStruct(&gameState->entityArena, WorldAreaEntity, game_memory::NoAlign());
    *areaEntity = {};
    // setup initialized entity parameters
    areaEntity->fFacingDirection = facing;
    areaEntity->iWorldPosition.regionCoord = selectedRgn->region;
    areaEntity->iWorldPosition.offset = offset;
    // setup te entity itself, 

    game_render_engine::addRenderAsset(gameState, dAssetType_Tree);
    EntityPieceReference* ref;
    uint32 sid = MakeStoreId(EntityType_Tree, &gameState->gameWorld.lastEntiyId);
    if (!hash_map::get(&gameState->gameWorld.entityHash, sid, ref)) {
        Entity* ent = PushStruct(&gameState->entityArena, Entity);
        *ent = {};
        ent->init(entity_id{ sid }, EntityType_Structure);
        ent->setPieces(PushArray(&gameState->entityArena, 1, EntityPiece), 1);

        areaEntity->gameEntity = ent;

        EntityPieceReference tmpRef = {};
        ref = &tmpRef;
        ref->ptr = ent;

        hash_map::add(&gameState->gameWorld.entityHash, *ref, sid);
        //
        ent->getPieces()->iEntityAsset = AssetType_Stone;
        ent->getPieces()->iEntityBounds.offset = v3(0.0f, 0.2f, 0.0f);
        ent->getPieces()->iEntityBounds.radialDimention = v3(0.42f, 0.2f, 0.26f);
        ent->getPieces()->iState = EntityState_Collides | EntityState_Active;
    }

    TEntityReference entityRef = { areaEntity, sid };
    rc_list::push(selectedRgn->m_LocalEntityList, entityRef, &gameState->entityArena);
}
void _AddWall(GameState * gameState, game_map_structs::WorldArea* selectedRgn, const vec3& facing, const vec3& offset) {
    if (!gameState) {
        return;
    }

    WorldAreaEntity* areaEntity = PushStruct(&gameState->entityArena, WorldAreaEntity, game_memory::NoAlign());
    *areaEntity = {};
    // setup initialized entity parameters
    areaEntity->fFacingDirection = facing;
    areaEntity->iWorldPosition.regionCoord = selectedRgn->region;
    areaEntity->iWorldPosition.offset = offset;
    // setup te entity itself, 

    game_render_engine::addRenderAsset(gameState, dAssetType_Tree);
    EntityPieceReference* ref;
    uint32 sid = MakeStoreId(EntityType_Tree, &gameState->gameWorld.lastEntiyId);
    if (!hash_map::get(&gameState->gameWorld.entityHash, sid, ref)) {
        Entity* ent = PushStruct(&gameState->entityArena, Entity);
        *ent = {};
        ent->init(entity_id{ sid }, EntityType_Tree);
        ent->setPieces(PushArray(&gameState->entityArena, 1, EntityPiece), 1);

        areaEntity->gameEntity = ent;

        EntityPieceReference tmpRef = {};
        ref = &tmpRef;
        ref->ptr = ent;

        hash_map::add(&gameState->gameWorld.entityHash, *ref, sid);
        //
        ent->getPieces()->iEntityAsset = dAssetType_Tree;
        ent->getPieces()->iEntityBounds.offset = v3(0.0f, 0.4f, 0.0f);
        ent->getPieces()->iEntityBounds.radialDimention = v3(0.42f, 0.4f, 0.26f);
        ent->getPieces()->iState = EntityState_Collides | EntityState_Active;
    }

    TEntityReference entityRef = { areaEntity, sid };
    rc_list::push(selectedRgn->m_LocalEntityList, entityRef, &gameState->entityArena);
}
void _AddStep(GameState * gameState, game_map_structs::WorldArea* selectedRgn, const vec3& size, const vec3& offset, real32 height) {
    if (!gameState) {
        return;
    }

    WorldAreaEntity* areaEntity = PushStruct(&gameState->entityArena, WorldAreaEntity, game_memory::NoAlign());
    *areaEntity = {};
    // setup initialized entity parameters
    areaEntity->fFacingDirection = v3(0.f, 0.f, 1.f);
    areaEntity->iWorldPosition.regionCoord = selectedRgn->region;
    areaEntity->iWorldPosition.offset = offset;
    // setup te entity itself, 

    EntityPieceReference* ref;
    uint32 sid = MakeStoreId(EntityType_Stair, &gameState->gameWorld.lastEntiyId);
    if (!hash_map::get(&gameState->gameWorld.entityHash, sid, ref)) {
        Stair* ent = PushStruct(&gameState->entityArena, Stair);
        *ent = {};
        ent->maxHeight = height;

        ent->init(entity_id{ sid }, EntityType_Stair);
        ent->setPieces(PushArray(&gameState->entityArena, 1, EntityPiece), 1);

        areaEntity->gameEntity = ent;

        EntityPieceReference tmpRef = {};
        ref = &tmpRef;
        ref->ptr = ent;

        hash_map::add(&gameState->gameWorld.entityHash, *ref, sid);
        //

        //(ent->base + 0)->iEntityAsset = AssetType_None;
        //(ent->base + 0)->iEntityBounds.offset = v3(0.0f, height*0.5f, 0.0f);
        //(ent->base + 0)->iEntityBounds.radialDimention = game_math::absolute(size);
        //(ent->base + 0)->iEntityBounds.radialDimention.y = height * 0.5f;
        //(ent->base + 0)->iState = EntityState_Collides | EntityState_Active | EntityState_Slope;

        (ent->getPieces() + 0)->iEntityAsset = dAssetType_Step;
        (ent->getPieces() + 0)->offset = v3(0.0f, 0.0f, 0.0f);
        (ent->getPieces() + 0)->iEntityBounds.offset = v3(0.0f, -size.y, 0.5f);
        (ent->getPieces() + 0)->iEntityBounds.radialDimention = game_math::absolute(size);
        (ent->getPieces() + 0)->iState = EntityState_Collides | EntityState_Active | EntityState_Slope;
        (ent->getPieces() + 0)->iEntityBounds.offset.y += height*0.5f;
        (ent->getPieces() + 0)->iEntityBounds.radialDimention.y += height*0.5f;
    }

    TEntityReference entityRef = { areaEntity, sid };
    rc_list::push(selectedRgn->m_LocalEntityList, entityRef, &gameState->entityArena);
}
void _AddFloor(GameState * gameState, game_map_structs::WorldArea* selectedRgn) {
    if (!gameState) {
        return;
    }
    WorldAreaEntity* areaEntity = PushStruct(&gameState->entityArena, WorldAreaEntity, game_memory::NoAlign());
    *areaEntity = {};
    // setup initialized entity parameters
    areaEntity->iWorldPosition.regionCoord = selectedRgn->region;
    areaEntity->iWorldPosition.offset = v3(0.0f, -game_math::minOffset, 0.0f);
    // setup te entity itself, 

    EntityPieceReference* ref;
    uint32 sid = MakeStoreId(EntityType_None, &gameState->gameWorld.lastEntiyId);
    if (!hash_map::get(&gameState->gameWorld.entityHash, sid, ref)) {
        Entity * ent = PushStruct(&gameState->entityArena, Entity);
        *ent = {};
        ent->init(entity_id{ sid }, EntityType_None);
        ent->setPieces(PushArray(&gameState->entityArena, 1, EntityPiece), 1);

        areaEntity->gameEntity = ent;

        EntityPieceReference tmpRef = {};
        ref = &tmpRef;
        ref->ptr = ent;

        hash_map::add(&gameState->gameWorld.entityHash, *ref, sid);
        //
        (ent->getPieces() + 0)->iEntityAsset = AssetType_None;
        (ent->getPieces() + 0)->iEntityBounds.offset = v3(0.0f, -game_math::minOffset, 0.0f);
        (ent->getPieces() + 0)->iEntityBounds.radialDimention = v3(gameState->regionUnitDimentions.x * 0.5f, game_math::epsilon, gameState->regionUnitDimentions.z * 0.5f);
        (ent->getPieces() + 0)->iState = EntityState_Collides | EntityState_Active;
    }

    TEntityReference entityRef = { areaEntity, sid };
    rc_list::push(selectedRgn->m_LocalEntityList, entityRef, &gameState->entityArena);
}

#define LEFT_B 1
#define RIGHT_B 2
#define TOP_B 4
#define BOTTOM_B 8
void _AddBarrier(GameState * gameState, game_map_structs::WorldArea* selectedRgn, vec3 offset, vec3 size) {
	if (!gameState) {
		return;
	}
	WorldAreaEntity* areaEntity = PushStruct(&gameState->entityArena, WorldAreaEntity, game_memory::NoAlign());
	*areaEntity = {};
	// setup initialized entity parameters
	areaEntity->iWorldPosition.regionCoord = selectedRgn->region;
	areaEntity->iWorldPosition.offset = offset;
	// setup te entity itself, 

	EntityPieceReference* ref;
	uint32 sid = MakeStoreId(EntityType_None, &gameState->gameWorld.lastEntiyId);
	if (!hash_map::get(&gameState->gameWorld.entityHash, sid, ref)) {
		Entity * ent = PushStruct(&gameState->entityArena, Entity);
		*ent = {};
        ent->init(entity_id{ sid }, EntityType_None);
        ent->setPieces(PushArray(&gameState->entityArena, 1, EntityPiece), 1);

		areaEntity->gameEntity = ent;

		EntityPieceReference tmpRef = {};
		ref = &tmpRef;
		ref->ptr = ent;

		hash_map::add(&gameState->gameWorld.entityHash, *ref, sid);
		//
		(ent->getPieces() + 0)->iEntityAsset = AssetType_None;
		(ent->getPieces() + 0)->iEntityBounds.offset = v3(0.0f, gameState->regionUnitDimentions.y * 0.5f, 0.0f);
		(ent->getPieces() + 0)->iEntityBounds.radialDimention = game_math::hadamard((gameState->regionUnitDimentions * 0.5f) , size);
		(ent->getPieces() + 0)->iState = EntityState_Collides | EntityState_Active;
	}

	TEntityReference entityRef = { areaEntity, sid };
	rc_list::push(selectedRgn->m_LocalEntityList, entityRef, &gameState->entityArena);
}
void _AddBarrier(GameState * gameState, game_map_structs::WorldArea* selectedRgn, uint32 t) {
	vec3 off = gameState->regionUnitDimentions * 0.5f;
	if (t & LEFT_B) {
		_AddBarrier(gameState, selectedRgn, v3(-off.x + game_math::minOffset, 0.0f, 0.0f), v3(game_math::epsilon, 1.0f, 1.0f));
	}
	if (t & RIGHT_B) {
		_AddBarrier(gameState, selectedRgn, v3(off.x - game_math::minOffset, 0.0f, 0.0f), v3(game_math::epsilon, 1.0f, 1.0f));
	}
	if (t & TOP_B) {
		_AddBarrier(gameState, selectedRgn, v3(0.0f, 0.0f, -off.z + game_math::minOffset), v3(1.0f, 1.0f, game_math::epsilon));
	}
	if (t & BOTTOM_B) {
		_AddBarrier(gameState, selectedRgn, v3(0.0f, 0.0f, off.z - game_math::minOffset), v3(1.0f, 1.0f, game_math::epsilon));
	}
}
void _AddHero(GameState * gameState, game_map_structs::WorldArea* selectedRgn, const vec3& facing, const vec3& offset) {
    WorldAreaEntity* areaEntity = PushStruct(&gameState->entityArena, WorldAreaEntity, game_memory::NoAlign());
    *areaEntity = {};
    // setup initialized entity parameters
    areaEntity->fFacingDirection = facing;
    areaEntity->iWorldPosition.regionCoord = selectedRgn->region;
    areaEntity->iWorldPosition.offset = offset;
    areaEntity->iWorldPosition.offset.y += coord_system::coordMinOffset;
    // setup te entity itself, 

    game_render_engine::addRenderAsset(gameState, dAssetType_HeroShadow);
    game_render_engine::addRenderAsset(gameState, dAssetType_HeroCape);
    game_render_engine::addRenderAsset(gameState, dAssetType_HeroTorso);
    game_render_engine::addRenderAsset(gameState, dAssetType_HeroHead);
    EntityPieceReference* ref;
    uint32 iStoredId = MakeStoreId(EntityType_Player, &gameState->gameWorld.lastEntiyId);
    if (!hash_map::get(&gameState->gameWorld.entityHash, iStoredId, ref)) {
        Hero* hero = PushStruct(&gameState->entityArena, Hero);
        *hero = {};
        hero->maxHitPoints = 100;
        hero->hitPoints = 100;
        Entity * ent = hero;

        ent->init(entity_id{ iStoredId }, EntityType_Player);
        ent->setPieces(PushArray(&gameState->entityArena, 4, EntityPiece), 4);
        areaEntity->gameEntity = ent;

        EntityPiece* e = ent->getPieces();
        //
        e->iEntityAsset = dAssetType_HeroShadow;
        e->iEntityBounds.offset = v3(0.f, 0.f, 0.0f);
        e->iEntityBounds.radialDimention = v3(0.02f, 0.02f, 0.02f);
        e->iState = EntityState_Grounded | EntityState_none;
        e->next = e + 1;
        *(++e) = {}; 

        e->next = e + 1;
        e->iEntityAsset = dAssetType_HeroTorso;
        e->offset = v3(0.0f, 0.0f, 0.0f);
        e->iEntityBounds.offset = v3(0.0f, 0.35f, 0.0f);
        e->iEntityBounds.radialDimention = v3(0.165f, 0.35f, 0.085f);
        e->iState = EntityState_Active | EntityState_Collides;

        *(++e) = {}; // cape
        e->next = e + 1;
        e->offset = v3(0.0f, 0.0f, 0.0f);
        e->iEntityAsset = dAssetType_HeroCape;
        e->iEntityBounds.offset = v3(0.0f, 0.2f, 0.0f);
        e->iEntityBounds.radialDimention = v3(0.22f, 0.15f, 0.08f);
        e->iState = EntityState_Collides;

        *(++e) = {}; //head
        e->next = NULL;
        e->iEntityAsset = dAssetType_HeroHead;
        e->offset = v3(0.0f, 0.0f, 0.0f);
        e->iEntityBounds.offset = v3(0.0f, 0.5f, 0.0f);
        e->iEntityBounds.radialDimention = v3(0.22f, 0.2f, 0.08f);
        e->iState = EntityState_Collides;
        e->maxTime = 1.0f;
        e->animTime = 0.0f;

        EntityPieceReference tmpRef = {};
        ref = &tmpRef;
        ref->ptr = ent;
        hash_map::add(&gameState->gameWorld.entityHash, *ref, iStoredId);
    }

    TEntityReference entityRef = { areaEntity, iStoredId };
    rc_list::push(selectedRgn->m_LocalEntityList, entityRef, &gameState->entityArena);
    gameState->controlledEntity = areaEntity;
}
void _AddMonster(GameState * gameState, game_map_structs::WorldArea* selectedRgn, const vec3& facing, const vec3& offset) {
    WorldAreaEntity* areaEntity = PushStruct(&gameState->entityArena, WorldAreaEntity, game_memory::NoAlign());
    *areaEntity = {};
    // setup initialized entity parameters
    areaEntity->fFacingDirection = facing;
    areaEntity->iWorldPosition.regionCoord = selectedRgn->region;
    areaEntity->iWorldPosition.offset = offset;
    areaEntity->iWorldPosition.offset.y += coord_system::coordMinOffset;
    // setup te entity itself, 

    game_render_engine::addRenderAsset(gameState, AssetType_HeroCape);
    EntityPieceReference* ref;
    uint32 iStoredId = MakeStoreId(EntityType_NPC, &gameState->gameWorld.lastEntiyId);
    if (!hash_map::get(&gameState->gameWorld.entityHash, iStoredId, ref)) {
        Monster* npc = PushStruct(&gameState->entityArena, Monster);
        *npc = {};
        npc->maxHitPoints = 25;
        npc->hitPoints = 25;
        Entity * ent = npc;

        ent->init(entity_id{ iStoredId }, EntityType_NPC);
        ent->setPieces(PushArray(&gameState->entityArena, 3, EntityPiece), 3);
        areaEntity->gameEntity = ent;

        npc->setAI(PushStruct(&gameState->entityArena, Brain));

        EntityPiece* e = PushArray(&gameState->entityArena, 3, EntityPiece);
        //
        e = ent->getPieces();
        e->iEntityAsset = AssetType_HeroShadow;
        e->iEntityBounds.offset = v3(0.f, 0.f, 0.0f);
        e->iEntityBounds.radialDimention = v3(0.02f, 0.02f, 0.02f);
        e->iState = EntityState_Grounded | EntityState_none;
        e->next = e + 1;

        *(++e) = {}; // cape
        e->next = e + 1;
        e->offset = v3(0.0f, 0.0f, 0.0f);
        e->iEntityAsset = AssetType_HeroCape;
        e->iEntityBounds.offset = v3(0.0f, 0.2f, 0.0f);
        e->iEntityBounds.radialDimention = v3(0.22f, 0.2f, 0.08f);
        e->iState = EntityState_Active | EntityState_Collides;

        EntityPieceReference tmpRef = {};
        ref = &tmpRef;
        ref->ptr = ent;
        hash_map::add(&gameState->gameWorld.entityHash, *ref, iStoredId);
    }

    TEntityReference entityRef = { areaEntity, iStoredId };
    rc_list::push(selectedRgn->m_LocalEntityList, entityRef, &gameState->entityArena);
}

game_map_structs::WorldArea * addRoom(GameState* gameState, vec3i& baseRoomPosition) {
    game_map_structs::GameWorld & world = gameState->gameWorld;
    game_map_structs::WorldArea *selectedRgn;
    // add room
    if (!world.getRegion(baseRoomPosition, selectedRgn)) {
        world.addRegion(baseRoomPosition);
        if (!world.getRegion(baseRoomPosition, selectedRgn)) {
            Assert(!"Error");
        }
    }

    _AddFloor(gameState, selectedRgn);
#if 1
//    _AddStep(gameState, selectedRgn, v3(0.5f, 0.001f, 0.5f), v3(2.0f, -game_math::minOffset, 0.0f), 1.1f);
    // add entities

	//         ent->base->iEntityBounds.radialDimention = v3(0.42f, 0.4f, 0.26f);

    real32 stepx = gameState->regionUnitDimentions.x / 0.84f;
    real32 stepy = gameState->regionUnitDimentions.z / 0.52f;

    int my = ((int)ceilf(stepy));
    int mx = ((int)ceilf(stepx));

    for (int ey = 0; ey < my; ++ey) {
        for (int ex = 0; ex < mx; ++ex) {

			vec3 place = v3(0.42f + ex * 0.84f - (gameState->regionUnitDimentions.x*0.5f), 0.0f, 0.26f + ey * 0.52f - (gameState->regionUnitDimentions.z*0.5f));

            uint32 id = AssetType_Tree;
            if (ey == 0 || ey == (my - 1)) {
                if (ex != 3) {
                    _AddWall(gameState, selectedRgn, v3(0.0f, 0.0f, -1.0f), place);
                }
            } else if (ey == 1 && ex == 5) {
                _AddHalfWall(gameState, selectedRgn, v3(0.0f, 0.0f, -1.0f), place);
            } else if (ex == 0 || ex == (mx - 1)) {
                if (ey != 3) {
                    _AddWall(gameState, selectedRgn, v3(0.0f, 0.0f, -1.0f), place);
                }
            }
        }
    }
#endif
	return selectedRgn;
}

void DEBUG_MAPInit(GameState* gameState) {
    game_map_structs::GameWorld & world = gameState->gameWorld;

    // add the hero
#if 1
    {
        game_map_structs::WorldArea* selectedRgn;
        // add room
        if (!world.getRegion(v3i(51, 101, 52), selectedRgn)) {
            world.addRegion(v3i(51, 101, 52));
            if (!world.getRegion(v3i(51, 101, 52), selectedRgn)) {
                Assert(!"Error");
            }
        }
        _AddHero(gameState, selectedRgn, v3(0.f, 0.0f, 1.f), v3(0.5f, 1.04f, 0.5f));
    }
#endif

#if 1
    random::random_series s = random::RandomSeed(1);
    for (int level = 0; level < 1; ++level) {
        vec3i baseRoomPosition = v3i(50, level + 100, 50);
        for (int roomCount = 0; roomCount < 1; ++roomCount) {
            // each room block is 3x3, rooms may overlap
            for (int roomY = 0; roomY < 3; ++roomY) {
                bool isLeft = false; // this row goes left?

                for (int roomX = 0; roomX < 3; ++roomX) {
                    game_map_structs::WorldArea* selectedRgn = addRoom(gameState, baseRoomPosition);
					if (selectedRgn) {
						uint32 code = 0;
						if (roomX == 0) {
							code |= LEFT_B;
						}
						if (roomX == 2) {
							code |= RIGHT_B;
						}
						if (roomY == 0) {
							code |= TOP_B;
						}
						if (roomY == 2) {
							code |= BOTTOM_B;
						}
						_AddBarrier(gameState, selectedRgn, code);
					}

                    baseRoomPosition += v3i(1, 0, 0);
                }
                baseRoomPosition += v3i(-3, 0, 1);
            }
            if (roomCount % 4 == 0) {
                baseRoomPosition += v3i(3, 0, -2);
            }
            if (roomCount % 4 == 1) {
                baseRoomPosition += v3i(2, 0, -1);
            }
            if (roomCount % 4 == 2) {
                baseRoomPosition += v3i(2, 0, -2);
            }
            if (roomCount % 4 == 3) {
                baseRoomPosition += v3i(3, 0, -1);
            }
        }
    }
    for (int level = 0; level < 100; ++level) {
        game_map_structs::WorldArea* selectedRgn;
        // each room block is 3x3, rooms may overlap
        for (int roomX = 0; roomX < 3; ++roomX) {
            if (!world.getRegion(v3i(50 + roomX, level, 49), selectedRgn)) {
                world.addRegion(v3i(50 + roomX, level, 49));
                if (!world.getRegion(v3i(50 + roomX, level, 49), selectedRgn)) {
                    Assert(!"Error");
                }
            }
            _AddWall(gameState, selectedRgn, v3(0.f), v3(-3.f,-1.49f,3.f));
            if (!world.getRegion(v3i(50 + roomX, level, 53), selectedRgn)) {
                world.addRegion(v3i(50 + roomX, level, 53));
                if (!world.getRegion(v3i(50 + roomX, level, 53), selectedRgn)) {
                    Assert(!"Error");
                }
            }
            _AddWall(gameState, selectedRgn, v3(0.f), v3(-3.f, -1.49f, -3.f));
        }

        for (int roomY = 0; roomY < 3; ++roomY) {
            if (!world.getRegion(v3i(49, level, 50 + roomY), selectedRgn)) {
                world.addRegion(v3i(49, level, 50 + roomY));
                if (!world.getRegion(v3i(49, level, 50 + roomY), selectedRgn)) {
                    Assert(!"Error");
                }
            }
            _AddWall(gameState, selectedRgn, v3(0.f), v3(2.f, -1.49f, -2.f));
            if (!world.getRegion(v3i(53, level, 50 + roomY), selectedRgn)) {
                world.addRegion(v3i(53, level, 50 + roomY));
                if (!world.getRegion(v3i(53, level, 50 + roomY), selectedRgn)) {
                    Assert(!"Error");
                }
            }
            _AddWall(gameState, selectedRgn, v3(0.f), v3(-2.f, -1.49f, -2.f));
        }
    }

#else

    game_map_structs::WorldArea* selectedRgn = 0L;
    // add room
    if (!world.getRegion(v3i(51, 100, 52), selectedRgn)) {
        world.addRegion(v3i(51, 100, 52));
        if (!world.getRegion(v3i(51, 100, 52), selectedRgn)) {
            Assert(!"Error");
        }
    }
    ////_AddWall(gameState, selectedRgn, v3(0.0f, 0.0f, -1.0f), v3(gameState->regionUnitDimentions.x*0.5, 0.0f, 0.0f));
    //_AddStep(gameState, selectedRgn, v3(gameState->regionUnitDimentions.x * 0.5f, 0.0f, gameState->regionUnitDimentions.z * 0.5f), v3(0.0f));
    _AddFloor(gameState, selectedRgn);

    //addRoom(gameState, v3i(51, 0, 52));

    //// add room
    //if (!world.getRegion(v3i(51, 0, 52), selectedRgn)) {
    //    world.addRegion(v3i(51, 0, 52));
    //    if (!world.getRegion(v3i(51, 0, 52), selectedRgn)) {
    //        Assert(!"Error");
    //    }
    //}
    ////_AddWall(gameState, selectedRgn, v3(0.0f, 0.0f, -1.0f), v3(gameState->regionUnitDimentions.x*0.5, 0.0f, 0.0f));
    //_AddStep(gameState, selectedRgn, v3(gameState->regionUnitDimentions.x * 0.5f, 0.0f, gameState->regionUnitDimentions.z * 0.5f), v3(0.0f));
	
    //for (int il=0;il<4;++il) {
    //    game_map_structs::WorldArea* selectedRgn;
    //    // add room
    //    if (!world.getRegion(v3i(50+il, 0, 50), selectedRgn)) {
    //        world.addRegion(v3i(50+il, 0, 50));
    //        if (!world.getRegion(v3i(50+il, 0, 50), selectedRgn)) {
    //            Assert(!"Error");
    //        }
    //    }
    //    _AddStep(gameState, selectedRgn, v3(gameState->regionUnitDimentions.x * 0.5f, 0.0f, gameState->regionUnitDimentions.z * 0.5f), v3(0.0f));
    //    for (real32 zOff = -3.f; zOff < 3.f; zOff += 0.8f) {
    //        _AddWall(gameState, selectedRgn, v3(0.0f, 0.0f, 1.0f), v3(-2.0f, 0.0f, zOff));
    //    }
    //}

#endif
}