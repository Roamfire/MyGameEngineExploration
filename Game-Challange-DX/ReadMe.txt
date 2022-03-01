/---------------
TODO Goals & Milestones:
1. draw animated player character (PC)- use asset group
2. input: control PC
- add controlled entity
3. camera: follow PC, animate level transition
- add camera following controlled entity
4. sort entities to proper draw order
5. entities collision:
 5.1 stop move
 5.2 correct move
 5.3 add to entity collision off 
6. debug system
- add stb, true type fonts
- add debug logging
- add debug rendering:
-- add ui menu
-- add ui HUD
-- add picking entities with mouse
7. audio playback, audio mixing
8. threaded rendering
- render optimization, multiple render regions; multithreaded rendering


*** Astroids game or frogger
9. multilevel world, add stairs to move up/down. (allow falling?)
- add constant gravity, setup floors to stop player from falling
- add up/down stairs
- add invisible collision regions
10. add AI
11. offscreen world update


*** Zelda like game
12. transition to OpenGL... BIG CHANGE
13. asset management, efficient memory model
14. lighting solution
15. ** normal maps???



Game:::----------------------------------------------------------------------------------------------
TO DO List...
- Implement multiple sim regions per frame
  1. per-entity clocking
  2. sim region merging? for multiplayer.
- Z!
  1. use 3d vectors
  2. how to have up/down? how it would look good in a 2d setting?
  3. Minkowski inclusion test for sim region
- Asset Loading
- Metagame / save game
  1. save state, for pausing or maybe level change
  2. crash recovery
  3. persistent state - locks/unlocks
  4. save slot?
- collision detection
  1. enter collision, exit collision (pass through entity, happens across multiple frames)
- AI
  1. path finding - bfs graph? a*?
  2. AI storage
  3. Brains
- World gen.
  1. place background things
  2. connections between areas?
  3. non-overlapping?
  - Maps
    1. magnets - how to???
- Animation , should lead to rendering
  1. skeletal animation (entity made from multiple entity volumes)
  2. particle system
- Audio
  1. sound effect triggers
  2. ambient sounds
  3. music
- Debug Code
  1. logging
  2. diagraming
PRODUCTION:
- rendering:
  1. 2d software rendering, multithread rendering
  2. 3d rendering, use OpenGl, add lighting solution.
     - mix 3d structures with billboard sprites?
     - generate normal maps from avilable images
- Game
  1. entity system
  2. world generation
** NOTE: will use OpenGL default 'right hand' coordinate system
-X to +X :: left to right
-Y to +Y :: bottom to top
-Z to +Z :: back to front
/---------------
lookup, david rosen character animation talk

************
Triangle mesh:::

// mesh
vertices [] v3
uvs [] v2
vertex_frames [] frame3

// light
lightmap_uvs [] v2
lightmap_colors [] v4
texture_colors  [] v4