/* ========================================================================
$File: $
$Date: $
$Revision: $
$Creator: Roman Markovetski $
$Notice: (C) Copyright 2014 by Farmland Blossoms. All Rights Reserved. $
======================================================================== */

// --
#include "../RaidCore/Headers/RaidCore_MathOperators.h"
#include "../RaidCore/Headers/RaidCore_Physics.h"
#include "../GameHandler/GameHandler.h"

#pragma warning(disable:4244)

// NOTE(Roman): generic functions, will not use a class since will never actually need any data encapsulated
namespace game_physics {
    static const int32 failureValue = ~0;

    static const real32 TerrainStopDrag = 6.0f;
    static const real32 ResistanceStopDrag = 2.0f;
    static const real32 EarthGravityAcceleration = -9.81f;

    vec3 applyMovementVector(vec3& Velocity, real32 MaxVelocity, real32 dt) {
        vec3 Pos = {};
        real32 speed = game_math::length(Velocity);

        if (speed > MaxVelocity) {
            Velocity *= (MaxVelocity / speed);
        }
        game_math::nullify(Velocity);
        Pos = Velocity * dt;
        return Pos;
    }
    vec3 applyAcceleration(vec3& Acceleration, vec3& Velocity, real32 MaxVelocity, real32 dt) {
        vec3 Pos = Velocity * dt + (Acceleration * 0.5f) * game_math::square(dt);
        Velocity += Acceleration * dt;
        real32 speed = game_math::length(Velocity);
        
        if (speed > MaxVelocity) {
            Velocity *= (MaxVelocity / speed);
        }
        game_math::nullify(Velocity);
        return Pos;
    }

    vec3 calculateTerrainDrag(const vec3& Velocity, real32 Coefficient, real32 dt) {
        vec3 drag = game_math::sign(Velocity) * (-Coefficient) * dt; // negetive velocity coefficient
        return drag;
    }

    real32 getDistance(vec3& Pt1, vec3& Pt2) {
        vec3 V = {};
        V = Pt1 - Pt2;
        return game_math::length(V);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////
    vec3 getGenericMotionOffset(vec3& CurrentVelocity, MotionProfile * motion, real32 MotionDragK, real32 dTinSec) {
        //
        // No acceleration, apply negetive acceleration, equal to TerrainDragK*velocity, until velocity == 0.0
        if (game_math::isZero(motion->fMagnitude)) {
            CurrentVelocity *= 0.25f;
            motion->fMagnitude = 1.0f;
        }
        else {
            if (game_math::isZero(motion->vAccelerate.x) || game_math::isZero(motion->vAccelerate.x * CurrentVelocity.x)) {
                CurrentVelocity.x *= 0.25f; 
            }
            if (game_math::isZero(motion->vAccelerate.y) || game_math::isZero(motion->vAccelerate.y * CurrentVelocity.y)) {
                CurrentVelocity.y *= 0.25f;
            }
            if (game_math::isZero(motion->vAccelerate.z) || game_math::isZero(motion->vAccelerate.z * CurrentVelocity.z)) {
                CurrentVelocity.z *= 0.25f;
            }
        }
        // Apply DRAG!!!
        return game_physics::applyAcceleration(motion->vAccelerate, CurrentVelocity, motion->fMaxSpeed, dTinSec) * motion->fMagnitude;
    }
}
