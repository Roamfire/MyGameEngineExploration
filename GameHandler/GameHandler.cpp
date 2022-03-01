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
#include "GameHandler.h"

#include "../RaidCore/RaidCore_FileFormats.cpp"
#include "../RaidCore/RaidCore_Utils.cpp"
#include "../RaidCore/RaidCore_Math.cpp"
#include "../RaidCore/RaidCore_MemManager.cpp"
#include "../RaidCore/RaidCore_WorldCoordinates.cpp"
#include "../RaidCore/RaidCore_RandomMath.cpp"
#include "../RaidCore/RaidCore_Physics.cpp"
#include "../GameHandler/RaidCore_SoundSystem.cpp"
#include "../RaidCore/RaidCore_RenderHandler.cpp"
#include "../RaidCore/RaidCore_Win32Timer.cpp"
#include "../RaidCore/RaidCore_Win32Utils.cpp"
#include "../GameHandler/RaidCore_GameEntities.cpp"
#include "../RaidCore/RaidCore_Actor.cpp"
#include "../GameHandler/GameHandler_Management.cpp"
#include "../RaidCore/RaidCore_GameState.cpp"
#include "../RaidCore/RaidCore_GameWorld.cpp"
#include "../RaidCore/RaidCore_Asset.cpp"
#include "../RaidCore/Raid_2DRenderDeviceImpl.cpp"

#ifdef RC_TESTCODE
#include "../GameHandler/debug_s.h"
extern debug_timer *DEBUGGlobalMemory = NULL;

#if _SHOW_AABB_PLAYER
GfxColor pl_aabb = (GfxColor)rgba(255, 0, 0, 128);
#endif
#if _SHOW_AABB_NPC
GfxColor npc_aabb = (GfxColor)rgba(160, 0, 0);
#endif
#if _SHOW_AABB_WALL
GfxColor wall_aabb = (GfxColor)rgba(0, 255, 255);
#endif
#if _SHOW_TILEGRID
GfxColor grid_aabb = (GfxColor)rgba(155, 200, 0);
#endif
#if _SHOW_RENDER_REGION
GfxColor regionx_aabb = (GfxColor)rgba(100, 255, 0);
GfxColor region_aabb = (GfxColor)rgba(0, 255, 0);
#endif
#if _SHOW_AABB_IMAGE
GfxColor c_aabb_image = rgba(255, 0, 0, 128);
#endif
#if _SHOW_TILEGRID
GfxColor grid_aabb = (GfxColor)rgba(155, 200, 0);
#endif

#endif

rc_macro_cpp void updateScreenViewport(GameState * gameState) {
    gameState->viewUnitDimention.z = render_const::screenUnitDimention.y;
    gameState->viewUnitDimention.x = render_const::screenUnitDimention.y * gameState->screenAspectRatio;
    gameState->unitsToPixels = (real32)(gameState->screenPixelResolution.x * render_const::monitorWidth);
}

GAMEHANDLER_API UPDATE_GAME_STATE(UpdateGameState) {
    GameState * gameState = gameMemory->gameState;
    if (!gameState) {
        return;
    }

    if ((screenHeight > 0 && screenWidth > 0) && ((screenWidth != gameState->screenPixelResolution.x) || (screenHeight != gameState->screenPixelResolution.y))) {
        gameState->screenPixelResolution = v2i(screenWidth, screenHeight);
        gameState->screenAspectRatio = game_math::safeRatio0(screenWidth, screenHeight);
        real32 regionSizeCoefficient = 1.0f;
        // always keep screen width in X units
        updateScreenViewport(gameState);

        game_render_engine::resizeRenderer(gameState, screenWidth, screenHeight);
    }

    InitAndLoadMapLevel(gameState, NULL);
}

GAMEHANDLER_API GAME_INDEPENDANT_PROCESS(GameIndependantProcess) {
    BEGIN_DEBUG_BLOCK;

    GameState * gameState = gameMemory->gameState;
    // NOTE(Roman)
    // if level is not initialized, we'll do it here
    // this would include:
    // 1. setup the map
    // 2. spawn tasks to load the assets; assets are loaded by priority
    switch (gameState->gameState) {
        case control_state_load_level: {
            game_memory::resetArena(&gameState->assetArena);
            break;
        }
        case control_state_load_level_stream: {

            break;
        }
        case control_state_pause: {
            break;
        }
        case control_state_run_mapTransition: {
            gameState->inputData.Delta = v3(0.f);
            gameState->inputData.motionMagnitude = 0.f;
            gameState->inputData.motionDirection = v3(0.f);
            break;
        }
        case control_state_run: {
            // second... update moving actors in the world active area(s),
            //	* the world active area is the minimal area that would react to the player.
            //    there can be multiple active areas, defined by the game type... also, a player may be defined as multiple moving actors
            // - calculate world active area(s), create a list of actors in the area(s) based on distance from player...
            // - process AI of each actor in area
            // - validate state change for each actor
            // - update state
            gameState->inputData.Delta.x = user_input::keyForce(input, user_input::key_horizontal);
            //    gameState->inputData.Delta.y = user_input::keyForce(input, user_input::key_btn_A) * 10.0f;
            gameState->inputData.Delta.z = user_input::keyForce(input, user_input::key_vertical);
            // NOTE(Roman): motionMagnitude is the scaling factor to set the motionDirection (acceleration) to a max unit vector
            // becuase the magnitude of (1,0,0) is equal to (0,1,0) but less then (1,1,0)
            gameState->inputData.motionMagnitude = game_math::safeRatio0(1.0f, game_math::length(gameState->inputData.Delta));
            if (1.f < gameState->inputData.motionMagnitude) {
                gameState->inputData.Delta /= gameState->inputData.motionMagnitude;
                gameState->inputData.motionMagnitude = 1.0f;
            }
            gameState->inputData.motionDirection = gameState->accelerateRate * gameState->inputData.Delta;

#ifdef RC_TESTCODE
            if (user_input::wasPressed(input, user_input::key_btn_A) && game_math::isZero(gameState->controlledEntity->iPtSpeed.y)) {
                gameState->controlledEntity->iPtSpeed.y = game_math::clamp(1.0f, gameState->controlledEntity->iPtSpeed.y + 3.5f, 3.5f);
                user_input::reset(input, user_input::key_btn_A);
            }

            vec3 move = { 0.f, 0.f, 0.f };
            if (user_input::isPressed(input, user_input::key_vertical)) {
                if (user_input::isPressed(input, user_input::key_btn_special3)) {
                    move.y = -gameState->maxSpeed_mps * user_input::keyForce(input, user_input::key_vertical);
                } else {
                    move.z = gameState->maxSpeed_mps * user_input::keyForce(input, user_input::key_vertical);
                }
            }
            if (user_input::isPressed(input, user_input::key_horizontal)) {
                move.x = gameState->maxSpeed_mps * user_input::keyForce(input, user_input::key_horizontal);
            }

            //
            if (user_input::wasPressed(input, user_input::key_btn_specialF)) {
                // set debug camera
                gameState->isDebugCamera = !gameState->isDebugCamera;
                if (gameState->isDebugCamera) {
                    gameState->activeCamera = &gameState->debugEntity;
                } else {
                    gameState->activeCamera = &gameState->cameraEntity;
                }
                user_input::reset(input, user_input::key_btn_specialF);
            }
            if (user_input::wasPressed(input, user_input::key_btn_specialE)) {
                // set debug camera
                gameState->isOrthographic = !gameState->isOrthographic;
                user_input::reset(input, user_input::key_btn_specialE);
            }
            if (gameState->isDebugCamera) {
                real32 zoomMod = 0.0f;
                Swap<real32>(&zoomMod, &move.y);

                if (game_math::absolute(zoomMod) > game_math::epsilon) {
                    gameState->debugEntity.distanceAboveTarget = game_math::clamp(1.5f, gameState->debugEntity.distanceAboveTarget + 0.05f * zoomMod, 31.6f);
                }
                coord_system::offsetRegionCoord(move * frameTime, gameState->regionUnitDimentions, &gameState->activeCamera->iWorldPosition);

                gameState->inputData.Delta = v3(0.f);
                gameState->inputData.motionMagnitude = 0.f;
                gameState->inputData.motionDirection = v3(0.f);
            }
#endif
            break;
        }
        case control_state_shutdown: {
            break;
        }
    }
}

GAMEHANDLER_API GAME_UPDATE_AND_RENDER(GameUpdateAndRender) {
    BEGIN_TIMED_BLOCK(GameUpdate);

    GameState * gameState = gameMemory->gameState;
    TemporaryMemory scratchRenderMemory = BeginTemporaryMemory(&gameState->scratchFrameArena);
    TemporaryMemory scratchUpdateMemory = BeginTemporaryMemory(&gameState->scratchUpdateArena);

    GameCamera& cameraEntity = *gameState->activeCamera;
    UpdateCamera(gameState, frameTime);

    /*
    TODO(roman):
    In addition to the background tiles we want to draw,
    We need to process all the stationary elements and the mobile actors in the current Z plane...
    */

    game_render_engine::RenderGroup * renderGroup = game_render_engine::startRenderProcess(gameState);
    game_render_engine::perspective(renderGroup, gameState->screenPixelResolution.x, gameState->screenPixelResolution.y, gameState->unitsToPixels, cameraEntity.focalLength, cameraEntity.distanceAboveTarget);

#ifdef RC_TESTCODE
    if (gameState->isOrthographic && gameState->isDebugCamera) {
        game_render_engine::orthographic(renderGroup, gameState->screenPixelResolution.x, gameState->screenPixelResolution.y, gameState->unitsToPixels, cameraEntity.focalLength, cameraEntity.distanceAboveTarget);
    }
#endif

    // first command we want is the screen clear
    game_render_engine::pushClear(gameState, 0x00044aa4); // color 0xAARRGGBB

    // Try to prepare the drawing assets and sort them in the drawing order
    vec2 screenCenter = v2(0.5f*(real32)gameState->screenPixelResolution.x,
                             0.5f*(real32)gameState->screenPixelResolution.y);

    GfxBox basis = game_rect::createGfxRect(gameState->viewUnitDimention * 0.5f);
    game_rect::expand(basis, 0.0f, gameState->regionUnitDimentions.y, 0.0f);
    game_rect::offset(basis, 0.0f, -gameState->regionUnitDimentions.y * 0.5f, 0.0f);
    real32 maxWidth = (basis.maxPoint.x - basis.minPoint.x);
    real32 maxHeight = (basis.maxPoint.z - basis.minPoint.z);

    BEGIN_TIMED_BLOCK(LoadSim);
    UpdateEntityList simEntityList = rc_list::initList<SimUpdateEntity>();
    BeginRegionSimulation(gameState, &simEntityList, basis);
    END_TIMED_BLOCK(LoadSim);

    BEGIN_TIMED_BLOCK(UpdateLoop);
    SimUpdateEntity* simEntity = NULL;
    rc_list::iterator<SimUpdateEntity>& itSearch = rc_list::getIterator(simEntityList);
    while (itSearch.has()) {
        simEntity = itSearch.selected();
        // NOTE(Roman): update the simEntity
        game_play::UpdateEntity(gameState, simEntity, simEntity->iPtSpeed, simEntityList, frameTime);
        itSearch.next();
    }
    END_TIMED_BLOCK(UpdateLoop);
    
    BEGIN_TIMED_BLOCK(ProcessVisuals);
    BEGIN_TIMED_BLOCK(GameRenderInit);
    rc_list::iterator<SimUpdateEntity> it = rc_list::getIterator(simEntityList);
    game_render_scene::RenderSpace * renderSpace = game_render_scene::InitRenderSpace(gameState, maxWidth, maxHeight);
    while (simEntity = it++) {
        if (EntityType_None != simEntity->entityType) {
            coord_system::RegionCoord targetCoord = gameState->cameraEntity.iWorldPosition;
            coord_system::offsetRegionCoord(simEntity->renderOffset, gameState->regionUnitDimentions, &targetCoord);
            real32 groundOffset = targetCoord.offset.y;
            targetCoord.offset.y = 0.0;
            vec3 renderPoint = coord_system::getDistance(targetCoord, cameraEntity.iWorldPosition, gameState->regionUnitDimentions);
            game_render_scene::GfxRenderUnit* renderCmd = game_render_scene::InsertRenderCommand(gameState, renderSpace, simEntity->base, renderPoint, groundOffset, NULL);
            if (renderCmd) {
                switch (simEntity->entityType) {
                    case EntityType_Player:
                        break;
                    case EntityType_NPC:
                        break;
                    default:
                        break;
                }
                if (simEntity->entityRef.ref->fFacingDirection.z < 0.0f) {
                    renderCmd->angle = mathf::acosf(game_math::dotProduct(simEntity->entityRef.ref->fFacingDirection, v3(1.0f, 0.0f, 0.0f)));
                } else {
                    renderCmd->angle = mathf::acosf(game_math::dotProduct(simEntity->entityRef.ref->fFacingDirection, v3(-1.0f, 0.0f, 0.0f))) + game_math::angle2Radians * 180;
                }
            }
#ifdef RC_TESTCODE
            if (gameState->isDebugCamera) {
                //coord_system::RegionCoord targetCoord = simEntity->entityRef.ref->iWorldPosition;
                //targetCoord.offset.y = 0.0;
                //simEntity->renderOffset = coord_system::getDistance(targetCoord, gameState->cameraEntity.iWorldPosition, gameState->regionUnitDimentions);
            }
#endif
        }
    }
    game_render_scene::OrderRenderVolumes(&gameState->scratchUpdateArena, renderSpace);
    EndRegionSimulation(gameState, &simEntityList);
    END_TIMED_BLOCK(GameRenderInit);

    //
    // NOTE(Roman): This section is the render setup, pushing all updated entities to the render queue.
    // 
    while (game_render_scene::GfxRenderUnit* renderCmd = game_render_scene::GetNextRenderCommand(renderSpace)) {
        real32 k = game_math::clamp(0.0f, game_math::absolute(renderCmd->renderPoint.y + gameState->regionUnitDimentions.y) - gameState->regionUnitDimentions.y - game_math::epsilon, gameState->regionUnitDimentions.y);
        renderGroup->renderAlpha = 1.0f - game_math::clamp01(game_math::safeRatio0(k, gameState->regionUnitDimentions.y));
        //
#ifdef RC_TESTCODE
        if (gameState->isDebugCamera) {
            renderGroup->renderAlpha = 1.0f;
        }
#endif
        if (game_math::epsilon < renderGroup->renderAlpha) {
            //
            EntityPiece* e = renderCmd->renderSource;
            vec3 offset = v3(0.0f);
            while (e) {
                offset += e->offset;
                renderGroup->transform.offset = renderCmd->renderPoint;
                vec3 entityP = v3(offset.x, renderCmd->offsetOnY + offset.y, offset.z);

                if (Entity::isEntityState(e, EntityState_Grounded)) {
                    if (entityP.y > 0.0f) {
                        entityP.y = 0.0f;
                    }
                }
                // NOTE(Roman): k is the relative offset outside of the visible cube
                // so beyond a certain point (perpendicular to the plane of the camera), the object begins to fade.
#ifdef RC_TESTCODE
                if (Entity::isEntityState(e, EntityState_Collides)) {
                    real32 ra = renderGroup->renderAlpha;
                    renderGroup->renderAlpha = ra * 0.75f;
                    vec3 rad = v3(e->iEntityBounds.radialDimention.x, 0.0f, e->iEntityBounds.radialDimention.z);
                    vec3 ep = entityP;
                    ep += e->iEntityBounds.offset;
                    ep.y -= e->iEntityBounds.radialDimention.y;
                    game_render_engine::pushRect(renderGroup, ep, rad, (AssetType)e->iEntityAsset != AssetType_None, c_aabb_image);
                    //ep.y += 2.0f * e->iEntityBounds.radialDimention.y;
                    //game_render_engine::pushRect(renderGroup, ep, rad, false32, c);
                    renderGroup->renderAlpha = ra;
                }
#endif
                if (game_math::isNotZero(e->maxTime)) {
                    e->animTime += frameTime;
                    if (e->animTime >= e->maxTime) {
                        e->animTime -= e->maxTime;
                    }
                }

                game_render_engine::GfxTexture *image;
                asset_vector av = { 0.0f, 0.0f, renderCmd->angle, e->animTime };
                asset_vector wt = { 0.0f, 0.0f, 1.0f, 1.0f };
                BitmapID bid = BestMatchAsset(renderGroup->assets, (AssetType)e->iEntityAsset, &av, &wt);
                image = GetBitmap(renderGroup->assets, bid);
                if (image) {
                    game_render_engine::pushSprite(renderGroup, image, 1.0f, entityP, render_const::defaultColor);
                } else {
                    LoadGameBitmap(gameState, { e->iEntityAsset });
                    ++renderGroup->MissingResourceCount;
                }
                e = e->next;
            }
        }
        //
    }
#ifdef RC_TESTCODE
#if _SHOW_RENDER_REGION
    if (gameState->isDebugCamera) {
        real32 ra = renderGroup->renderAlpha;
        renderGroup->renderAlpha = 1.0f;
        coord_system::RegionCoord targetCoord = gameState->cameraEntity.iWorldPosition;
        targetCoord.offset.y = 0.0;
        renderGroup->transform.offset = coord_system::getDistance(targetCoord, cameraEntity.iWorldPosition, gameState->regionUnitDimentions);
        vec3 radialDimention = v3(gameState->screenPixelResolution.x / gameState->unitsToPixels, 0.f, gameState->screenPixelResolution.y / gameState->unitsToPixels) * game_math::safeRatio1(gameState->cameraEntity.distanceAboveTarget, gameState->cameraEntity.focalLength) * 0.5f;
        vec3 noOffset  = v3(0.0f);
        game_render_engine::pushRect(renderGroup, noOffset, radialDimention, false32, regionx_aabb);
        renderGroup->renderAlpha = ra;
    }
#endif
#endif

    END_TIMED_BLOCK(ProcessVisuals);
    END_TIMED_BLOCK(GameUpdate);

    game_render_engine::completeRender(gameState);
    EndTemporaryMemory(scratchUpdateMemory);
    EndTemporaryMemory(scratchRenderMemory);
}

GAMEHANDLER_API INIT_GAME_STATE(InitGameState) {
    BEGIN_DEBUG_BLOCK;

    GameState * gameState = gameMemory->gameState;
    if (!gameState) {
        gameState = gameMemory->gameState = PushStruct(gameMemory->GamePersistentMemoryArena, GameState);
        gameState->gameState = control_state_init;
        gameState->runState = run_state_run;

		//
		gameState->gameInput = input;
        
        //
        // TODO(Roman): Do we need an internal timer??
        // internal timer would be used for per frame timing
        gameState->raidTickGlobal = PushStruct(gameMemory->GamePersistentMemoryArena, game_timer::RaidTick);
        game_timer::Init(gameState->raidTickGlobal);

        //
        // INITIAL SETUP OF UNITS
        gameState->maxSpeed_mps = 7.5f;
        gameState->accelerateRate = 30.0f;
        //
        // Persist == 320 Mb
        gameState->engineArena = *game_memory::createMemoryArena(gameMemory->GamePersistentMemoryArena, MByte(96) - sizeof(game_memory::arena)); // 224Mb
        gameState->assetArena = *game_memory::createMemoryArena(gameMemory->GamePersistentMemoryArena, MByte(96) - sizeof(game_memory::arena));  // 160Mb
        gameState->entityArena = *game_memory::createMemoryArena(gameMemory->GamePersistentMemoryArena, MByte(64) - sizeof(game_memory::arena)); // 64Mb
        // Transcient
        gameState->scratchFrameArena = *game_memory::createMemoryArena(gameMemory->GameTranscientMemoryArena, MByte(100) - sizeof(game_memory::arena)); // 
        gameState->scratchUpdateArena = *game_memory::createMemoryArena(gameMemory->GameTranscientMemoryArena, MByte(100) - sizeof(game_memory::arena)); // 
        //
        setupGameState(gameState);
        //
        gameState->gameState = control_state_load_level;
        //
        gameState->cameraEntity.focalLength = 0.62f;
		gameState->cameraEntity.distanceAboveTarget = 5.55f; // 9.0f;
        gameState->cameraEntity.iFacing = v3(0.0f, -1.0f, 0.0f);
        gameState->cameraEntity.iPtSpeed = v3(0.0f);
        gameState->cameraEntity.iWorldPosition.regionCoord = v3i(51, 100, 52);
        gameState->cameraEntity.iWorldPosition.offset = v3(0.0f, 0.0f, 0.0f);
        gameState->cameraEntity.state = CameraState_EntityFollowBounded;

        gameState->activeCamera = &gameState->cameraEntity;

#ifdef RC_TESTCODE
        gameState->runState = run_state_debug | run_state_show_collision | run_state_show_outline;
        gameState->debugEntity = gameState->cameraEntity;
        gameState->debugEntity.focalLength = 0.62f;
        gameState->debugEntity.distanceAboveTarget = 17.6f; // 9.0f;
        gameState->debugEntity.state = CameraState_FreeRoam;
#endif

        gameState->viewUnitDimention = v3(render_const::screenUnitDimention.x, 2.0f * gameState->regionUnitDimentions.y, render_const::screenUnitDimention.y);

        gameState->addEntry = gameMemory->platformApi.addEntry;
        gameState->completeAllWork = gameMemory->platformApi.completeAllWork;
        gameState->loadFile = gameMemory->platformApi.loadFile;

        // create TemporaryState
        TemporaryState* tempState = PushStruct(&gameState->engineArena, TemporaryState);
        tempState->TranArena = *game_memory::createMemoryArena(gameMemory->GameTranscientMemoryArena, MByte(60) - sizeof(game_memory::arena));
        tempState->LowPriorityQueue = gameMemory->LowPriorityQueue;
        tempState->HighPriorityQueue = gameMemory->HighPriorityQueue;
        for (uint32 taskI = 0; taskI < ArrayCount(tempState->Tasks); ++taskI) {
            tempState->Tasks[taskI].beingUsed = false32;
            tempState->Tasks[taskI].memoryFlush = {};
            tempState->Tasks[taskI].arena = *game_memory::createMemoryArena(&tempState->TranArena, MByte(10) - sizeof(game_memory::arena));
        }
        // create the GameAssets
        gameState->assets = AllocateGameAssets(&gameState->assetArena, MByte(4) - sizeof(game_memory::arena), tempState);
        // NOTE(Roman): reserve a place for collision rule hash

    }

    if (!gameState->renderTargetPtr) {
        game_render_engine::initRenderer(gameState, window);
    }
    updateScreenViewport(gameState);
}

GAMEHANDLER_API SHUTDOWN_GAME_STATE(ShutdownGameState) {
    GameState * gameState = gameMemory->gameState;
    if (gameState) {
        if (gameState->renderTargetPtr) {
            game_render_engine::freeRenderTarget(gameState);
        }
    }
}

// --- (GameMemory* gameMemory, real32, real32, game_sound::SoundPlayBuffer* pBuffer, game_sound::sound_piece* newSound)
// GameMemory* gameMemory
// real32
// real32
// game_sound::SoundPlayBuffer* pBuffer
//
GAMEHANDLER_API GAME_GET_SOUND_BUFFERS(GameGetSoundBuffers) {
    GameState * gameState = gameMemory->gameState;

	// NOTE(Roman): Multiplex all playing sounds into the buffer
	newSound->source;
    newSound->length;
}

//---------------------------------------------------------------------------------------------------------//
//
