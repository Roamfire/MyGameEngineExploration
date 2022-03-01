/* ========================================================================
$File: $
$Date: $
$Revision: $
$Creator: Roman Markovetski $
$Notice: (C) Copyright 2014 by Farmland Blossoms. All Rights Reserved. $
======================================================================== */

//
// NOTE(Roman): the Actor is a game logic entity representitive
// here we decide what happens:
// 1. on collision
// 2. user input
// 3. game  mode changes
// 4. scenematic events
// 5. AI input
// 
// this is where an entity is aware of it being a man/dog/cat/sword etc.

#ifndef __RC_ACTOR_H_
#include "../RaidCore/Headers/RaidCore_Common.h"
#include "../RaidCore/Headers/RaidCore_Math.h"

enum npc_state {
    NpcState_Idle = 0,
    NpcState_Search = (1 << 0),
    NpcState_Follow = (1 << 1),
    NpcState_Attack = (1 << 2),
    NpcState_Walk = (1 << 3),
    NpcState_Run = (1 << 4),
    NpcState_Hurt = (1 << 5),
    NpcState_Dead = (1 << 6),
};

struct Stair : public Entity {
    real32 maxHeight;
};

struct ActorHP {
    uint32 maxHitPoints;
    uint32 hitPoints;
};

class Hero : public Entity, public ActorHP {
    uint32 state;
    uint32 recover;
};

struct Brain {

};

class Monster : public Entity, public ActorHP {
    uint32 state;
    uint32 recover;

    Brain
        *ai;
    Entity
        *target; // to follow

public :
    Brain* getAI() { return ai; }
     void setAI(Brain* mind) { ai = mind; }
   Entity* getTarget() { return target; }
};

#define __RC_ACTOR_H_
#endif//__RC_ACTOR_H_
