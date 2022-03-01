/* ========================================================================
$File: $
$Date: $
$Revision: $
$Creator: Roman Markovetski $
$Notice: (C) Copyright 2014 by Farmland Blossoms. All Rights Reserved. $
======================================================================== */

#ifndef __RC_X_INPUT_HEADERS_

#include "RaidCore_Common.h"
struct GameState;

/**
* kbKeyState,
* A key can have three states...
* 1. e_kbReset - idle state, not pressed
* 2. e_kbPressed - already pressed, key continues to stay pressed
*/
namespace user_input {
    enum eKbState {
        e_kbReset = 0,
        e_kbPressed = 1
    };
    /**
    * kbKeyMods
    */
#define RC_LALT 0x10
#define RC_RALT 0x20
#define RC_ALT 0x30
#define RC_LCTRL 0x01
#define RC_RCTRL 0x02
#define RC_CTRL 0x03
#define RC_LSHIFT 0x04
#define RC_RSHIFT 0x08
#define RC_SHIFT 0x0C
    union kbKeyMods {
        struct kbKeyModFields {
            uint8 leftCtrl : 1;
            uint8 rightCtrl : 1;
            uint8 leftShift : 1;
            uint8 rightShift : 1;
            uint8 leftAlt : 1;
            uint8 rightAlt : 1;
            uint8 reserved : 2;
        } Fields;
        uint8 State;
    };

    //
    // Keyboard data structure
    struct IOActionState {
        kbKeyMods
            lastModState;
        eKbState
            lastState;
        uint8
            clickCount;
        real32 
            force;
    };

    //
    // Controller data structure
    struct ControllerState {
        uint32 CtrlId;
        uint32 lastPacketNumber;

        real32 R_XAxis;
        real32 R_YAxis;
        real32 L_XAxis;
        real32 L_YAxis;

        uint8 Pad_Up;
        uint8 Pad_Down;
        uint8 Pad_Left;
        uint8 Pad_Right;

        uint8 RB_Trigger; // 0-255
        uint8 RT_Trigger; // 0-255
        uint8 LB_Trigger; // 0-255
        uint8 LT_Trigger; // 0-255
        uint8 Btn_X; // SQUARE
        uint8 Btn_Y; // TRIANGLE
        uint8 Btn_A; // X
        uint8 Btn_B; // CIRCLE
        uint8 Btn_Start; // Play
        uint8 Btn_Select; // Back
    };
    //
    void InitControllerHandler(ControllerState*);
    typedef ControllerState ControllerHandlerImpl;
    //
    enum {
        key_vertical = 0,
        key_horizontal,
        key_btn_A,
        key_btn_B,
        key_btn_X,
        key_btn_Y,
        key_btn_Start,
        key_btn_Select,
        key_btn_RB,
        key_btn_RT,
        key_btn_LB,
        key_btn_LT,
        //
        key_btn_RT3, // 
        key_btn_LT3, // 
        //
        key_btn_special0,
        key_btn_special1,
        key_btn_special2,
        key_btn_special3,
        key_btn_special4,
        key_btn_special5,
        key_btn_special6,
        key_btn_special7,
        key_btn_special8,
        key_btn_special9,
        key_btn_specialA,
        key_btn_specialB,
        key_btn_specialC,
        key_btn_specialD,
        key_btn_specialE,
        key_btn_specialF,

        key_states_count
    };
    struct GameInput {
        // Note: library code does NOT free the allocated handlers!!!
        // Caller must handle resource deallocation.
        IOActionState
            m_keys[key_states_count];

        // Note: library code does NOT free the allocated handlers!!!
        // Attached controllers must be deallocated by the caller!!!
        ControllerState
            m_controllers[4];
        uint8
            m_controllerCount;
    };

    ////
    // Keyboard & Mouse
	inline void addKeyForce(user_input::GameInput* gameInput, uint32 id, eKbState state, real32 force) {
		IOActionState* pState = gameInput->m_keys + id;
		Assert(NULL != pState);
		if (state == e_kbReset) {
			pState->force -= force;
		}
		else {
			pState->force += force;
		}
		if (state == pState->lastState)
			return;
		++(pState->clickCount);
		pState->lastState = state;
	}
	inline void setKeyState(user_input::GameInput* gameInput, uint32 id, eKbState state, real32 force = 0.0f) {
        IOActionState* pState = gameInput->m_keys + id;
        Assert(NULL != pState);
        if (state == e_kbReset) {
            pState->force = 0.0f;
        } else {
            pState->force = force;
        }
        if (state == pState->lastState)
            return;
        ++(pState->clickCount);
        pState->lastState = state;
    }
    inline void addKeyStateSwitch(user_input::GameInput* gameInput, uint32 id, uint8 count = 1) {
        IOActionState* pState = gameInput->m_keys + id;
        Assert(NULL != pState);
        pState->clickCount += count;
        pState->lastState = eKbState::e_kbReset;
    }
    inline void reset(user_input::GameInput* gameInput, uint32 id) {
        IOActionState* state = gameInput->m_keys + id;
        state->clickCount = (uint32)state->lastState;
        state->lastState = e_kbReset;
        state->force = 0.0f;
    }
    inline bool32 wasPressed(user_input::GameInput* gameInput, uint32 id) {
        IOActionState* state = gameInput->m_keys + id;
        bool32 Result = ((state->clickCount > 1) ||
                      ((state->clickCount == 1) && (state->lastState != user_input::e_kbPressed)));
        return(Result);
    }
    inline bool32 isPressed(user_input::GameInput* gameInput, uint32 id) {
        IOActionState* state = gameInput->m_keys + id;
        bool32 Result = (state->lastState == user_input::e_kbPressed);
        return(Result);
    }
    inline real32 keyForce(user_input::GameInput* gameInput, uint32 id) {
        IOActionState* state = gameInput->m_keys + id;
        real32 Result = (state->force);
        return(Result);
    }
}

#define __RC_X_INPUT_HEADERS_
#endif//__RC_X_INPUT_HEADERS_

