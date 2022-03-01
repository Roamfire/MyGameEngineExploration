/* ========================================================================
$File: $
$Date: $
$Revision: $
$Creator: Roman Markovetski $
$Notice: (C) Copyright 2014 by Farmland Blossoms. All Rights Reserved. $
======================================================================== */

#ifndef __RC_X_GAME_STATE_H_
#include "RaidCore_Common.h"
#include "RaidCore_UtilsHeader.h"
#include "RaidCore_RenderHandler.h"
#include "RaidCore_GameWorld.h"
#include "RaidCore_Entity.h"
#include "RaidCore_Camera.h"
#include "RaidCore_Asset.h"
#include "RaidCore_UserInputHdr.h"
#include "RaidCore_GameImport.h"

struct GameInputState
{
	// xDelta, yDelta correspond to the up,down,left,and right keys or controller
	//  also, if mouse input is used as directional motion, xDelta and yDelta will correspond to the vector distance where the controller entity is in (0,0)
	union {
		struct {
			real32 xDelta;
			real32 yDelta;
			//
			real32 zDelta;
			//
			real32 w;
		};
		vec3
			Delta;
	};
	//
	real32 motionMagnitude;
	vec3 motionDirection;
	//
	// key state
	uint32 mappedKeyCount;
	uint32 *mappedKeys;
};

#include "RaidCore_Task.h"

struct TemporaryState {
    bool32 IsInitialized;
    game_memory::arena TranArena;

    struct TaskWithMemory Tasks[4];

    PlatformWorkQueue *HighPriorityQueue;
    PlatformWorkQueue *LowPriorityQueue;
};

enum game_control_state {
    control_state_init =0, // initial memory buildup
    control_state_load_level, // show a loading screen, flush assets, start loading new assets
    control_state_load_level_stream, // setup a task thread to: load additional level, flush low priority assets, load new assets; set state back to run...
    control_state_run, // game loop run; input -> update -> render -> sound...
    control_state_run_mapTransition, // stop input processing until done
    control_state_pause, // freeze run, stop game audio, keep ambiant and music track running
    control_state_shutdown // pause game, stop all threads, exit game loop when all threads are down 
};
enum game_run_state {
    run_state_run = 0,
    run_state_debug = 1<<0,
    run_state_show_collision = 1<<1,
    run_state_show_outline = 1<<2
};
struct GameState
{
    game_control_state
        gameState;
    uint32
        runState;

	GameAssets* assets;

	///////////////////////////////////////////////////
	// System objects
	// 1. Timer globals
	game_timer::RaidTick* raidTickGlobal;

	///////////////////////////////////////////////////
	// Rendering stats...
	// 1. Screen resolution
	// 2. tile/map start rendering point
	// 3. range of visible planes
	vec2i
		screenPixelResolution; // Window client area resolution in pixels
    real32
        screenAspectRatio; // safeRatio0(screenPixelResolution.x, screenPixelResolution.y)
    vec3
        viewUnitDimention; // this is the size of the cubic view frustrum in units (center is at camera focal length)
    real32 unitsToPixels;  // transform from units to screen pixels -- calculated to keep same ratio on Z == Camera Focal
    //
	vec3
		regionUnitDimentions;

	GameInputState
		inputData;
    user_input::GameInput*
		gameInput;

	real32
		accelerateRate,
		maxSpeed_mps;	// Max Default Speed at meters per second (or velocity)
						// so... 
						// F(t)		- position over time...
						// F'(t)	- velocity
						// F"(t)	- accelaration

	//
	// Sim regions update at a slower rate than the view area.
	// So, each frame we update the display area (real time control area), we update a different sim region
	// sim regions adjacent to the view area react to the view area entities... so every 8 frames all adjacent regions process events.
	//
	// All entities in sim regions that exit the region into another sim region only test vs collision or AI and process action result,
	// but do not process reactions.
	coord_system::RegionCoord
		simRegionPosition;

	/*
	A = F/m ... Can be constant
	V = A*t + V(old)
	Pos = A/2 * t^2 + V*t + Pos(old)
	*/

	//
	// What is the current time slice we are calculating our update on...
	real32
		m_TimeFrame;
	uint32
		updateId; // current frame # out of the 1second framerate

	///////////////////////////////////////////////////
	// World states
	// 1. max dimentions of plane(s) and tile maps
	// 2. pixel dimention of a tile (not to be confused with the actual bitmap dimentions, those may differ wildly)
	// 3. game world position, reletive to game world center of coordinates
	// Contained inside the GameWorld structure
	game_map_structs::GameWorld
		gameWorld;

	//
	// Display render target
	game_render_engine::GameRenderTarget*
		renderTargetPtr;

	//
	// Entities and Actors
	GameCamera
		cameraEntity; // Special entity
    GameCamera
        *activeCamera; // Special entity
#ifdef RC_TESTCODE
	GameCamera
		debugEntity; // Special entity
    bool32
        isDebugCamera, isOrthographic;
#endif
    //
    WorldAreaEntity*
        cameraTargetEntity; // to follow

	WorldAreaEntity*
		controlledEntity; // reference to the entity under user control

	///////////////////////////////////////////////////
    platform_add_entry *addEntry;
    platform_complete_all_work *completeAllWork;
    platform_load_file *loadFile;

    ///////////////////////////////////////////////////
	// memory_func arena's
	game_memory::arena
		engineArena;        // Render engine memory - cache all the data that is loaded in the rendering system. loaded data will contain the "handles" for the assets being rendered, reference id's to the images in the asset system, or handles for OpenGl buffers, etc.
	game_memory::arena
		assetArena;         // loaded assets - raw data, this memory arena should contains data structures that support reuse of allocated memory
	game_memory::arena
		entityArena;        // memory for game world, regions generated (loaded), all entities in game.
	game_memory::arena
        scratchFrameArena;  // used to contain all data needed to draw a frame, flushed each frame.
	game_memory::arena
		scratchUpdateArena; // contains all data generated in each update cycle, flushed for each rendered frame.
};
void setupGameState(GameState* gameState);
void changeState(GameState* gameState, game_control_state state);

#define __RC_X_GAME_STATE_H_
#endif//__RC_X_GAME_STATE_H_
